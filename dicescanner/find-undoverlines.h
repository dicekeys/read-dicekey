#pragma once

#include <float.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include <iostream>
#include <math.h>
#include "vfunctional.h"
#include "rectangle.h"
#include "find-squares.h"
#include "die.h"
#include "rotate.h"
#include "ocr.h"
#include "sample-point.h"

using namespace cv;

static float normalizeAngle(float angle)
{
	const float normalizedAngle = angle - round(angle / 90) * 90;
	return normalizedAngle;
}

// returns sequence of squares detected on the image.
static std::vector<RectangleDetected> findUndoverlines(const cv::Mat& gray, int N = 13)
{
	float min_underline_length = float(std::min(gray.size[0], gray.size[1])) / 80;
	float max_underline_length = float(std::min(gray.size[0], gray.size[1])) / 8;

	std::vector<RectangleDetected> candidateUnderOverLines = vfilter<RectangleDetected>(
		findRectangles(gray, N), isRectangleShapedLikeUndoverline);

	if (candidateUnderOverLines.size() > 25) {
		// Remove rectangles that stray from the median

		float medianArea = median(vmap<RectangleDetected, float>(candidateUnderOverLines,
			[](RectangleDetected r) -> float { return r.area; }));
		float minArea = 0.75f * medianArea;
		float maxArea = medianArea / 0.75f;
		candidateUnderOverLines = vfilter<RectangleDetected>(candidateUnderOverLines, [minArea, maxArea](RectangleDetected r) {
			return  (r.area >= minArea && r.area <= maxArea);
			});

		// Recalculate median for survivors
		float areaHighPercentile = percentile(
			vmap<RectangleDetected, float>(candidateUnderOverLines, [](RectangleDetected r) -> float { return r.area; }),
			85
		);
		// Calculate slope of survivors
		float medianAngle = median(vmap<RectangleDetected, float>(candidateUnderOverLines,
			[](RectangleDetected r) -> float { return normalizeAngle(r.angle); }));

		candidateUnderOverLines = removeOverlappingRectangles(candidateUnderOverLines, [areaHighPercentile, medianAngle](RectangleDetected r) -> float {
			float deviationFromSideRatio = (r.shorterSideLength / r.longerSideLength) / undoverlineWidthOverLength;
			if (deviationFromSideRatio < 1 && deviationFromSideRatio > 0) {
				deviationFromSideRatio = 1 / deviationFromSideRatio;
			}
			deviationFromSideRatio -= 1;
			float devationFromSideLengthRatioPenalty = 2.0f * deviationFromSideRatio;
			float deviationFromTargetArea = r.area < areaHighPercentile ?
				// Deviation penalty for falling short of target
				((areaHighPercentile / r.area) - 1) :
				// The consequences of capturing extra area are smaller,
				// so cut the penalty in half for those.
				(((r.area / areaHighPercentile) - 1) / 2);
			// The penalty from deviating from the target angle
			float angleDiff = normalizeAngle(r.angle) - medianAngle;
			angleDiff = MIN(angleDiff, 90.0f - angleDiff);
			float deviationFromTargetAngle = 2.0f * angleDiff;

			return devationFromSideLengthRatioPenalty + deviationFromTargetArea + deviationFromTargetAngle;
			});
	}

	return candidateUnderOverLines;
}


cv::Point2f pointBetween(cv::Point a, cv::Point b)
{
	return cv::Point2f(
		(a.x + b.x) / 2.0f,
		(a.y + b.y) / 2.0f
	);
}

struct UndoverlineShape {
	bool isVertical = false;
	cv::Point2f lineThroughMidWidthStart = cv::Point2f(0, 0);
	cv::Point2f lineThroughMidWidthEnd = cv::Point2f(0, 0);
	cv::Point2f center = cv::Point2f(0,0);
	uchar whiteBlackThreshold = 128;
	float length = 0.0f;
	float height = 0.0f;
	float angle = 0.0f;
	std::vector<uchar> medianPixelValues = std::vector<uchar>(NumberOfDotsInUndoverline);
	uint binaryCodingReadForwardOrBackward = 0;
};

