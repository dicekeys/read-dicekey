
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
#include "rotate.h"
#include "distance.h"

#include <iostream> // FIXME after this works

// static float slope(const cv::Point & a, const cv::Point & b)
// {
// 	return a.x == b.x ?
// 		FLT_MAX :
// 		// Note that since the y axis goes downward in OpenCV (unlike high school math),
// 		// we calculate delta y using a.y - b.y instead of b.y - a.y
// 		((float)a.y - b.y) / ((float)b.x - a.x);
// };


struct DieFound {
	cv::Mat image;
	bool underlineFound;
	RectangleDetected underline;
	cv::Mat textImage;
	int orientationInDegrees;
};


static std::vector<cv::Mat> findDice(cv::Mat &image, std::vector<RectangleDetected> &candidateDiceSquares, float &approxPixelsPerMm)
{
	std::vector<cv::Mat> r;

	const float mmBetweenDieCenter = 8.0f + 1.8f;
	const float dieSizeAsFractionOfDistanceBetweenDice = 8.0f / mmBetweenDieCenter;
	const float mmFromDieCenterToUnderlineCenter = 2.15f;
	const float maxMmFromDieCenterToUnderlineCenter = 2.0f * mmFromDieCenterToUnderlineCenter;

	const float dieSize = 8; // 8mm die size
	const float gapBetweenDiceEdges = 1.8f; // 1.8mm
	const float dieSizeToDistBetweenDice = ((dieSize + gapBetweenDiceEdges) / dieSize);

	//
	// Sort dice based on their slope-adjusted location
	//
	auto medianLineLength = median(vmap<RectangleDetected, float>(candidateDiceSquares, [](RectangleDetected r) { return (float)r.longerSideLength; }));
	auto ssquares = candidateDiceSquares;
	//r.size = medianLineLength;
	auto distanceBetween = medianLineLength * dieSizeToDistBetweenDice;
	float y_threshold = distanceBetween / 2;

	//r.squares = squares;
	std::sort(ssquares.begin(), ssquares.end(), [y_threshold](const RectangleDetected a, const RectangleDetected b) {
		if (abs(a.center.y - b.center.y) > y_threshold) {
			// After adjusting for slope, there's a big enough difference in the Y axis
			// to sort based on the row (Y axis, or height from top to bottom)
			return a.center.y < b.center.y;
		}
		else {
			// Within the same row, sort by column (x axis)
			return a.center.x < b.center.x;
		}
		});

	//
	// Find the median distance between dice by taking the mean distance between
	// squares and their horizontal neighbors
	///
	std::vector<float> distancesBetweenCenters;
	for (uint i = 1; i < ssquares.size(); i++) {
		if (ssquares[i].center.x < ssquares[i - 1].center.x) {
			distancesBetweenCenters.push_back(distance2f(ssquares[i].center, ssquares[i - 1].center));
		}
	}
	if (distancesBetweenCenters.size() > 0) {
		distanceBetween = median(distancesBetweenCenters);
	}

	// Cluster the adjusted x and y values so we can look for
	// outliers, and re-create squares that the algorithm failed
	// to find.
	ValueClusters xClusters(distanceBetween * 0.15f);
	ValueClusters yClusters(distanceBetween * 0.15f);
	for (auto rect : ssquares) {
		xClusters.addSample(rect.center.x);
		yClusters.addSample(rect.center.y);
	}

	auto xClusterV = vfilter<ValueCluster>(xClusters.clusterVector(), [](ValueCluster c) -> bool { return c.samples.size() >= 3; });
	auto yClusterV = vfilter<ValueCluster>(yClusters.clusterVector(), [](ValueCluster c) -> bool { return c.samples.size() >= 3; });

	auto xClusterMedians = vmap<ValueCluster, float>(xClusterV, [](ValueCluster c) -> float { return c.median;  });
	auto yClusterMedians = vmap<ValueCluster, float>(yClusterV, [](ValueCluster c) -> float { return c.median;  });

	std::vector<float> xDists, yDists;
	for (uint i = 1; i < xClusterMedians.size(); i++) {
		xDists.push_back (xClusterMedians[i] - xClusterMedians[i - 1]);
	}
	for (uint i = 1; i < yClusterMedians.size(); i++) {
		yDists.push_back(yClusterMedians[i] - yClusterMedians[i - 1]);
	}

	float centerX = median(xClusterMedians);
	float centerY = median(yClusterMedians);
	float distBetweenX = median(xDists);
	float distBetweenY = median(yDists);
	float xSize = distBetweenX * dieSizeAsFractionOfDistanceBetweenDice;
	float ySize = distBetweenY * dieSizeAsFractionOfDistanceBetweenDice;
	float halfXSize = xSize / 2;
	float halfYSize = ySize / 2;

	approxPixelsPerMm =
		( (distBetweenX + distBetweenY) / 2 ) / // pixels between die centers (averaging x and y distances)
		mmBetweenDieCenter; // over the number of mm between die centers in our box design


	const float maxDistanceDieCenterToUnderlineCenter = approxPixelsPerMm *
		maxMmFromDieCenterToUnderlineCenter;

	for (int die = 0; die < 25; die++) {
		int y = die / 5;
		int x = die % 5;
		float cx = centerX + (x - 2) * distBetweenX;
		float left = cx - halfXSize;
		// float right = cx + halfXSize;
		float cy = centerY + (y - 2) * distBetweenY;
		float top = cy - halfYSize;
		cv::Point2f dieCenter(cx, cy);

		// find the closest underline
		int indexOfClosestUnderline = -1;
		float closestDistance = INFINITY;

		cv::Rect myROI((int) left, (int) top, (int) xSize, (int) ySize);
		cv::Mat dieImage = image(myROI);
		
		r.push_back(dieImage);

		// FIXME -- try to orient
	}

	// FIXME -- more work to adjust for square-generation errors

	// If more than 5 clusters, pick group of five that has most samples
	// Create locations of 25 die centers.
	// Ensure there is a square for each center, removing squares not associated with die location
	// Fill in missing squares.


	return r;
}

