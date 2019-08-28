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
#include "inconsolata-700.h"

// FIXME -- constant in next line is a hack derived by trial and error
// and would be better to derive in a more formal way
const float textWidthAdjustmentMultiplier = 0.825f;

struct DieCharactersRead {
	const OcrResult lettersMostLikelyFirst;
	const OcrResult digitsMostLikelyFirst;
};

static void writeDieCharacters(
	cv::Mat& imageColor,
	cv::Point2f dieCenter,
	float angleInRadians,
	float pixelsPerMm,
	char letter,
	char digit,
	bool possibleErrors = 0
) {
	const float textHeightDestinationPixels = DieDimensionsMm::textRegionHeight * pixelsPerMm;
	const float textWidthDestinationPixels = DieDimensionsMm::textRegionWidth * textWidthAdjustmentMultiplier * pixelsPerMm;
	const float destinationPixelsBetweenLetterAndDigit = DieDimensionsMm::spaceBetweenLetterAndDigit * pixelsPerMm;
	float charWidthDestinationPixels = (textWidthDestinationPixels - destinationPixelsBetweenLetterAndDigit) / 2;

	float letterLeftRelativeToDieCenterInDestinationPixels = -(textWidthDestinationPixels / 2);
	float digitLeftRelativeToDieCenterInDestinationPixels = (destinationPixelsBetweenLetterAndDigit / 2);
	float letterAndDigitTopRelativeToDieCenterInDestinationPixels = -textHeightDestinationPixels / 2;


	const float centerSpaceInOriginPixels = DieDimensionsMm::spaceBetweenLetterAndDigit * pixelsPerMm *
		Inconsolata700::outlineCharWidthInPixels / charWidthDestinationPixels;
	const float textTopInOriginPixels = -float(Inconsolata700::outlineCharHeightInPixels) / 2;
	const float letterLeftInOriginPixels = -(Inconsolata700::outlineCharWidthInPixels + centerSpaceInOriginPixels/2);
	const float digitLeftInOriginPixels = centerSpaceInOriginPixels / 2;
	
	const float deltaXFraction = charWidthDestinationPixels / float(Inconsolata700::outlineCharWidthInPixels);
	const float deltaYFraction = textHeightDestinationPixels / float(Inconsolata700::outlineCharHeightInPixels);

	const float deltaXFromSourceChangeInX = deltaXFraction * cos(-angleInRadians);
	const float deltaXFromSourceChangeInY = deltaYFraction * sin(-angleInRadians);
	const float deltaYFromSourceChangeInX = deltaXFraction * cos(float(-angleInRadians + M_PI / 2));
	const float deltaYFromSourceChangeInY = deltaYFraction * sin(float(-angleInRadians + M_PI / 2));

	const float letterTopLeftX = dieCenter.x +
		letterLeftInOriginPixels * deltaXFromSourceChangeInX +
		textTopInOriginPixels * deltaXFromSourceChangeInY;
	const float letterTopLeftY = dieCenter.y +
		letterLeftInOriginPixels * deltaYFromSourceChangeInX +
		textTopInOriginPixels * deltaYFromSourceChangeInY;
	const float digitTopLeftX = dieCenter.x +
		digitLeftInOriginPixels * deltaXFromSourceChangeInX +
		textTopInOriginPixels * deltaXFromSourceChangeInY;
	const float digitTopLeftY = dieCenter.y +
		digitLeftInOriginPixels * deltaYFromSourceChangeInX +
		textTopInOriginPixels * deltaYFromSourceChangeInY;

	const OcrChar* letterRecord = vreduce<OcrChar, const OcrChar*>( Inconsolata700::letters.characters,
		[letter](const OcrChar* r, const OcrChar* c) -> const OcrChar* {return c->character == letter ? c : r; },
		(const OcrChar*)(NULL)
	);

	const OcrChar* digitRecord = vreduce<OcrChar, const OcrChar*>(Inconsolata700::digits.characters,
		[digit](const OcrChar* r, const OcrChar* c) -> const OcrChar * {return c->character == digit ? c : r; },
		(const OcrChar*)(NULL)
		);

	if (letterRecord) {
		for (auto p : letterRecord->outlinePoints) {
			const int x = int(round(letterTopLeftX + deltaXFromSourceChangeInX * p.x + deltaXFromSourceChangeInY * p.y));
			const int y = int(round(letterTopLeftY + deltaYFromSourceChangeInX * p.x + deltaYFromSourceChangeInY * p.y));
			// BGR => B=0, G=1, R=2, A=3
			if (possibleErrors) {
				imageColor.at<cv::Vec3b>(y, x)[2] = 192;
				imageColor.at<cv::Vec3b>(y, x)[1] = 96;
				imageColor.at<cv::Vec3b>(y, x)[0] = 0;
			}
			else {
				imageColor.at<cv::Vec3b>(y, x)[1] = 192;
				imageColor.at<cv::Vec3b>(y, x)[0] = 0;
				imageColor.at<cv::Vec3b>(y, x)[2] = 0;
			}
		}
	}

	if (digitRecord) {
		for (auto p : digitRecord->outlinePoints) {
			const int x = int(round(digitTopLeftX + deltaXFromSourceChangeInX * p.x + deltaXFromSourceChangeInY * p.y));
			const int y = int(round(digitTopLeftY + deltaYFromSourceChangeInX * p.x + deltaYFromSourceChangeInY * p.y));
			if (possibleErrors) {
				imageColor.at<cv::Vec3b>(y, x)[2] = 192;
				imageColor.at<cv::Vec3b>(y, x)[1] = 96;
				imageColor.at<cv::Vec3b>(y, x)[0] = 0;
			}
			else {
				imageColor.at<cv::Vec3b>(y, x)[1] = 192;
				imageColor.at<cv::Vec3b>(y, x)[0] = 0;
				imageColor.at<cv::Vec3b>(y, x)[2] = 0;
			}
		}
	}
}

