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
#include "dice.h"
//#include "ocr.h"
#include "simple-ocr.h"
#include "decode-die.h"
#include "find-undoverlines.h"
#include "value-clusters.h"
#include "bit-operations.h"
#include "find-dice.h"
#include "assemble-dice-key.h"

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
	int textWidthPixels = int(ceil(DieDimensionsMm::textRegionWidth * 0.8f * mmToPixels));
	// Use an even text region width so we can even split it in two at the center;
	if ((textWidthPixels % 2) == 1) {
		textWidthPixels += 1;
	}
	cv::Size textRegionSize = cv::Size(textWidthPixels, textHeightPixels);

	const auto textImage = copyRotatedRectangle(grayscaleImage, dieCenter, degreesToRotateToRemoveAngleOfDie, textRegionSize);
	cv::Mat textBlurred, textEdges;
	cv::medianBlur(textImage, textBlurred, 5);
	cv::threshold(textBlurred, textEdges, whiteBlackThreshold, 255, cv::THRESH_BINARY + cv::THRESH_OTSU);

	// Setup a rectangle to define your region of interest
	const cv::Rect letterRect(0, 0, textRegionSize.width / 2, textRegionSize.height);
	const cv::Rect digitRect( textRegionSize.width / 2, 0, textRegionSize.width / 2, textRegionSize.height);
	auto letterImage = textEdges(letterRect);
	auto digitImage = textEdges(digitRect);

	// FIXME -- remove after development debugging
	cv::imwrite("text-region.png", textImage);
	cv::imwrite("letter.png", letterImage);
	cv::imwrite("digit.png", digitImage);

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


static std::vector<DieRead> readDice(const cv::Mat &colorImage, bool outputOcrErrors = false)
{
	cv::Mat grayscaleImage;

	cv::cvtColor(colorImage, grayscaleImage, cv::COLOR_BGR2GRAY);
	DiceAndStrayUndoverlinesFound diceAndStrayUndoverlinesFound = findDiceAndStrayUndoverlines(colorImage, grayscaleImage);
	auto orderedDiceResult = orderDiceAndInferMissingUndoverlines(diceAndStrayUndoverlinesFound);
	if (!orderedDiceResult.valid) {
		return {};
	}
	std::vector<DieRead> orderedDice = orderedDiceResult.orderedDice;
	const float angleOfDiceKeyInRadiansNonCononicalForm = orderedDiceResult.angleInRadiansNonCononicalForm;

	for (auto &die : orderedDice) {
		// Average the angle of the underline and overline
		const auto charsRead = readDieCharacters(colorImage, grayscaleImage, die.center, die.inferredAngleInRadians,
			diceAndStrayUndoverlinesFound.pixelsPerMm,
			// The threshold between black pixels and white pixels is calculated as the average (mean)
			// of the threshold used for the underline and for the overline.
			uchar( (uint(die.underline.whiteBlackThreshold) + uint(die.overline.whiteBlackThreshold))/2 ),
			outputOcrErrors ? die.underline.dieFaceInferred.letter : '\0',
			outputOcrErrors ? die.underline.dieFaceInferred.digit : '\0'
		);
		const float orientationInRadians = die.inferredAngleInRadians - angleOfDiceKeyInRadiansNonCononicalForm;
		const float orientationInClockwiseRotationsFloat = orientationInRadians * float(4.0 / (2.0 * M_PI));
		const uchar orientationInClockwiseRotationsFromUpright = uchar(round(orientationInClockwiseRotationsFloat) + 4) % 4;
		die.orientationAs0to3ClockwiseTurnsFromUpright = orientationInClockwiseRotationsFromUpright;
		int letterIndex = 
		die.ocrLetter = charsRead.letter;
		die.ocrDigit = charsRead.digit;
	}
	//// calculate the average angle mod 90 so we can generate a rotation function
	//for (size_t i = 0; i < diceFound.size(); i++) {
	//	diceFound[i].angleAdjustedCenter = rotatePointClockwiseAroundOrigin(diceFound[i].center, angleOfDiceKeyInRadiansNonCononicalForm);
	//}

	//// Sort the dice based on their positions after adjusting the angle
	//std::sort(diceFound.begin(), diceFound.end(), [halfDieSize](DieRead a, DieRead b) {
	//	if (a.angleAdjustedCenter.y < (b.angleAdjustedCenter.y - halfDieSize)) {
	//		// Die a is at least a half die above die b, and therefore comes before it
	//		return true;
	//	}
	//	else if (b.angleAdjustedCenter.y < (a.angleAdjustedCenter.y - halfDieSize)) {
	//		// Die b is at least a half die above die a, and therefore comes before it
	//		return false;
	//	}
	//	// Die a and die b are roughly the same from top to bottom, so order left to right
	//	return a.angleAdjustedCenter.x < b.angleAdjustedCenter.x;
	//});

	return orderedDice;
}

