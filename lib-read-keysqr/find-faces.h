//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#pragma once

#include <float.h>
#include <vector>
#include "graphics/cv.h"
#include "read-faces.h"

struct FacesAndStrayUndoverlinesFound {
	std::vector<FaceRead> facesFound;
	std::vector<Undoverline> strayUndoverlines;
	float pixelsPerFaceEdgeWidth;
};

FacesAndStrayUndoverlinesFound findFacesAndStrayUndoverlines(
	const cv::Mat &grayscaleImage
);
