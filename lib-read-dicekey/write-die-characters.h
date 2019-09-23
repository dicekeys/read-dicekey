#pragma once
//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#include <float.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include "graphics/color.h"


void writeDieCharacters(
	cv::Mat& imageColor,
	cv::Point2f dieCenter,
	float angleInRadians,
	float pixelsPerMm,
	char letter,
	char digit,
	Color letterColor,
	Color digitColor
);