static DieCharactersRead readDieCharacters(
	const cv::Mat& imageColor,
	const cv::Mat& grayscaleImage,
	cv::Point2f dieCenter,
	float angleRadians,
	float mmToPixels,
	unsigned char whiteBlackThreshold,
	std::string writeErrorUnlessThisLetterIsRead = "",
	std::string writeErrorUnlessThisDigitIsRead = ""
) {
	// Rotate to remove the angle of the die
	const float degreesToRotateToRemoveAngleOfDie = radiansToDegrees(angleRadians);
	const int textHeightPixels = int(ceil(DieDimensionsMm::textRegionHeight * mmToPixels));
	// FIXME -- constant in next line is a hack
	int textWidthPixels = int(ceil(DieDimensionsMm::textRegionWidth * textWidthAdjustmentMultiplier * mmToPixels));
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
	int charWidth = int((textRegionSize.width - round(DieDimensionsMm::spaceBetweenLetterAndDigit * mmToPixels)) / 2);
	const cv::Rect letterRect(0, 0, charWidth, textRegionSize.height);
	const cv::Rect digitRect(textRegionSize.width - charWidth, 0, charWidth, textRegionSize.height);
	auto letterImage = textEdges(letterRect);
	auto digitImage = textEdges(digitRect);

	// FIXME -- remove after development debugging
	//cv::imwrite("temp/text-region.png", textImage);
	// //	cv::imwrite("temp/text-blurred.png", textBlurred);
	// cv::imwrite("temp/text-edges.png", textEdges);
	// cv::imwrite("temp/letter.png", letterImage);
	// cv::imwrite("temp/digit.png", digitImage);

	const OcrResult lettersMostLikelyFirst = readLetter(letterImage);
	const OcrResult digitsMostLikelyFirst = readDigit(digitImage);

	const char letter0 = lettersMostLikelyFirst[0].character;
	const char digit0 = digitsMostLikelyFirst[0].character;

	// FIXME -- remove after development debugging
	static int error = 1;
	if (writeErrorUnlessThisLetterIsRead.length() != 0 && writeErrorUnlessThisLetterIsRead != "--" && writeErrorUnlessThisLetterIsRead.find_first_of( letter0 ) == -1) {
		cv::imwrite(
			"ocr-errors/error-" + std::to_string(error++) + "-read-" + writeErrorUnlessThisLetterIsRead +
			"-as-" + std::string(1, dashIfNull(letter0)) + ".png",
			letterImage);
	}
	if (writeErrorUnlessThisDigitIsRead.length() != 0 && writeErrorUnlessThisDigitIsRead != "--" && writeErrorUnlessThisDigitIsRead.find_first_of( digit0 ) == -1) {
		cv::imwrite(
			"ocr-errors/error-" + std::to_string(error++) + "-read-" + writeErrorUnlessThisDigitIsRead +
			"-as-" + std::string(1, dashIfNull(digit0)) + ".png",
			digitImage);
	}

	return { lettersMostLikelyFirst, digitsMostLikelyFirst };
}