static UndoverlineShape isolateUndoverline(cv::Mat image, RectangleDetected line) {
	UndoverlineShape result;

	const int lineHeight = MAX(line.bottomLeft.y, line.bottomRight.y) - MIN(line.topLeft.y, line.topRight.y);
	const int lineWidth = MAX(line.topRight.x, line.bottomRight.x) - MIN(line.topLeft.x, line.bottomLeft.x);
	result.isVertical = lineHeight > lineWidth;
	cv::Point2f start, end;
	float pixelStepX, pixelStepY;

	if (result.isVertical) {
		// Vertical (the line is closer to vertical than horizontal)
		// start from half way between top left and top right and proceed to half way from bottom left and bottom right
		start = pointBetween(line.topLeft, line.topRight);
		end = pointBetween(line.bottomLeft, line.bottomRight);
		// A step moving one Y pixel moves a fraction of a pixel in the x direction
		pixelStepY = 1;
		pixelStepX = ((end.x - start.x) / (end.y - start.y));
	}
	else {
		// Horizontal (the line is closer to horizontal than vertical)
		// start from half way between top left and bottom left and proceed from half way between top right and bottom right
		start = pointBetween(line.topLeft, line.bottomLeft);
		end = pointBetween(line.topRight, line.bottomRight);
		// A step moving one X pixel moves a fraction of a pixel in the Y direction
		pixelStepX = 1;
		pixelStepY = ((end.y - start.y) / (end.x - start.x));
	}
	if (abs(pixelStepX) > 1 || abs(pixelStepY) > 1) {
		std::cerr << "Pixel step error " <<pixelStepX << ", " << pixelStepY;
	}

	// Extend start and end .15mm to side in case we cut off the edge
	float fractionToExtend = 0.15f / 6.0f;
	float fractionToExtendH = (end.x - start.x) * fractionToExtend;
	float fractionToExtendV = (end.y - start.y) * fractionToExtend;
	start.x -= fractionToExtendH;
	start.y -= fractionToExtendV;
	end.x += fractionToExtendH;
	end.y += fractionToExtendV;


	// Take 25 samples of points between start and end so that we can find
	// theshold between light and dark.
	float deltaH = end.x - start.x;
	float deltaV = end.y - start.y;
	const std::vector<float> UndoverlineWhiteDarkSamplePoints = { 0,
		0.05f, 0.1f, 0.15f, 0.2f, 0.25f, 0.3f, 0.35f, 0.4f, 0.45f, 0.5f,
		0.55f, 0.6f, 0.65f, 0.7f, 0.75f, 0.8f, 0.85f, 0.9f, 0.95f, 1
	};
	const size_t numSamples = NumberOfDotsInUndoverline + 2;
	std::vector<uchar> pixelSamples = samplePointsAlongLine(image, start, end, UndoverlineWhiteDarkSamplePoints, 5);
	uchar whiteBlackThreshold = bimodalThreshold(pixelSamples, 4, 4);

	// Trim the start of the line by moving the start closer to the end,
	// until we reach the first black pixel	
	while (image.at<uchar>(start) > whiteBlackThreshold && (start.x < end.x || start.y < end.y)) {
		// The starting point hasn't reached the black underline.
		start.x += pixelStepX;
		start.y += pixelStepY;
	}

	// Trim the end of the line by moving the end closer to the start,
	// until we reach the first black pixel	
	while (image.at<uchar>(end) > whiteBlackThreshold && (start.x < end.x || start.y < end.y)) {
		// The starting point hasn't reached the black underline.
		end.x -= pixelStepX;
		end.y -= pixelStepY;
	}

	// Recalculate distances based on new start/end point
	deltaH = end.x - start.x;
	deltaV = end.y - start.y;
	result.length = sqrt(deltaH * deltaH + deltaV * deltaV);
	result.height = undoverlineWidthOverLength * result.length;

	// Calculate the width in pixels of the dots that encode data in undoverline's
	// by taking the length of the line in pixels * the fraction of a line consumed
	// by each dot.
	const float undoverlineDotWidthInPixels = result.length * DieDimensionsFractional::undoverlineDotWidth;

	size_t numberOfPixelsToSampleAroundPoint =
		undoverlineDotWidthInPixels < 2.0f ? 1 :
		undoverlineDotWidthInPixels < 3.0f ? 5 :
		undoverlineDotWidthInPixels < 3.5 ? 9 :
		undoverlineDotWidthInPixels < 4.5 ? 13 :
		21;
	result.medianPixelValues = samplePointsAlongLine(
		image, start, end,
		DieDimensionsFractional::dotCentersAsFractionOfUndoverline,
		numberOfPixelsToSampleAroundPoint
	);

	// The binary coding has 11 bits,
	// from most significant (10) to least (0)
	//   Bit 10:   always 1
	//   Bit  9:   1 if overline, 0 if underline
	//   Bits 8-1: underline/overline-specific encoding that maps to letter and digit
	//             underline encoding always has at least two 1s and one 0
	//             overline encoding always has at leat two 0s and one 1
	//   Bit  0:   always 0 
	//
	// Thus, all 11-bit encodings have at least four 0s and four 1s.]
	const size_t minNumberOf0s = 4, minNumberOf1s = 4;
	// minFractionOfZerosAndOnes = float(4.0f / float(NumberOfDotsInUndoverline));

	// In finding a white/black threshold, the sampling should ensure
	// there are at least enough zeros an dones above/below the threshold.
	result.whiteBlackThreshold = bimodalThreshold(result.medianPixelValues, minNumberOf0s, minNumberOf1s);

	result.binaryCodingReadForwardOrBackward = 0;
	for (size_t i = 0; i < NumberOfDotsInUndoverline; i++) {
		// 1s are white, or lower values)
		result.binaryCodingReadForwardOrBackward <<= 1;
		if (result.medianPixelValues[i] > result.whiteBlackThreshold) {
			result.binaryCodingReadForwardOrBackward += 1;
		}
	}

	result.lineThroughMidWidthStart = start;
	result.lineThroughMidWidthEnd = end;
	result.center = pointBetween(start, end);
	result.angle = float( (atan2(deltaV, deltaH) * 180 / M_PI) );
	return result;
}


