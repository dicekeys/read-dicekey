
#include <float.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include "vfunctional.h"
#include "rectangle.h"
#include "find-squares.h"
#include "value-clusters.h"


static double distance2d(const cv::Point2d& a, const cv::Point2d& b) {
	double dx = a.x - b.x;
	double dy = a.y - b.y;
	return sqrt(dx * dx + dy * dy);
}

static float distance2f(const cv::Point2f & a, const cv::Point2f & b) {
	float dx = a.x - b.x;
	float dy = a.y - b.y;
	return sqrt(dx * dx + dy * dy);
}

static float slope(const cv::Point & a, const cv::Point & b)
{
	return a.x == b.x ?
		FLT_MAX :
		// Note that since the y axis goes downward in OpenCV (unlike high school math),
		// we calculate delta y using a.y - b.y instead of b.y - a.y
		((float)a.y - b.y) / ((float)b.x - a.x);
}

struct DiceSquares {
	float slope;
	float angleRadians;
	float size;
	float distanceBetween;
	std::vector<RectangleDetected> squares;
};


#define _USE_MATH_DEFINES
#include <math.h>

static cv::Mat removeSlope(cv::Mat image, RectanglesFound & rects, float slope) {
	auto angleRadians = atan(slope);
	auto angleDegrees = angleRadians * 360 / (2 * (float)M_PI);
	const float sinAngle = sin(angleRadians);
	const float cosAngle = cos(angleRadians);

	cv::Mat rotatedImage;
	// Rotate around the top left for simlicity
	auto rotationMatrix = cv::getRotationMatrix2D(cv::Point2f(0, 0), angleDegrees, 1.0f);

	cv::warpAffine(image, rotatedImage, rotationMatrix, image.size());

	auto rotatePoint = [sinAngle, cosAngle](cv::Point2f point) -> cv::Point2f {
		auto x = (point.x * cosAngle) - (point.y * sinAngle);
		auto y = (point.x * sinAngle) + (point.y * cosAngle);
		return cv::Point2f(x, y);
	};

	// Rotate candidate dice
	//const auto area90thPercentile = percentile(vmap<RectangleDetected, float>(rects.candidateDiceSquares, [](RectangleDetected r) -> float {
	//	r.area;
	//	}), 90.0f);
	// const float squareEdgeLength = sqrt(area90thPercentile) * 1.15; // Add 15% to make sure we don't miss anything

	rects.candidateDiceSquares = vmap<RectangleDetected, RectangleDetected>(rects.candidateDiceSquares,
		[rotatePoint](RectangleDetected r) -> RectangleDetected {
			return RectangleDetected(rotatePoint(r.center), r.size, 0.0f, r.contourArea);
		});

	// Rotate candidate underlines
	rects.candidateUnderlineRectangles = vmap<RectangleDetected, RectangleDetected>(rects.candidateUnderlineRectangles,
		[rotatePoint](RectangleDetected r) -> RectangleDetected {
			return RectangleDetected(rotatePoint(r.center), r.size, 0.0f, r.contourArea);
		});
}

static DiceSquares filterAndOrderSquares(const std::vector<RectangleDetected> &squares)
{
	DiceSquares r;

	const float dieSize = 8; // 8mm die size
	const float gapBetweenDiceEdges = 1.8f; // 1.8mm
	const float dieSizeToDistBetweenDice = ((dieSize + gapBetweenDiceEdges) / dieSize);

	//
	// Calculate the median slope, and create functions to remove and restore the slope
	//
	r.slope = median(vmap<RectangleDetected, float>(squares, [](RectangleDetected r) { return slope(r.topLeft(), r.topRight()); }));
	r.angleRadians = atan(r.slope);

	auto removeSlope = [r](cv::Point2f point) -> cv::Point2f {
		// y = mx + b => b = y - mx, where b is the y intercept and m is the slope
		float y_intercept = point.y - r.slope * point.x;
		cv::Point2f y_intercept_point = cv::Point2d(0, y_intercept);
		// the length of the line from the y intercept
		float dist_from_y_intercept = distance2f(y_intercept_point, point);
		return cv::Point2f(dist_from_y_intercept, y_intercept);
	};
	auto restoreSlope = [r](cv::Point2f point) -> cv::Point2f {
		float dist_from_y_intercept = point.x;
		auto y = point.y + dist_from_y_intercept * sin(r.angleRadians);
		auto x = dist_from_y_intercept * cos(r.angleRadians);
		return cv::Point2f(x, y);
	};


	//
	// Sort dice based on their slope-adjusted location
	//
	auto medianLineLength = median(vmap<RectangleDetected, float>(squares, [](RectangleDetected r) { return (float)r.longerSideLength; }));
	r.size = medianLineLength;
	r.distanceBetween = medianLineLength * dieSizeToDistBetweenDice;
	float y_threshold = r.distanceBetween / 2;


	r.squares = squares;
	std::sort(r.squares.begin(), r.squares.end(), [removeSlope, y_threshold](const RectangleDetected a, const RectangleDetected b) {
		const auto adjusted_a_center = removeSlope(a.center);
		const auto adjusted_b_center = removeSlope(b.center);
		if (abs(adjusted_a_center.y - adjusted_b_center.y) > y_threshold) {
			// After adjusting for slope, there's a big enough difference in the Y axis
			// to sort based on the row (Y axis, or height from top to bottom)
			return adjusted_a_center.y < adjusted_b_center.y;
		}
		else {
			// Within the same row, sort by column (x axis)
			return adjusted_a_center.x < adjusted_b_center.x;
		}
		});

	//
	// Find the median distance between dice by taking the mean distance between
	// squares and their horizontal neighbors
	///
	std::vector<float> distancesBetweenCenters;
	for (uint i = 1; i < r.squares.size(); i++) {
		if (r.squares[i].center.x < r.squares[i - 1].center.x) {
			distancesBetweenCenters.push_back(distance2f(r.squares[i].center, r.squares[i - 1].center));
		}
	}
	if (distancesBetweenCenters.size() > 0) {
		r.distanceBetween = median(distancesBetweenCenters);
	}

	// Cluster the adjusted x and y values so we can look for
	// outliers, and re-create squares that the algorithm failed
	// to find.
	ValueClusters xClusters(r.distanceBetween * 0.33f);
	ValueClusters yClusters(r.distanceBetween * 0.33f);
	for (auto rect : r.squares) {
		auto adjustedCenter = removeSlope(rect.center);
		xClusters.addSample(adjustedCenter.x);
		xClusters.addSample(adjustedCenter.y);
	}

	// FIXME -- more work to adjust for square-generation errors

	// If more than 5 clusters, pick group of five that has most samples
	// Create locations of 25 die centers.
	// Ensure there is a square for each center, removing squares not associated with die location
	// Fill in missing squares.


	return r;
}

