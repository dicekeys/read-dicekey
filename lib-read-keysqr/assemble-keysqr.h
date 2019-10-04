//  © 2019 Stuart Edward Schechter (Github: @uppajung)
#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include "find-undoverlines.h"
#include "find-faces.h"
#include "read-faces.h"

struct FacesOrderdWithMissingFacesInferredFromUnderlines {
	bool valid = false;
	std::vector<FaceRead> orderedFaces = {};
	// The angle of what was read on the page, without any conversion to have
	// the top left be the corner with the earliest letter in the alphabet
	float angleInRadiansNonCononicalForm = NAN;
	float pixelsPerFaceEdgeWidth;
};

FacesOrderdWithMissingFacesInferredFromUnderlines orderFacesAndInferMissingUndoverlines(
	const cv::Mat &grayscaleImage,
	const FacesAndStrayUndoverlinesFound& facesAndStrayUndoverlinesFound,
	float maxMmFromRowOrColumnLine = 1.0f // 1 mm
);