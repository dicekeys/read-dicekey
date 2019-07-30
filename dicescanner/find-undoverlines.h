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
#include "point-operations.h"
#include "find-rectangles.h"
#include "rectangle.h"
#include "die.h"
#include "rotate.h"
#include "ocr.h"
#include "sample-point.h"
#include "decode-die.h"

static float normalizeAngle(float angle)
{
	const float normalizedAngle = angle - round(angle / 90) * 90;
	return normalizedAngle;
}

const float undoverlineWidthOverLength = DieDimensionsMm::undoverlineThickness / DieDimensionsMm::undoverlineLength;
const float minWidthOverLength = undoverlineWidthOverLength / 1.5f;
const float maxWidthOverLength = undoverlineWidthOverLength * 1.5f;

static bool isRectangleShapedLikeUndoverline(RectangleDetected rect) {
	float shortToLongRatio = rect.shorterSideLength / rect.longerSideLength;
	return (
		shortToLongRatio >= minWidthOverLength &&
		shortToLongRatio <= maxWidthOverLength
		);
}

struct DieRead {
	Line underline;
	Line overline;
	unsigned char underlineEncoding = 0, overlineEncoding = 0;
	cv::Point2f centerInferred;
	char letterRead = 0, digitRead = 0;
	float letterConfidence, digitConfidence;
};

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

// struct UndoverlineShape {
// 	// bool isVertical = false;
// 	cv::Point2f start = cv::Point2f(0, 0);
// 	cv::Point2f end = cv::Point2f(0, 0);
// 	uchar whiteBlackThreshold = 128;
// 	// std::vector<uchar> medianPixelValues = std::vector<uchar>(NumberOfDotsInUndoverline);
// 	// uint binaryCodingReadForwardOrBackward = 0;
// };


static Line undoverlineRectToLine(cv::Mat grayscaleImage, RectangleDetected lineBoundaryRect) {
	const int lineHeight = MAX(lineBoundaryRect.bottomLeft.y, lineBoundaryRect.bottomRight.y) - MIN(lineBoundaryRect.topLeft.y, lineBoundaryRect.topRight.y);
	const int lineWidth = MAX(lineBoundaryRect.topRight.x, lineBoundaryRect.bottomRight.x) - MIN(lineBoundaryRect.topLeft.x, lineBoundaryRect.bottomLeft.x);

	const bool isVertical = lineHeight > lineWidth;
	cv::Point2f start, end;
	float pixelStepX, pixelStepY;

	if (isVertical) {
		// Vertical (the line is closer to vertical than horizontal)
		// start from half way between top left and top right and proceed to half way from bottom left and bottom right
		start = pointBetween2f(lineBoundaryRect.topLeft, lineBoundaryRect.topRight);
		end = pointBetween2f(lineBoundaryRect.bottomLeft, lineBoundaryRect.bottomRight);
		// A step moving one Y pixel moves a fraction of a pixel in the x direction
		pixelStepY = 1;
		pixelStepX = ((end.x - start.x) / (end.y - start.y));
	}
	else {
		// Horizontal (the line is closer to horizontal than vertical)
		// start from half way between top left and bottom left and proceed from half way between top right and bottom right
		start = pointBetween2f(lineBoundaryRect.topLeft, lineBoundaryRect.bottomLeft);
		end = pointBetween2f(lineBoundaryRect.topRight, lineBoundaryRect.bottomRight);
		// A step moving one X pixel moves a fraction of a pixel in the Y direction
		pixelStepX = 1;
		pixelStepY = ((end.y - start.y) / (end.x - start.x));
	}
	assert(abs(pixelStepX) <= 1);
	assert(abs(pixelStepY) <= 1);


	// Take 25 samples of points between start and end so that we can find
	// theshold between light and dark.
	const std::vector<float> UndoverlineWhiteDarkSamplePoints = { 0,
		0.05f, 0.1f, 0.15f, 0.2f, 0.25f, 0.3f, 0.35f, 0.4f, 0.45f, 0.5f,
		0.55f, 0.6f, 0.65f, 0.7f, 0.75f, 0.8f, 0.85f, 0.9f, 0.95f, 1
	};
	const size_t numSamples = NumberOfDotsInUndoverline + 2;
	std::vector<uchar> pixelSamples = samplePointsAlongLine(grayscaleImage, start, end, UndoverlineWhiteDarkSamplePoints);
	uchar whiteBlackThreshold = bimodalThreshold(pixelSamples, 4, 4);

	// Recalculate center and angle by finding point halfway between the sides at
	// at 10%, 90% of distance.
	// We can then re-approximate start and end by 
	// Angle is angle between 10% and 90% point.
	// looking for top and bottom borders
	//FIXME

	// Extend start and end .15mm to side in case we cut off the edge
	float fractionToExtend = 0.15f / 6.0f;
	float fractionToExtendH = (end.x - start.x) * fractionToExtend;
	float fractionToExtendV = (end.y - start.y) * fractionToExtend;
	start.x = MAX(0, MIN(start.x - fractionToExtendH, grayscaleImage.rows - 1));
	start.y = MAX(0, MIN(start.y - fractionToExtendV, grayscaleImage.rows - 1));
	end.x = MAX(0, MIN(end.x + fractionToExtendH, grayscaleImage.rows - 1));
	end.y = MAX(0, MIN(end.y + fractionToExtendV, grayscaleImage.rows - 1));

	// Trim the start of the line by moving the start closer to the end,
	// until we reach the first black pixel
	cv::Point2f oldStart = start;
	cv::Point2f oldEnd = end;
	while (
		grayscaleImage.at<uchar>(start) > whiteBlackThreshold &&
		isPointBetween2f(start.x + pixelStepX, start.y + pixelStepY, oldStart, oldEnd)
		) {
		// The starting point hasn't reached the black underline.
		start.x += pixelStepX;
		start.y += pixelStepY;
	}

	// Trim the end of the line by moving the end closer to the start,
	// until we reach the first black pixel	
	while (
		grayscaleImage.at<uchar>(end) > whiteBlackThreshold &&
		isPointBetween2f(end.x - pixelStepX, end.y - pixelStepY, start, oldEnd)
		) {
		// The starting point hasn't reached the black underline.
		end.x -= pixelStepX;
		end.y -= pixelStepY;
	}
	return { start, end };
}


