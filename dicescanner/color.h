#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

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
