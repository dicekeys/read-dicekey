//  © 2019 Stuart Edward Schechter (Github: @uppajung)
#pragma once

#include "graphics/cv.h"
#include "find-undoverlines.h"
#include "find-faces.h"
#include "read-faces.h"

class FacesOrderedWithMissingFacesInferredFromUnderlines {
	public:
	bool valid = false;
	std::vector<FaceRead> orderedFaces = {};
	// The angle of what was read on the page, without any conversion to have
	// the top left be the corner with the earliest letter in the alphabet
	float angleInRadiansNonCanonicalForm = NAN;
	float pixelsPerFaceEdgeWidth;

	FacesOrderedWithMissingFacesInferredFromUnderlines() {}

	FacesOrderedWithMissingFacesInferredFromUnderlines(
		std::vector<FaceRead> _orderedFaces,
		float _angleInRadiansNonCanonicalForm,
		float _pixelsPerFaceEdgeWidth
	) {
		valid = true;
		orderedFaces = _orderedFaces;
		angleInRadiansNonCanonicalForm = _angleInRadiansNonCanonicalForm;
		pixelsPerFaceEdgeWidth = _pixelsPerFaceEdgeWidth;
	}
};

FacesOrderedWithMissingFacesInferredFromUnderlines orderFacesAndInferMissingUndoverlines(
	const cv::Mat &grayscaleImage,
	const FacesAndStrayUndoverlinesFound& facesAndStrayUndoverlinesFound,
	float maxMmFromRowOrColumnLine = 1.0f // 1 mm
);