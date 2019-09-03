#pragma once

#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

class RRectCorners {
	public:
		cv::Point bottomLeft;
		cv::Point topLeft;
		cv::Point topRight;
		cv::Point bottomRight;

	RRectCorners(const cv::RotatedRect rrect) {
		std::vector<cv::Point2f> points = std::vector<cv::Point2f>(4);
		rrect.points(points.data());
		float minX = points[0].x, maxX = points[0].x;
		float minY = points[0].y, maxY = points[0].y;
		for (size_t i = 1; i < 4; i++) {
			minX = std::min(minX, points[i].x);
			maxX = std::max(maxX, points[i].x);
			minY = std::min(minY, points[i].y);
			maxY = std::max(maxY, points[i].y);
		}
		const float width = maxX - minX;
		const float height = maxY - minY;
		// The order is bottomLeft, topLeft, topRight, bottomRight.
		rrect.points(points.data());
		if (width > height) {
			// sort left to right
			std::sort(points.begin(), points.end(), [](cv::Point2f a, cv::Point2f b) { return a.x < b.x; });
			topLeft     = points[0].y < points[1].y ? points[0] : points[1];
			bottomLeft  = points[0].y < points[1].y ? points[1] : points[0];
			topRight    = points[2].y < points[3].y ? points[2] : points[3];
			bottomRight = points[2].y < points[3].y ? points[3] : points[2];
		} else {
			// sort top to bottom
			std::sort(points.begin(), points.end(), [](cv::Point2f a, cv::Point2f b) { return a.y < b.y; });
			topLeft     = points[0].x < points[1].x ? points[0] : points[1];
			topRight    = points[0].x < points[1].x ? points[1] : points[0];
			bottomLeft  = points[2].x < points[3].x ? points[2] : points[3];
			bottomRight = points[2].x < points[3].x ? points[3] : points[2];
		}
		rrect.points(points.data());
	}
};

class RectangleDetected {
public:
	float contourArea;
	float area;
	float angleInDegrees;
	float longerSideLength;
	float shorterSideLength;
	int foundAtThreshold;
	cv::RotatedRect rotatedRect;
	cv::Point2f center;
	cv::Size2f size;
	std::vector<cv::Point2f> points = std::vector<cv::Point2f>(4);

	RectangleDetected(cv::RotatedRect rrect, float _contourArea, int _foundAtThreshold) {
		rotatedRect = rrect;
		rrect.points(points.data());
		contourArea = _contourArea;
		foundAtThreshold = _foundAtThreshold;
		center = rrect.center;
		size = rrect.size;
		angleInDegrees = rrect.angle;
		shorterSideLength = std::min(rrect.size.width, rrect.size.height);
		longerSideLength = std::max(rrect.size.width, rrect.size.height);
		area = shorterSideLength * longerSideLength;
	}

	RectangleDetected(cv::Point2f center, cv::Size2f size, float angle, float _contourArea, int _foundAtThreshold) :
		RectangleDetected(cv::RotatedRect(center, size, angle), _contourArea, _foundAtThreshold) {
	}

	RectangleDetected(std::vector<cv::Point> contour, int _foundAtThreshold) :
		RectangleDetected(cv::minAreaRect(contour), (float)cv::contourArea(contour), _foundAtThreshold) {
	};

	// Default constructor creates point of size 0 at origin.
	RectangleDetected() : RectangleDetected(cv::Point2f(0,0), cv::Size2f(), 0, 0, -1) {}

	// The deviation between this rectangle and square representing the face of a die
	float deviationFromNorm(float targetArea, float targetAngle, float targetShortToLongSideRatio = 1) {
		// The penalty for deviating from a squareness is based on the ratio of the two lengths
		float deviationFromSideRatio = (shorterSideLength / longerSideLength) / targetShortToLongSideRatio;
		if (deviationFromSideRatio < 1 && deviationFromSideRatio > 0) {
			deviationFromSideRatio = 1 / deviationFromSideRatio;
		}
		deviationFromSideRatio -= 1;
		float devationFromSideLengthRatioPenalty = 2.0f * deviationFromSideRatio;
		float deviationFromTargetArea = area < targetArea ?
			// Deviation penalty for falling short of target
			((targetArea / area) - 1) :
			// The consequences of capturing extra area are smaller,
			// so cut the penalty in half for those.
			(((area / targetArea) - 1) / 2);
		// The penalty from deviating from the target angle
		float deviationFromTargetAngle = 2.0f * float( abs( int(angleInDegrees - targetAngle) % 90)) / 90.0f;

		return devationFromSideLengthRatioPenalty + deviationFromTargetArea + deviationFromTargetAngle;
	};

	bool contains(const cv::Point2d &point) const {
		return cv::pointPolygonTest(points, point, false) >= 0;
	}

	bool overlaps(const RectangleDetected& otherRect) {
		return
			otherRect.contains(center) || contains(otherRect.center);
	}
};
