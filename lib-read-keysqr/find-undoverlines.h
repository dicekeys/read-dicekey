//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#pragma once

#include <float.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include "undoverline.h"

struct UnderlinesAndOverlines {
	std::vector<Undoverline> underlines;
	std::vector<Undoverline> overlines;
};

UnderlinesAndOverlines findReadableUndoverlines(
	const cv::Mat &grayscaleImage
);

Undoverline readUndoverline(
	const cv::Mat &grayscaleImage,
	const cv::RotatedRect &rectEncompassingLine
);