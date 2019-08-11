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
#include "geometry.h"
#include "find-rectangles.h"
#include "rectangle.h"
#include "die-specification.h"
#include "rotate.h"
#include "ocr.h"
#include "sample-point.h"
#include "decode-die.h"


const float undoverlineWidthAsFractionOfLength = DieDimensionsMm::undoverlineThickness / DieDimensionsMm::undoverlineLength;
const float minWidthOverLength = undoverlineWidthAsFractionOfLength / 1.5f;
const float maxWidthOverLength = undoverlineWidthAsFractionOfLength * 1.5f;

static bool isRectangleShapedLikeUndoverline(RectangleDetected rect) {
	float shortToLongRatio = rect.shorterSideLength / rect.longerSideLength;
	return (
		shortToLongRatio >= minWidthOverLength &&
		shortToLongRatio <= maxWidthOverLength
		);
}


// returns sequence of squares detected on the image.
static std::vector<RectangleDetected> findCandidateUndoverlines(const cv::Mat& gray, int N = 13)
{
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
		// Calculate the modal slope of the surviving undoverlines (mod 90) so that we can
		// favor underlines with similar slopes
		// (mod 90 because undoverlines may be at one of four 90-degree rotations,
		//  and on a cicular line so that angles of 1 and 89 are distance 2, not distance 88)
		float targetAngleInDegrees = findPointOnCircularSignedNumberLineClosestToCenterOfMass(
			vmap<RectangleDetected, float>(candidateUnderOverLines,
				[](RectangleDetected r) -> float { return r.angleInDegrees; }),
			float(45));

		candidateUnderOverLines = removeOverlappingRectangles(candidateUnderOverLines, [areaHighPercentile, targetAngleInDegrees](RectangleDetected r) -> float {
			float deviationFromSideRatio = (r.shorterSideLength / r.longerSideLength) / undoverlineWidthAsFractionOfLength;
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
			const float angleDiff = distanceInModCircularRangeFromNegativeNToN(r.angleInDegrees, targetAngleInDegrees, float(90));
			float deviationFromTargetAngle = 2.0f * angleDiff;

			return devationFromSideLengthRatioPenalty + deviationFromTargetArea + deviationFromTargetAngle;
			});
	}

	return candidateUnderOverLines;
}

