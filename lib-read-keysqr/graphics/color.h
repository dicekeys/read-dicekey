#pragma once

#include "cv.h"

class Color {
	public:
	unsigned char r;
	unsigned char g;
	unsigned char b;
  cv::Scalar scalar;

	Color(unsigned char _r, unsigned char _g, unsigned char _b) {
		r = _r;
		g = _g;
		b = _b;
    scalar = cv::Scalar(b, g, r);
	}

};
