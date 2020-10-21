#pragma once

//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#include <float.h>
#include "graphics/cv.h"
#include "graphics/color.h"
#include "read-faces.h"

// Colors are in BGR format
const Color colorNoErrorGreen(0, 192, 0);
const Color colorSmallErrorOrange = Color(192, 96, 0);
const Color colorBigErrorRed = Color(128, 0, 0);

Color errorMagnitudeToColor(unsigned errorMagnitude);

cv::Mat visualizeReadResults(
	cv::Mat &overlayImage,
	const std::vector<FaceRead> &faces,
	float angleInRadiansNonCanonicalForm,
	float pixelsPerFaceEdgeWidth
);