//
//static uint undoverlinePixelValuesToBits(std::vector<uchar> dotValues)
//{
//	// The smallest number of white blocks would be
//	//   1 for the orientation
//	//   1 for the letter (true even if permuted and inverted)
//	//   1 for digit (true even if permuted and inverted)
//	float minFractionOfZerosAndOnes = 3.0f / 11.0f;
//	uchar threshold = bimodalThreshold(dotValues, minFractionOfZerosAndOnes, minFractionOfZerosAndOnes);
//
//	// The binary coding has 11 bits,
//	// from most significant (10) to least (0)
//	//   Bit 10:   always 1
//	//   Bit  9:   1 if overline, 0 if underline
//	//   Bits 8-1: Letter/digit byte (permuted & negated in overlines)
//	//   Bit  0:   always 0 
//	uint binaryCodingReadForwardOrBackward = 0;
//	for (size_t i = 0; i < NumberOfDotsInUndoverline; i++) {
//		// 1s are white, or lower values)
//		binaryCodingReadForwardOrBackward <<= 1;
//		if (dotValues[i] > threshold) {
//			binaryCodingReadForwardOrBackward += 1;
//		}
//	}
//	return binaryCodingReadForwardOrBackward;
//}

static void readUndoverline(cv::Mat imageColor, cv::Mat image, RectangleDetected line)
{
	Mat imageCopy = imageColor.clone();

	const Point points[4] = {
		cv::Point(line.bottomLeft),
		cv::Point(line.topLeft),
		cv::Point(line.topRight),
		cv::Point(line.bottomRight),
	};

	const Point* ppoints[1] = {
		points
	};

	int npt[] = { 4 };

	UndoverlineShape undoverline = isolateUndoverline(image, line);

	polylines(imageCopy, ppoints, npt, 1, true, cv::Scalar(0, 0, 255), 2);
	cv::imwrite("underline-within-image.png", imageCopy);
	cv::imwrite("underline-isolated.png", copyRotatedRectangle(image, undoverline.center, undoverline.angle, cv::Size2f(undoverline.length, undoverline.length/6.0f)));
	// auto undoverlinePixelValues = readUndoverlinePoints(image, undoverline);
	// uint binaryCodingReadForwardOrBackward = undoverlinePixelValuesToBits(undoverlinePixelValues);

	auto decodedUndoverline = decodeUndoverlineBits(undoverline.binaryCodingReadForwardOrBackward, undoverline.isVertical);

	if (!decodedUndoverline.isValid) {
		return;
	}

	// Correct the angle of the undoverline since we read it in the reverse direction
	// by offsetting it by 180 degress
	if (decodedUndoverline.wasReadInReverseOrder) {
		undoverline.angle = undoverline.angle < 0 ?
			undoverline.angle + 180 :
			undoverline.angle - 180;
	}

	float upAngleInDegrees =
		undoverline.angle + (decodedUndoverline.isOverline ? 90 : -90);
	float upAngleInRadians = float(upAngleInDegrees * (2 * M_PI / 360.0));

	// pixels per mm the length of the overline in pixels of it's length in mm,
	// or, undoverline.length / mmDieUndoverlineLength;
	double mmToPixels = double(undoverline.length) / DieDimensionsMm::undoverlineLength;
	float pixelsFromCenterOfUnderlineToCenterOfDie = float(
		DieDimensionsMm::centerOfUndoverlineToCenterOfDie *
		// mmFromCenterOfUndoverlineToCenterOfDie *
		mmToPixels);
	int textHeightPixels = int(ceil(DieDimensionsMm::textRegionHeight * mmToPixels));
	int textWidthPixels = int(ceil(DieDimensionsMm::textRegionWidth * mmToPixels));
	// Use an even text region width so we can even split it in two at the center;
	if ((textWidthPixels % 2) == 1) {
		textWidthPixels += 1;
	}
	cv::Size textRegionSize = cv::Size(textWidthPixels, textHeightPixels);

	const auto x = line.center.x + pixelsFromCenterOfUnderlineToCenterOfDie * cos(upAngleInRadians);
	const auto y = line.center.y + pixelsFromCenterOfUnderlineToCenterOfDie * sin(upAngleInRadians);
	const cv::Point2f dieCenter = cv::Point2f(x, y);

	const auto textImage = copyRotatedRectangle(image, dieCenter, undoverline.angle, textRegionSize);
	// Setup a rectangle to define your region of interest
	const cv::Rect letterRect(0,0, textRegionSize.width / 2, textRegionSize.height);
	const cv::Rect digitRect( textRegionSize.width / 2, 0, textRegionSize.width / 2, textRegionSize.height);
	auto letterImage = textImage(letterRect);
	auto digitImage = textImage(digitRect);

	cv::imwrite("image-being-processed.png", image);
	cv::imwrite("text-region.png", textImage);
	cv::imwrite("letter.png", letterImage);
	cv::imwrite("digit.png", digitImage);

	const auto l = readCharacter(letterImage, false);
	const auto d = readCharacter(digitImage, true);

	static int error = 1;
	if (l.confidence > 50.0f && decodedUndoverline.letter != l.charRead) {
		std::string errBase = "error-" + std::to_string(error++) + "-read-" + std::string(1, decodedUndoverline.letter) + "-as-" + std::string(1, l.charRead);
		cv::imwrite(errBase + "-line.png", copyRotatedRectangle(image, line.center, line.angle, cv::Size2f( line.size.width + 2, line.size.height + 2)));
		cv::imwrite(errBase + ".png", letterImage);
	}
	if (d.confidence > 50.0f && decodedUndoverline.digit != d.charRead) {
		cv::imwrite("error-" + std::to_string(error++) + "-read-" + std::string(1, decodedUndoverline.digit) + "-as-" + std::string(1, d.charRead) + ".png", digitImage);
	}


	std::string result = std::string("") + decodedUndoverline.letter + decodedUndoverline.digit + char('0' + decodedUndoverline.numberOf90DegreeeClockwiseRotationsFromUpright);
	
	// line.angle;
}
