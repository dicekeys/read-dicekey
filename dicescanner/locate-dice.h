
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


static double distance2d(const Point2d &a, const Point2d &b) {
	double dx = a.x - b.x;
	double dy = a.y - b.y;
	return sqrt( dx * dx + dy * dy);
}


static double slope(const Point &a, const Point &b)
{
	return a.x == b.x ?
		DBL_MAX :
		((double)b.y - a.y) /((double)b.x - a.x);
}

struct DiceSquares {
    double slope;
    double angleRadians;
    double size;
    double distanceBetween;
    std::vector<Rectangle> squares;
};

static DiceSquares filterAndOrderSquares(const vector<Rectangle> &squares)
{
    DiceSquares r;

	const double dieSize = 8; // 8mm die size
	const double gapBetweenDiceEdges = 1.8; // 1.8mm
	const double dieSizeToDistBetweenDice = ((dieSize + gapBetweenDiceEdges) / dieSize);

	//
	// Calculate the median slope, and create functions to remove and restore the slope
	//
	r.slope = median(vmap<Rectangle, double>(squares, [](Rectangle r) { return slope(r.topLeft, r.topRight); }));
	r.angleRadians = atan(r.slope);

	auto removeSlope = [r](Point2d point) -> Point2d {
		// y = mx + b => b = y - mx, where b is the y intercept and m is the slope
		double y_intercept = point.y - r.slope * point.x;
		Point2d y_intercept_point = Point2d(0, y_intercept);
		// the length of the line from the y intercept
		double dist_from_y_intercept = distance2d(y_intercept_point, point);
		return Point2d(dist_from_y_intercept, y_intercept);
	};
	auto restoreSlope = [r](Point2d point) -> Point2d {
		double dist_from_y_intercept = point.x;
		auto y = point.y + dist_from_y_intercept * sin(r.angleRadians);
		auto x = dist_from_y_intercept * cos(r.angleRadians);
		return Point2d(x, y);
	};


	//
	// Sort dice based on their slope-adjusted location
	//
	auto medianLineLength = median(vmap<Rectangle, double>(squares, [](Rectangle r) { return (double)r.maxSideLength; }));
	r.size = medianLineLength;
	r.distanceBetween = medianLineLength * dieSizeToDistBetweenDice;
	double y_threshold = r.distanceBetween  / 2;


	r.squares = squares;
	std::sort(r.squares.begin(), r.squares.end(), [removeSlope, y_threshold](const Rectangle a, const Rectangle b) {
		const auto adjusted_a_center = removeSlope(a.center);
		const auto adjusted_b_center = removeSlope(b.center);
		if (abs(adjusted_a_center.y - adjusted_b_center.y) > y_threshold) {
			// After adjusting for slope, there's a big enough difference in the Y axis
			// to sort based on the row (Y axis, or height from top to bottom)
			return adjusted_a_center.y < adjusted_b_center.y;
		} else {
			// Within the same row, sort by column (x axis)
			return adjusted_a_center.x < adjusted_b_center.x;
		}
	});

	//
	// Find the median distance between dice by taking the mean distance between
	// squares and their horizontal neighbors
	///
	vector<double> distancesBetweenCenters;
	for (uint i = 1; i < r.squares.size(); i++) {
		if (r.squares[i].center.x < r.squares[i-1].center.x) {
			distancesBetweenCenters.push_back( distance2d(r.squares[i].center, r.squares[i-1].center ));
		}
	}
	if (distancesBetweenCenters.size() > 0) {
		r.distanceBetween = median(distancesBetweenCenters);
	}

	// Cluster the adjusted x and y values so we can look for
	// outliers, and re-create squares that the algorithm failed
	// to find.
	ValueClusters xClusters(r.distanceBetween * 0.33);
	ValueClusters yClusters(r.distanceBetween * 0.33);
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