static std::vector<DieFace> diceReadToDiceKey(const std::vector<DieRead> diceRead, bool reportErrsToStdErr = false)
{
	if (diceRead.size() != 25) {
		throw std::string("A DiceKey must contain 25 dice but only has " + std::to_string(diceRead.size()));
	}
	std::vector<DieFace> diceKey;
	for (size_t i = 0; i < diceRead.size(); i++) {
		DieRead dieRead = diceRead[i];
		const DieFaceSpecification& underlineInferred = dieRead.underline.dieFaceInferred;
		const DieFaceSpecification& overlineInferred = dieRead.overline.dieFaceInferred;
		const char digitRead = dieRead.ocrDigit;
		const char letterRead = dieRead.ocrLetter;
		if (!dieRead.underline.found) {
			if (reportErrsToStdErr) {
				std::cerr << "Underline for die " << i << " not found\n";
			}
		}
		if (!dieRead.overline.found) {
			if (reportErrsToStdErr) {
				std::cerr << "Overline for die " << i << " not found\n";
			}
		}
		if ((dieRead.underline.found && dieRead.overline.found) &&
			(
				underlineInferred.letter != overlineInferred.letter ||
				underlineInferred.digit != overlineInferred.digit) 
			) {
			const int bitErrorsIfUnderlineCorrect = hammingDistance(dieRead.underline.dieFaceInferred.overlineCode, dieRead.overline.letterDigitEncoding);
			const int bitErrorsIfOverlineCorrect = hammingDistance(dieRead.overline.dieFaceInferred.underlineCode, dieRead.underline.letterDigitEncoding);
			const int minBitErrors = std::min(bitErrorsIfUnderlineCorrect, bitErrorsIfOverlineCorrect);
			// See if this error can be explained by a single bit-read error.
			// report error mismatch between undoverline and overline
			if (reportErrsToStdErr) {
				std::cerr << "Mismatch at die " << i << " between underline and overline: " <<
					dashIfNull(underlineInferred.letter) << dashIfNull(underlineInferred.digit) << " != " <<
					dashIfNull(overlineInferred.letter) << dashIfNull(overlineInferred.digit) <<
					" best explained by " << minBitErrors << " bit error in " << 
						(bitErrorsIfUnderlineCorrect < bitErrorsIfOverlineCorrect ? "overline" : "underline") <<
					" (ocr returned " << dashIfNull(dieRead.ocrLetter) << dashIfNull(dieRead.ocrDigit) << ")" <<
					"\n";
			}
		}
		if (letterRead == 0) {
			if (reportErrsToStdErr) {
				std::cerr << "Letter at die " << i << " could not be read " <<
				"(underline=>'" << dashIfNull(underlineInferred.letter) <<
				"', overline=>'" << dashIfNull(overlineInferred.letter) << "')\n";
			}
		}
		if (digitRead == 0) {
			if (reportErrsToStdErr) {
				std::cerr << "Digit at die " << i << " could not be read " <<
				"(underline=>'" << dashIfNull(underlineInferred.digit) <<
				"', overline=>'" << dashIfNull(overlineInferred.digit) << "')\n";
			}
		}
		if (letterRead && (dieRead.underline.found || dieRead.overline.found) &&
			letterRead != underlineInferred.letter && letterRead != overlineInferred.letter) {
			// report OCR error on letter
			if (reportErrsToStdErr) {
				std::cerr << "Mismatch at die " << i << " between ocr letter, '" << letterRead <<
				"', the underline ('" << dashIfNull(underlineInferred.letter) <<
				"'), and overline ('" << dashIfNull(overlineInferred.letter) << "')\n";
			}
		}
		if (digitRead && (dieRead.underline.found || dieRead.overline.found) &&
			digitRead != underlineInferred.digit && digitRead != overlineInferred.digit) {
			// report OCR error on digit
			if (reportErrsToStdErr) {
				std::cerr << "Mismatch at die " << i << " between ocr digit, '" << digitRead <<
				"', the underline ('" << dashIfNull(underlineInferred.digit) <<
				"'), and overline ('" << dashIfNull(overlineInferred.digit) << ")'\n";
			}
		}

		diceKey.push_back(DieFace({
			majorityOfThree(
				underlineInferred.letter, overlineInferred.letter, dieRead.ocrLetter
			),
			majorityOfThree(
				underlineInferred.digit, overlineInferred.digit, dieRead.ocrDigit
			),
			dieRead.orientationAs0to3ClockwiseTurnsFromUpright
			}));
	}
	return diceKey;
}
