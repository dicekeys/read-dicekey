#include <float.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>


static float slope(const cv::Point & a, const cv::Point & b)
{
	return a.x == b.x ?
		FLT_MAX :
		// Note that since the y axis goes downward in OpenCV (unlike high school math),
		// we calculate delta y using a.y - b.y instead of b.y - a.y
		((float)a.y - b.y) / ((float)b.x - a.x);
};

