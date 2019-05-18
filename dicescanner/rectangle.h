#pragma once

#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

// helper function:
// finds a cosine of angle between vectors
// from pt0->pt1 and from pt0->pt2
static double angle(cv::Point pt1, cv::Point pt2, cv::Point pt0)
{
	double dx1 = (double)pt1.x - pt0.x;
	double dy1 = (double)pt1.y - pt0.y;
	double dx2 = (double)pt2.x - pt0.x;
	double dy2 = (double)pt2.y - pt0.y;
	return (dx1 * dx2 + dy1 * dy2) / sqrt((dx1 * dx1 + dy1 * dy1) * (dx2 * dx2 + dy2 * dy2) + 1e-10);
}

static cv::Point2d getCenter(std::vector<cv::Point> &points)
{
	float x = 0;
	float y = 0;
	for (auto& point : points) {
		x += point.x;
		y += point.y;
	}
	x /= points.size();
	y /= points.size();
	return cv::Point2d(x, y);
}

class Rectangle {
public:
	std::vector<cv::Point> points;
	cv::Point topLeft;
	cv::Point topRight;
	cv::Point bottomLeft;
	cv::Point bottomRight;
	double area;
	double maxCos;
	std::vector<double> sideLengths;
	double maxSideLength;
	double minSideLength;
	cv::Point2d center;
	// quality is used to determine which of two overlapping rectangles
	// is better, prefering straighter corners and more area.
	double qualityLowerIsBetter;

	Rectangle(std::vector<cv::Point> &fromPoints) {
		points = fromPoints;
		std::sort(points.begin(), points.end(), [](cv::Point a, cv::Point b) {return a.y < b.y; });
		// The first two points are the top two.  Sort them top left and top right
		if (points[0].x > points[1].x) {
			swap(points[0], points[1]);
		}
		// The second set of two points are the bottom two.  Sort them right then left.
		if (points[2].x < points[3].x) {
			swap(points[2], points[3]);
		}
		topLeft = points[0];
		topRight = points[1];
		bottomRight = points[2];
		bottomLeft = points[3];

		center = getCenter(points);
		area = contourArea(points);
		maxCos = 0;
		sideLengths.push_back(
			maxSideLength = minSideLength = norm(points[3] - points[0])
		);
		for (int p = 1; p < 4; p++) {
			double sideLength = norm(points[p] - points[(p + 1) % 4]);
			maxSideLength = MAX(maxSideLength, sideLength);
			minSideLength = MIN(minSideLength, sideLength);
			sideLengths.push_back(sideLength);
		}
		for (int j = 2; j < 5; j++)
		{
			// find the maximum cosine of the angle between joint edges
			double cosine = fabs(angle(points[j % 4], points[j - 2], points[j - 1]));
			maxCos = MAX(maxCos, cosine);
		}
		// qualityLowerIsBetter = area > 0 ? maxCos / area : 1;
		qualityLowerIsBetter = area > 0 ? maxCos / pow(area, 4) : 1;
	}

	bool contains(const cv::Point2d &point) const {
		return cv::pointPolygonTest(points, point, false) >= 0;
	}

	bool overlaps(const Rectangle& otherRect) {
		return
			otherRect.contains(center) || contains(otherRect.center);
	}
};
