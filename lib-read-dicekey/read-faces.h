#pragma once

//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#include <string>
#include <vector>
#include <limits>
#include <chrono>

#include "undoverline.h"
#include "face-read.h"
#include "simple-ocr.h"

struct ReadFaceResult {
//	public:
	bool success;
	std::vector<FaceRead> faces;
	float angleInRadiansNonCanonicalForm;
	float pixelsPerFaceEdgeWidth;
	std::vector<FaceRead> strayFaces;
//	std::vector<Undoverline> strayUndoverlines;
};

ReadFaceResult readFaces(
	const cv::Mat &grayscaleImage,
	bool outputOcrErrors = false
);
