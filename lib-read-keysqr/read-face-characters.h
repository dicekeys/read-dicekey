#pragma once
//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#include <float.h>
#include "graphics/cv.h"
#include "graphics/color.h"
#include "simple-ocr.h"

struct CharactersReadFromFaces {
	const OcrResult lettersMostLikelyFirst;
	const OcrResult digitsMostLikelyFirst;
};

CharactersReadFromFaces readCharactersOnFace(
	const cv::Mat& grayscaleImage,
	cv::Point2f faceCenter,
	float angleRadians,
	float pixelsPerFaceEdgeWidth,
	unsigned char whiteBlackThreshold,
	std::string writeErrorUnlessThisLetterIsRead = "",
	std::string writeErrorUnlessThisDigitIsRead = ""
);
