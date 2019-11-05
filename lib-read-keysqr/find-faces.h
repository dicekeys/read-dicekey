//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#pragma once

#include <float.h>
#include <vector>
#include "graphics/cv.h"
#include "read-faces.h"

struct FaceAndStrayUndoverlinesFound {
	std::vector<FaceUndoverlines> facesFound;
	std::vector<Undoverline> strayUndoverlines;
	float pixelsPerFaceEdgeWidth;
};

FaceAndStrayUndoverlinesFound findFacesAndStrayUndoverlines(
	const cv::Mat &grayscaleImage
);
