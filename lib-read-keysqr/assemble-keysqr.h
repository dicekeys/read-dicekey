//  © 2019 Stuart Edward Schechter (Github: @uppajung)
#pragma once

#include "graphics/cv.h"
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