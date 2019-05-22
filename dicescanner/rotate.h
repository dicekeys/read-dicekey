
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

	return rotatedImage;
}


