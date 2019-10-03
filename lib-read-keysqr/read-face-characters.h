#pragma once
//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#include <float.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include "simple-ocr.h"
#include "graphics/color.h"

struct CharactersReadFromFaces {
	const OcrResult lettersMostLikelyFirst;
	const OcrResult digitsMostLikelyFirst;
};

CharactersReadFromFaces readCharactersOnFace(
	const cv::Mat& imageColor,
	const cv::Mat& grayscaleImage,
	cv::Point2f faceCenter,
	float angleRadians,
	float pixelsPerFaceEdgeWidth,
	unsigned char whiteBlackThreshold,
	std::string writeErrorUnlessThisLetterIsRead = "",
	std::string writeErrorUnlessThisDigitIsRead = ""
);
