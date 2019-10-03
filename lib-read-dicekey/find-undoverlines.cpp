//  © 2019 Stuart Edward Schechter (Github: @uppajung)
#include <float.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include <iostream>
#include <math.h>
#include <limits>
#include "utilities/vfunctional.h"
#include "utilities/statistics.h"
#include "graphics/geometry.h"
#include "graphics/find-rectangles.h"
#include "graphics/rectangle.h"
#include "graphics/rotate.h"
#include "graphics/sample-point.h"
#include "graphics/draw-rotated-rect.h"
#include "keysqr-element-face-specification.h"
#include "decode-die.h"
#include "undoverline.h"
#include "find-undoverlines.h"

const float minWidthOverLength = undoverlineWidthAsFractionOfLength / 1.5f;
const float maxWidthOverLength = undoverlineWidthAsFractionOfLength * 1.5f;

bool isRectangleShapedLikeUndoverline(const RectangleDetected* rect) {
	float shortToLongRatio = rect->shorterSideLength / rect->longerSideLength;
	return (
		shortToLongRatio >= minWidthOverLength &&
		shortToLongRatio <= maxWidthOverLength
		);
}

float findTighestModalAreaOfRects(const std::vector<RectangleDetected> &rects, int numberInMode = 35) {
	const int halfModeSize = MIN(int((rects.size() / 2) - 1), numberInMode / 2);
	std::vector<float> areas = vmap<RectangleDetected, float>(rects, [](const RectangleDetected *r) -> float {
		return r->area;
	});
	std::sort(areas.begin(), areas.end(), [](float a, float b) -> bool {return a < b;});
	float tightestModeRange = std::numeric_limits<float>::max();
	float areaAtTigghtestMode = NAN;
	int maxPossibleIndex = int(areas.size() - (halfModeSize + 2));
	for (int i = halfModeSize; i < maxPossibleIndex; i++) {
		const float modeRange = areas[i + halfModeSize] / areas[i - halfModeSize];

		if (modeRange < tightestModeRange ||
				// Just in case we found a very tight range of tiny things
				// (e.g., lots of 6x1 boxes that have small total area)
				(modeRange < 1.2 && areas[i] > 3 * areaAtTigghtestMode)
			) {
			tightestModeRange = modeRange;
			areaAtTigghtestMode = areas[i];
		}
	}
	return areaAtTigghtestMode;
}


// returns sequence of squares detected on the image.
std::vector<RectangleDetected> findCandidateUndoverlines(const cv::Mat& colorImage, const cv::Mat& grayscaleImage, int N = 13)
{
	std::vector<RectangleDetected> candidateUndoverlines = vfilter<RectangleDetected>(
		findRectangles(grayscaleImage, N), isRectangleShapedLikeUndoverline);

	if (candidateUndoverlines.size() > 25) {
		float tightestArea = findTighestModalAreaOfRects(candidateUndoverlines);
		float minArea = 0.75f * tightestArea;
		float maxArea = tightestArea / 0.75f;
		candidateUndoverlines = vfilter<RectangleDetected>(candidateUndoverlines, [minArea, maxArea](const RectangleDetected *r) {
			return  (r->area >= minArea && r->area <= maxArea);
			});

		// Calculate the modal slope of the surviving undoverlines (mod 90) so that we can
		// favor underlines with similar slopes
		// (mod 90 because undoverlines may be at one of four 90-degree rotations,
		//  and on a cicular line so that angles of 1 and 89 are distance 2, not distance 88)
		float targetAngleInDegrees = findPointOnCircularSignedNumberLineClosestToCenterOfMass(
			vmap<RectangleDetected, float>(candidateUndoverlines,
				[](const RectangleDetected *r) -> float { return r->angleInDegrees; }),
			float(45));

		candidateUndoverlines = removeOverlappingRectangles(candidateUndoverlines, [tightestArea, targetAngleInDegrees](RectangleDetected r) -> float {
			float deviationFromSideRatio = (r.shorterSideLength / r.longerSideLength) / undoverlineWidthAsFractionOfLength;
			if (deviationFromSideRatio < 1 && deviationFromSideRatio > 0) {
				deviationFromSideRatio = 1 / deviationFromSideRatio;
			}
			deviationFromSideRatio -= 1;
			float devationFromSideLengthRatioPenalty = 2.0f * deviationFromSideRatio;
			float deviationFromTargetArea = r.area < tightestArea ?
				// Deviation penalty for falling short of target
				((tightestArea / r.area) - 1) :
				// The consequences of capturing extra area are smaller,
				// so cut the penalty in half for those.
				(((r.area / tightestArea) - 1) / 2);
			// The penalty from deviating from the target angle
			const float angleDiff = distanceInModCircularRangeFromNegativeNToN(r.angleInDegrees, targetAngleInDegrees, float(90));
			float deviationFromTargetAngle = 2.0f * angleDiff;

			return devationFromSideLengthRatioPenalty + deviationFromTargetArea + deviationFromTargetAngle;
			});

			// Uncomment for debugging
			// for (auto const r : candidateUndoverlines) {
			// 	drawRotatedRect(colorImage, r.rotatedRect, cv::Scalar(255, 0, 255));
			// }
	}

	return candidateUndoverlines;
}


