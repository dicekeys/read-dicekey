//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#pragma once

#include <float.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <vector>
#include "read-faces.h"

struct FacesAndStrayUndoverlinesFound {
	std::vector<FaceRead> facesFound;
	std::vector<Undoverline> strayUndoverlines;
	float pixelsPerFaceEdgeWidth;
};

FacesAndStrayUndoverlinesFound findFacesAndStrayUndoverlines(
	const cv::Mat &colorImage,
	const cv::Mat &grayscaleImage
);
