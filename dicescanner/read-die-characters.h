#pragma once

#include <float.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include <iostream>
#include <math.h>
#include "vfunctional.h"
#include "statistics.h"
#include "geometry.h"
#include "die-specification.h"
#include "simple-ocr.h"
#include "rotate.h"

struct DieCharactersRead {
	char letter= '\0';
	char digit = '\0';
};

static DieCharactersRead readDieCharacters(
	const cv::Mat &imageColor,
	const cv::Mat &grayscaleImage,
	cv::Point2f dieCenter,
	float angleRadians,
	float mmToPixels,
	unsigned char whiteBlackThreshold,
	char writeErrorUnlessThisLetterIsRead = 0,
	char writeErrorUnlessThisDigitIsRead = 0
) {
	// Rotate to remove the angle of the die
	const float degreesToRotateToRemoveAngleOfDie = radiansToDegrees(angleRadians);
	int textHeightPixels = int(ceil(DieDimensionsMm::textRegionHeight * mmToPixels));
	// FIXME -- constant in next line is a hack
	int textWidthPixels = int(ceil(DieDimensionsMm::textRegionWidth * 0.825f * mmToPixels));
	// Use an even text region width so we can even split it in two at the center;
	if ((textWidthPixels % 2) == 1) {
		textWidthPixels += 1;
	}
	cv::Size textRegionSize = cv::Size(textWidthPixels, textHeightPixels);
	cv::Mat textEdges;
	const uchar valueRepresentingBlack = 255;

	const auto textImage = copyRotatedRectangle(grayscaleImage, dieCenter, degreesToRotateToRemoveAngleOfDie, textRegionSize);
	// Previously, we blurred image before thresholding.  It may make sense to do that
	// again when we get back images from real dice, so leaving this code here.
	// cv::Mat textBlurred
	// cv::medianBlur(textImage, textBlurred, 3);
	// cv::threshold(textBlurred, textEdges, whiteBlackThreshold, valueRepresentingBlack, cv::THRESH_BINARY);
	// at which point we'd remove the line below
	cv::threshold(textImage, textEdges, whiteBlackThreshold, valueRepresentingBlack, cv::THRESH_BINARY);

	// Setup a rectangle to define your region of interest
	int charWidth = int( ( textRegionSize.width - round(DieDimensionsMm::spaceBetweenLetterAndDigit * mmToPixels) ) / 2);
	const cv::Rect letterRect(0, 0, charWidth, textRegionSize.height);
	const cv::Rect digitRect(textRegionSize.width - charWidth, 0, charWidth, textRegionSize.height);
	auto letterImage = textEdges(letterRect);
	auto digitImage = textEdges(digitRect);

	// FIXME -- remove after development debugging
	//cv::imwrite("text-region.png", textImage);
	// //	cv::imwrite("text-blurred.png", textBlurred);
	// cv::imwrite("text-edges.png", textEdges);
	// cv::imwrite("letter.png", letterImage);
	// cv::imwrite("digit.png", digitImage);

	const int letterIndex = readLetter(letterImage);
	const int digitIndex = readDigit(digitImage);

	char letter = letterIndex < 0 ? '\0' : DieLetters[letterIndex];
	char digit = digitIndex < 0 ? '\0' : DieDigits[digitIndex];

	// FIXME -- remove after development debugging
	static int error = 1;
	if (writeErrorUnlessThisLetterIsRead != 0 && writeErrorUnlessThisLetterIsRead != letter) {
		cv::imwrite("error-" + std::to_string(error++) + "-read-" + std::string(1, writeErrorUnlessThisLetterIsRead) + "-as-" + std::string(1, dashIfNull(letter)) + ".png", letterImage);
	}
	if (writeErrorUnlessThisDigitIsRead != 0 && writeErrorUnlessThisDigitIsRead != digit) {
		cv::imwrite("error-" + std::to_string(error++) + "-read-" + std::string(1, writeErrorUnlessThisDigitIsRead) + "-as-" + std::string(1, dashIfNull(digit)) + ".png", digitImage);
	}

	return {letter, digit};
}
