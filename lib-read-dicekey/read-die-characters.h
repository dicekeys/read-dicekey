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

struct DieCharactersRead {
	const OcrResult lettersMostLikelyFirst;
	const OcrResult digitsMostLikelyFirst;
};

DieCharactersRead readDieCharacters(
	const cv::Mat& imageColor,
	const cv::Mat& grayscaleImage,
	cv::Point2f dieCenter,
	float angleRadians,
	float mmToPixels,
	unsigned char whiteBlackThreshold,
	std::string writeErrorUnlessThisLetterIsRead = "",
	std::string writeErrorUnlessThisDigitIsRead = ""
);
