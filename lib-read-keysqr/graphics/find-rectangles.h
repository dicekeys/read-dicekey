//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#pragma once

#include <float.h>
#include "cv.h"
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
