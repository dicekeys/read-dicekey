#pragma once
//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#include <float.h>
#include "graphics/cv.h"
#include "graphics/color.h"

void writeFaceCharacters(
	cv::Mat& imageColor,
	cv::Point2f faceCenter,
	float angleInRadians,
	float pixelsPerMm,
	char letter,
	char digit,
	Color letterColor,
	Color digitColor
);