static uint readUndoverlineBits(cv::Mat grayscaleImage, Line undoverline) {
	//const int lineHeight = MAX(line.bottomLeft.y, line.bottomRight.y) - MIN(line.topLeft.y, line.topRight.y);
	//const int lineWidth = MAX(line.topRight.x, line.bottomRight.x) - MIN(line.topLeft.x, line.bottomLeft.x);
	//result.isVertical = lineHeight > lineWidth;
	//cv::Point2f start, end;
	//float pixelStepX, pixelStepY;

	//if (result.isVertical) {
	//	// Vertical (the line is closer to vertical than horizontal)
	//	// start from half way between top left and top right and proceed to half way from bottom left and bottom right
	//	start = pointBetween2f(line.topLeft, line.topRight);
	//	end = pointBetween2f(line.bottomLeft, line.bottomRight);
	//	// A step moving one Y pixel moves a fraction of a pixel in the x direction
	//	pixelStepY = 1;
	//	pixelStepX = ((end.x - start.x) / (end.y - start.y));
	//}
	//else {
	//	// Horizontal (the line is closer to horizontal than vertical)
	//	// start from half way between top left and bottom left and proceed from half way between top right and bottom right
	//	start = pointBetween2f(line.topLeft, line.bottomLeft);
	//	end = pointBetween2f(line.topRight, line.bottomRight);
	//	// A step moving one X pixel moves a fraction of a pixel in the Y direction
	//	pixelStepX = 1;
	//	pixelStepY = ((end.y - start.y) / (end.x - start.x));
	//}
	//assert(abs(pixelStepX) <= 1);
	//assert(abs(pixelStepY) <= 1);


	//// Take 25 samples of points between start and end so that we can find
	//// theshold between light and dark.
	//const std::vector<float> UndoverlineWhiteDarkSamplePoints = { 0,
	//	0.05f, 0.1f, 0.15f, 0.2f, 0.25f, 0.3f, 0.35f, 0.4f, 0.45f, 0.5f,
	//	0.55f, 0.6f, 0.65f, 0.7f, 0.75f, 0.8f, 0.85f, 0.9f, 0.95f, 1
	//};
	//const size_t numSamples = NumberOfDotsInUndoverline + 2;
	//std::vector<uchar> pixelSamples = samplePointsAlongLine(grayscaleImage, start, end, UndoverlineWhiteDarkSamplePoints, 5);
	//uchar whiteBlackThreshold = bimodalThreshold(pixelSamples, 4, 4);

	//// Recalculate center and angle by finding point halfway between the sides at
	//// at 10%, 90% of distance.
	//// We can then re-approximate start and end by 
	//// Angle is angle between 10% and 90% point.
	//// looking for top and bottom borders
	////FIXME
	//
	//// Extend start and end .15mm to side in case we cut off the edge
	//float fractionToExtend = 0.15f / 6.0f;
	//float fractionToExtendH = (end.x - start.x) * fractionToExtend;
	//float fractionToExtendV = (end.y - start.y) * fractionToExtend;
	//start.x = MAX(0, MIN(start.x - fractionToExtendH, grayscaleImage.rows - 1));
	//start.y = MAX(0, MIN(start.y - fractionToExtendV, grayscaleImage.rows - 1));
	//end.x = MAX(0, MIN(end.x + fractionToExtendH, grayscaleImage.rows - 1));
	//end.y = MAX(0, MIN(end.y + fractionToExtendV, grayscaleImage.rows - 1));

	//// Trim the start of the line by moving the start closer to the end,
	//// until we reach the first black pixel
	//cv::Point2f oldStart = start;
	//cv::Point2f oldEnd = end;
	//while (
	//	grayscaleImage.at<uchar>(start) > whiteBlackThreshold &&
	//	isPointBetween2f(start.x + pixelStepX, start.y + pixelStepY, oldStart, oldEnd)
	//) {
	//	// The starting point hasn't reached the black underline.
	//	start.x += pixelStepX;
	//	start.y += pixelStepY;
	//}

	//// Trim the end of the line by moving the end closer to the start,
	//// until we reach the first black pixel	
	//while (
	//	grayscaleImage.at<uchar>(end) > whiteBlackThreshold &&
	//	isPointBetween2f( end.x - pixelStepX, end.y - pixelStepY, start, oldEnd)
	//) {
	//	// The starting point hasn't reached the black underline.
	//	end.x -= pixelStepX;
	//	end.y -= pixelStepY;
	//}

	//// Recalculate distances based on new start/end point
	//result.length = distance2f(start, end);
	//result.height = undoverlineWidthOverLength * result.length;

	//if (result.length < NumberOfDotsInUndoverline) {
	//	// There are no longer enough pixels to read.
	//	// Return now so that this line can be invalidated.
	//	return result;
	//}

	// Calculate the width in pixels of the dots that encode data in undoverline's
	// by taking the length of the line in pixels * the fraction of a line consumed
	// by each dot.
	const std::vector<uchar> medianPixelValues = samplePointsAlongLine(
		grayscaleImage, undoverline.start, undoverline.end,
		DieDimensionsFractional::dotCentersAsFractionOfUndoverline
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
	const uchar whiteBlackThreshold = bimodalThreshold(medianPixelValues, minNumberOf0s, minNumberOf1s);
	uint binaryCodingReadForwardOrBackward = sampledPointsToBits(medianPixelValues, whiteBlackThreshold);
	return binaryCodingReadForwardOrBackward;
}

static void readUndoverline(cv::Mat imageColor, cv::Mat image, RectangleDetected rectEncompassingLine)
{
	cv::Mat imageCopy = imageColor.clone();

	const cv::Point points[4] = {
		cv::Point(rectEncompassingLine.bottomLeft),
		cv::Point(rectEncompassingLine.topLeft),
		cv::Point(rectEncompassingLine.topRight),
		cv::Point(rectEncompassingLine.bottomRight),
	};

	const cv::Point* ppoints[1] = {
		points
	};

	int npt[] = { 4 };

	Line undoverline = undoverlineRectToLine(image, rectEncompassingLine);
	const float undoverlineLength = lineLength(undoverline);
	const uint binaryCodingReadForwardOrBackward = readUndoverlineBits(image, undoverline);
	const bool isVertical = abs(undoverline.end.x - undoverline.start.x) < abs(undoverline.end.y - undoverline.start.y);
	float angle = angle2f(undoverline);
	cv::Point2f center = pointBetween2f(undoverline);

	polylines(imageCopy, ppoints, npt, 1, true, cv::Scalar(0, 0, 255), 2);
	cv::imwrite("undoverline-within-image.png", imageCopy);
	cv::imwrite("undoverline-isolated.png", copyRotatedRectangle(image, center, angle, cv::Size2f(undoverlineLength, undoverlineLength/6.0f)));

	auto decodedUndoverline = decodeUndoverlineBits(binaryCodingReadForwardOrBackward, isVertical);

	if (!decodedUndoverline.isValid) {
		return;
	}

	// Correct the angle of the undoverline since we read it in the reverse direction
	// by offsetting it by 180 degress
	if (decodedUndoverline.wasReadInReverseOrder) {
		angle = angle < 0 ?
			angle + 180 :
			angle - 180;
	}

	float upAngleInDegrees =
		angle + (decodedUndoverline.isOverline ? 90 : -90);
	float upAngleInRadians = float(upAngleInDegrees * (2 * M_PI / 360.0));

	// pixels per mm the length of the overline in pixels of it's length in mm,
	// or, undoverlineLength / mmDieUndoverlineLength;
	double mmToPixels = double(undoverlineLength) / DieDimensionsMm::undoverlineLength;
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

	const auto x = rectEncompassingLine.center.x + pixelsFromCenterOfUnderlineToCenterOfDie * cos(upAngleInRadians);
	const auto y = rectEncompassingLine.center.y + pixelsFromCenterOfUnderlineToCenterOfDie * sin(upAngleInRadians);
	const cv::Point2f dieCenter = cv::Point2f(x, y);

	const auto textImage = copyRotatedRectangle(image, dieCenter, angle, textRegionSize);
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
		cv::imwrite(errBase + "-line.png", copyRotatedRectangle(image, rectEncompassingLine.center, rectEncompassingLine.angle, cv::Size2f( rectEncompassingLine.size.width + 2, rectEncompassingLine.size.height + 2)));
		cv::imwrite(errBase + ".png", letterImage);
	}
	if (d.confidence > 50.0f && decodedUndoverline.digit != d.charRead) {
		cv::imwrite("error-" + std::to_string(error++) + "-read-" + std::string(1, decodedUndoverline.digit) + "-as-" + std::string(1, d.charRead) + ".png", digitImage);
	}

}