static Line undoverlineRectToLine(cv::Mat grayscaleImage, RectangleDetected lineBoundaryRect) {
	const int lineHeight = MAX(lineBoundaryRect.bottomLeft.y, lineBoundaryRect.bottomRight.y) - MIN(lineBoundaryRect.topLeft.y, lineBoundaryRect.topRight.y);
	const int lineWidth = MAX(lineBoundaryRect.topRight.x, lineBoundaryRect.bottomRight.x) - MIN(lineBoundaryRect.topLeft.x, lineBoundaryRect.bottomLeft.x);

	const bool isVertical = lineHeight > lineWidth;
	cv::Point2f start, end;
	float pixelStepX, pixelStepY;

	if (isVertical) {
		// Vertical (the line is closer to vertical than horizontal)
		// start from half way between top left and top right and proceed to half way from bottom left and bottom right
		start = midpoint2f(lineBoundaryRect.topLeft, lineBoundaryRect.topRight);
		end = midpoint2f(lineBoundaryRect.bottomLeft, lineBoundaryRect.bottomRight);
		// A step moving one Y pixel moves a fraction of a pixel in the x direction
		pixelStepY = 1;
		pixelStepX = ((end.x - start.x) / (end.y - start.y));
	}
	else {
		// Horizontal (the line is closer to horizontal than vertical)
		// start from half way between top left and bottom left and proceed from half way between top right and bottom right
		start = midpoint2f(lineBoundaryRect.topLeft, lineBoundaryRect.bottomLeft);
		end = midpoint2f(lineBoundaryRect.topRight, lineBoundaryRect.bottomRight);
		// A step moving one X pixel moves a fraction of a pixel in the Y direction
		pixelStepX = 1;
		pixelStepY = ((end.y - start.y) / (end.x - start.x));
	}
	assert(abs(pixelStepX) <= 1);
	assert(abs(pixelStepY) <= 1);


	// Take 25 samples of points between start and end so that we can find
	// theshold between light and dark.
	const std::vector<float> UndoverlineWhiteDarkSamplePoints = { 0.0f,
		0.03333f, 0.0666f, 0.1f,
		0.13333f, 0.1666f, 0.2f,
		0.23333f, 0.2666f, 0.3f,
		0.33333f, 0.3666f, 0.4f,
		0.43333f, 0.4666f, 0.5f,
		0.53333f, 0.5666f, 0.6f,
		0.63333f, 0.6666f, 0.7f,
		0.73333f, 0.7666f, 0.8f,
		0.83333f, 0.8666f, 0.9f,
		0.93333f, 0.9666f, 1.0f
	};
	const auto sampleSize = getNumberOfPixelsToSample(distance2f(start, end) / UndoverlineWhiteDarkSamplePoints.size());
	std::vector<uchar> pixelSamples = samplePointsAlongLine(grayscaleImage, start, end, UndoverlineWhiteDarkSamplePoints, sampleSize);
	uchar whiteBlackThreshold = bimodalThreshold(pixelSamples, 4, 4);

	// FUTURE
	// Recalculate center and angle by finding point halfway between the sides at
	// at 10%, 90% of distance.
	// We can then re-approximate start and end by 
	// Angle is angle between 10% and 90% point.
	// looking for top and bottom borders

	// Extend start and end .15mm to side in case we cut off the edge
	float fractionToExtend = 0.15f / DieDimensionsMm::undoverlineLength;
	float fractionToExtendH = (end.x - start.x) * fractionToExtend;
	float fractionToExtendV = (end.y - start.y) * fractionToExtend;
	// Okay to have points that go off graph as our sampling function
	// will ignore partial samples off image and, if entire sample is off image,
	// return white
	start.x -= fractionToExtendH;
	start.y -= fractionToExtendV;
	end.x += fractionToExtendH;
	end.y += fractionToExtendV;

	// Trim the start of the line by moving the start closer to the end,
	// until we reach the first black pixel
	cv::Point2f oldStart = start;
	cv::Point2f oldEnd = end;
	while (
		samplePoint(grayscaleImage, start, sampleSize) > whiteBlackThreshold &&
		isPointBetween2f(start.x + pixelStepX, start.y + pixelStepY, oldStart, oldEnd)
		) {
		// The starting point hasn't reached the black underline.
		start.x += pixelStepX;
		start.y += pixelStepY;
	}

	// Trim the end of the line by moving the end closer to the start,
	// until we reach the first black pixel	
	while (
		samplePoint(grayscaleImage, end, sampleSize) > whiteBlackThreshold &&
		isPointBetween2f(end.x - pixelStepX, end.y - pixelStepY, start, oldEnd)
		) {
		// The starting point hasn't reached the black underline.
		end.x -= pixelStepX;
		end.y -= pixelStepY;
	}
	return { start, end };
}


static uint readUndoverlineBits(cv::Mat grayscaleImage, Line undoverline, unsigned char &whiteBlackThreshold) {
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
	if (whiteBlackThreshold == 0 || whiteBlackThreshold == 255) {
		whiteBlackThreshold = bimodalThreshold(medianPixelValues, minNumberOf0s, minNumberOf1s);
	}
	uint binaryCodingReadForwardOrBackward = sampledPointsToBits(medianPixelValues, whiteBlackThreshold);
	return binaryCodingReadForwardOrBackward;
}


struct Undoverline {
	Line line;
	unsigned char whiteBlackThreshold;
	unsigned int binaryCodingReadForwardOrBackward;
	UndoverlineTypeOrientationAndEncoding decoded;
	cv::Point2f inferredDieCenter;
	DieFaceSpecification dieFaceInferred;
};


struct UnderlinesAndOverlines {
	std::vector<Undoverline> underlines;
	std::vector<Undoverline> overlines;
};


