#pragma once

#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

//// helper function:
//// finds a cosine of angle between vectors
//// from pt0->pt1 and from pt0->pt2
//static float angle(cv::Point pt1, cv::Point pt2, cv::Point pt0)
//{
//	float dx1 = (float)pt1.x - pt0.x;
//	float dy1 = (float)pt1.y - pt0.y;
//	float dx2 = (float)pt2.x - pt0.x;
//	float dy2 = (float)pt2.y - pt0.y;
//	return (dx1 * dx2 + dy1 * dy2) / sqrt((dx1 * dx1 + dy1 * dy1) * (dx2 * dx2 + dy2 * dy2));
//}

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

class RectangleDetected {
public:
	float contourArea;
	float area;
	float angle;
	float longerSideLength;
	float shorterSideLength;
	cv::Point2d center;
	cv::Size2f size;
	std::vector<cv::Point2f> points = std::vector<cv::Point2f>(4);

	// Calculated by RotatedRect.points(), which returns in order
	// bottomLeft, topLeft, topRight, bottomRight.
	cv::Point bottomLeft() { return points[0]; };
	cv::Point topLeft() { return points[1]; };
	cv::Point topRight() { return points[2]; };
	cv::Point bottomRight() { return points[3]; };

	RectangleDetected(cv::RotatedRect rrect, float _contourArea) {
		contourArea = _contourArea;
		center = rrect.center;
		size = rrect.size;
		angle = rrect.angle;
		// The order is bottomLeft, topLeft, topRight, bottomRight.
		rrect.points(points.data());
		shorterSideLength = std::min(rrect.size.width, rrect.size.height);
		longerSideLength = std::max(rrect.size.width, rrect.size.height);
		area = shorterSideLength * longerSideLength;
	}

	RectangleDetected(cv::Point2f center, cv::Size2f size, float angle, float _contourArea) :
		RectangleDetected(cv::RotatedRect(center, size, angle), _contourArea) {
	}

	RectangleDetected(std::vector<cv::Point> contour) :
		RectangleDetected(cv::minAreaRect(contour), (float)cv::contourArea(contour)) {
	};

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
		float deviationFromTargetAngle = 2.0f * abs(angle - targetAngle) / 90;

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
