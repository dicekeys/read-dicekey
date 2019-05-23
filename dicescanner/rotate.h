
#pragma once

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


#define _USE_MATH_DEFINES
#include <math.h>

static cv::Mat rotateImageAndRectanglesFound(cv::Mat image, RectanglesFound& rects, float slope) {
	auto angleRadians = atan(slope);

	const float radians45Degrees = 45.0f * 2.0f * (float)M_PI / 360.0f;
	const float radians90Degrees = 90.0f * 2.0f * (float)M_PI / 360.0f;
	while (angleRadians > radians45Degrees) {
		angleRadians -= radians90Degrees;
	}
	while (angleRadians < -radians45Degrees) {
		angleRadians += radians90Degrees;
	}
	auto angleDegrees = angleRadians * 360 / (2 * (float)M_PI);

	cv::Point2f center = cv::Point2f(image.size[0] / 2.0f, image.size[1] / 2.0f);
	const float sinAngle = sin(angleRadians);
	const float cosAngle = cos(angleRadians);

	cv::Mat rotatedImage;
	// Rotate around the center of the image to remove the angle
	auto rotationMatrix = cv::getRotationMatrix2D(center, -angleDegrees, 1.0f);

	cv::warpAffine(image, rotatedImage, rotationMatrix, image.size());

	auto rotatePoint = [center, sinAngle, cosAngle](cv::Point2f point) -> cv::Point2f {
		// Get point relative to center
		auto x_c = point.x - center.x;
		auto y_c = point.y - center.y;

		// Rotate
		auto x = (x_c * cosAngle) - (y_c * sinAngle);
		auto y = (x_c * sinAngle) + (y_c * cosAngle);

		// Get point relative to 0,0 by adding back in center
		return cv::Point2f(x + center.x, y + center.y);
	};

	// Rotate candidate dice
	//const auto area90thPercentile = percentile(vmap<RectangleDetected, float>(rects.candidateDiceSquares, [](RectangleDetected r) -> float {
	//	r.area;
	//	}), 90.0f);
	// const float squareEdgeLength = sqrt(area90thPercentile) * 1.15; // Add 15% to make sure we don't miss anything

	rects.candidateDiceSquares = vmap<RectangleDetected, RectangleDetected>(rects.candidateDiceSquares,
		[rotatePoint, angleDegrees](RectangleDetected r) -> RectangleDetected {
			return RectangleDetected(rotatePoint(r.center), r.size, r.angle + angleDegrees, r.contourArea);
		});

	// Rotate candidate underlines
	rects.candidateUnderlineRectangles = vmap<RectangleDetected, RectangleDetected>(rects.candidateUnderlineRectangles,
		[rotatePoint, angleDegrees](RectangleDetected r) -> RectangleDetected {
			return RectangleDetected(rotatePoint(r.center), r.size, r.angle + angleDegrees, r.contourArea);
		});

	return rotatedImage;
}