static cv::Mat highlightUndoverline(cv::Mat& imageColor, RectangleDetected line) {
	cv::Mat imageCopy = imageColor.clone();

	const cv::Point points[4] = {
		cv::Point(line.bottomLeft),
		cv::Point(line.topLeft),
		cv::Point(line.topRight),
		cv::Point(line.bottomRight),
	};

	const cv:: Point* ppoints[1] = {
		points
	};

	int npt[] = { 4 };


	polylines(imageCopy, ppoints, npt, 1, true, cv::Scalar(0, 0, 255), 2);
	return imageCopy;
	cv::imwrite("underline-within-image.png", imageCopy);
}


static UnderlinesAndOverlines findReadableUndoverlines(cv::Mat colorImage, cv::Mat grayscaleImage)
{
	const std::vector<RectangleDetected> candidateUndoverlineRects =
		findCandidateUndoverlines(grayscaleImage);

	std::vector<Undoverline> underlines;
	std::vector<Undoverline> overlines;

	for (auto rectEncompassingLine: candidateUndoverlineRects) {
		Line undoverline = undoverlineRectToLine(grayscaleImage, rectEncompassingLine);
		const float undoverlineLength = lineLength(undoverline);
		unsigned char whiteBlackThreshold = 0;
		const uint binaryCodingReadForwardOrBackward = readUndoverlineBits(grayscaleImage, undoverline, whiteBlackThreshold);
		const bool isVertical = abs(undoverline.end.x - undoverline.start.x) < abs(undoverline.end.y - undoverline.start.y);

		// FIXME -- remove debugging when all works.
		///Users/stuart/github/dice-scanner/
		 cv::imwrite("undoverline-highlighted.png", highlightUndoverline(colorImage, rectEncompassingLine));
		 const cv::Point2f center = midpointOfLine(undoverline);
		 const float lineLen = lineLength(undoverline);
		 const float angle = angleOfLineInSignedDegrees2f(undoverline);
		 cv::imwrite("underline-isolated.png", copyRotatedRectangle(
		 	grayscaleImage,
		 	center,
		 	angle,
		 	cv::Size2f( lineLen, lineLen * undoverlineWidthAsFractionOfLength )));

		const auto decoded = decodeUndoverline11Bits(binaryCodingReadForwardOrBackward, isVertical);
		if (!decoded.isValid) {
			continue;
		}

		DieFaceSpecification dieFace = decodeUndoverlineByte(decoded.isOverline, decoded.letterDigitEncoding);

		if (decoded.wasReadInReverseOrder) {
			undoverline = reverseLineDirection(undoverline);
		}

		float upAngleInRadians = angleOfLineInSignedRadians2f(undoverline) +
			(decoded.isOverline ? NinetyDegreesAsRadians : -NinetyDegreesAsRadians);

		// pixels per mm the length of the overline in pixels of it's length in mm,
		// or, undoverlineLength / mmDieUndoverlineLength;
		double mmToPixels = double(undoverlineLength) / DieDimensionsMm::undoverlineLength;

		float pixelsFromCenterOfUnderlineToCenterOfDie = float(
			DieDimensionsMm::centerOfUndoverlineToCenterOfDie *
			mmToPixels);

		const cv::Point2f lineCenter = midpointOfLine(undoverline);
		const auto x = lineCenter.x + pixelsFromCenterOfUnderlineToCenterOfDie * cos(upAngleInRadians);
		const auto y = lineCenter.y + pixelsFromCenterOfUnderlineToCenterOfDie * sin(upAngleInRadians);
		const cv::Point2f dieCenter = cv::Point2f(x, y);


		Undoverline thisUndoverline = {
			decoded.wasReadInReverseOrder ? reverseLineDirection(undoverline) : undoverline,
			whiteBlackThreshold,
			binaryCodingReadForwardOrBackward,
			decoded,
			dieCenter,
			dieFace
		};
		if (decoded.isOverline) {
			overlines.push_back(thisUndoverline);
		} else {
			underlines.push_back(thisUndoverline);
		}
	}

	// Sort underlines and overlines on y axis
	std::sort( underlines.begin(), underlines.end(), [](Undoverline a, Undoverline b) {return a.inferredDieCenter.y < b.inferredDieCenter.y; } );
	std::sort( overlines.begin(), overlines.end(), [](Undoverline a, Undoverline b) {return a.inferredDieCenter.y < b.inferredDieCenter.y; } );

	return {underlines, overlines};
}

