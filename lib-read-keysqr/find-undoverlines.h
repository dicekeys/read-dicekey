//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#pragma once

#include <float.h>
#include "graphics/cv.h"
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