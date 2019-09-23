//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#pragma once

#include <float.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include <iostream>
#include "rectangle.h"

std::vector<RectangleDetected> removeOverlappingRectangles(
	std::vector<RectangleDetected> rectangles,
	std::function<float(RectangleDetected)> comparatorLowerIsBetter
);

// returns sequence of squares detected on the image.
std::vector<RectangleDetected> findRectangles(
	const cv::Mat &gray,
	uint N = 13,
	double minPerimeter = 50
);