Line undoverlineRectToLine(const cv::Mat &grayscaleImage, const cv::RotatedRect &lineBoundaryRect) {
	const RRectCorners corners(lineBoundaryRect);
	const int lineHeight = std::max(corners.bottomLeft.y, corners.bottomRight.y) - std::min(corners.topLeft.y, corners.topRight.y);
	const int lineWidth = std::max(corners.topRight.x, corners.bottomRight.x) - std::min(corners.topLeft.x, corners.bottomLeft.x);

	const bool isVertical = lineHeight > lineWidth;
	cv::Point2f start, end;
	float pixelStepX, pixelStepY;

	if (isVertical) {
		// Vertical (the line is closer to vertical than horizontal)
		// start from half way between top left and top right and proceed to half way from bottom left and bottom right
		start = midpoint2f(corners.topLeft, corners.topRight);
		end = midpoint2f(corners.bottomLeft, corners.bottomRight);
		// A step moving one Y pixel moves a fraction of a pixel in the x direction
		pixelStepY = 1;
		pixelStepX = ((end.x - start.x) / (end.y - start.y));
	}
	else {
		// Horizontal (the line is closer to horizontal than vertical)
		// start from half way between top left and bottom left and proceed from half way between top right and bottom right
		start = midpoint2f(corners.topLeft, corners.bottomLeft);
		end = midpoint2f(corners.topRight, corners.bottomRight);
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
	float fractionToExtend = 0.15f / ElementDimensionsMm::undoverlineLength;
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
	const bool sampleThreeVerticalPoints = !isVertical;
	while (
		samplePoint(grayscaleImage, start, 3, sampleThreeVerticalPoints) > whiteBlackThreshold &&
		isPointBetween2f(start.x + pixelStepX, start.y + pixelStepY, oldStart, oldEnd)
		) {
		// The starting point hasn't reached the black underline.
		start.x += pixelStepX;
		start.y += pixelStepY;
	}

	// Trim the end of the line by moving the end closer to the start,
	// until we reach the first black pixel	
	while (
		samplePoint(grayscaleImage, end, 3, sampleThreeVerticalPoints) > whiteBlackThreshold &&
		isPointBetween2f(end.x - pixelStepX, end.y - pixelStepY, start, oldEnd)
		) {
		// The starting point hasn't reached the black underline.
		end.x -= pixelStepX;
		end.y -= pixelStepY;
	}
	return { start, end };
}

Undoverline readUndoverline(
	const cv::Mat &colorImage,
	const cv::Mat &grayscaleImage,
	const cv::RotatedRect &rectEncompassingLine
) {
	const Line undoverlineStartingAtImageLeft = undoverlineRectToLine(grayscaleImage, rectEncompassingLine);

	const std::vector<uchar> medianPixelValues = samplePointsAlongLine(
		grayscaleImage, undoverlineStartingAtImageLeft.start, undoverlineStartingAtImageLeft.end,
		ElementDimensionsFractional::dotCentersAsFractionOfUndoverline
	);

	// In finding a white/black threshold, the sampling should ensure
	// there are at least enough zeros an dones above/below the threshold.
	unsigned char whiteBlackThreshold = bimodalThreshold(medianPixelValues, MinNumberOfBlackDotsInUndoverline, MinNumberOfWhiteDotsInUndoverline);
	uint binaryCodingReadForwardOrBackward = sampledPointsToBits(medianPixelValues, whiteBlackThreshold);

	return Undoverline(rectEncompassingLine, undoverlineStartingAtImageLeft, whiteBlackThreshold, binaryCodingReadForwardOrBackward);
}

UnderlinesAndOverlines findReadableUndoverlines(
	const cv::Mat &colorImage,
	const cv::Mat &grayscaleImage
) {
	const std::vector<RectangleDetected> candidateUndoverlineRects =
		findCandidateUndoverlines(colorImage, grayscaleImage);

	std::vector<Undoverline> underlines;
	std::vector<Undoverline> overlines;

	for (const RectangleDetected &rectEncompassingLine: candidateUndoverlineRects) {
		const Undoverline undoverline = readUndoverline(colorImage, grayscaleImage, rectEncompassingLine.rotatedRect);

		if (undoverline.found && undoverline.determinedIfUnderlineOrOverline) {
			if (undoverline.isOverline) {
				overlines.push_back(undoverline);
			} else {
				underlines.push_back(undoverline);
			}
		}
	}

	// Sort underlines and overlines on y axis
	std::sort( underlines.begin(), underlines.end(), [](Undoverline a, Undoverline b) {return a.inferredDieCenter.y < b.inferredDieCenter.y; } );
	std::sort( overlines.begin(), overlines.end(), [](Undoverline a, Undoverline b) {return a.inferredDieCenter.y < b.inferredDieCenter.y; } );

	return {underlines, overlines};
}

