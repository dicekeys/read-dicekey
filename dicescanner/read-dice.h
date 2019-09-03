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
#include "simple-ocr.h"
#include "decode-die.h"
#include "find-undoverlines.h"
#include "value-clusters.h"
#include "bit-operations.h"
#include "find-dice.h"
#include "assemble-dice-key.h"
#include "read-die-characters.h"


class ReadDiceResult {
	public:
	bool success;
	std::vector<DieRead> dice;
	float angleInRadiansNonCononicalForm;
	float pixelsPerMm;
	std::vector<DieRead> strayDice;
	std::vector<Undoverline> strayUndoverlines;
};

static ReadDiceResult readDice(const cv::Mat &colorImage, bool outputOcrErrors = false)
{
	cv::Mat grayscaleImage;

	cv::cvtColor(colorImage, grayscaleImage, cv::COLOR_BGR2GRAY);
	DiceAndStrayUndoverlinesFound diceAndStrayUndoverlinesFound = findDiceAndStrayUndoverlines(colorImage, grayscaleImage);
	auto orderedDiceResult = orderDiceAndInferMissingUndoverlines(diceAndStrayUndoverlinesFound);
	if (!orderedDiceResult.valid) {
		return { false, {}, 0, 0, diceAndStrayUndoverlinesFound.diceFound, diceAndStrayUndoverlinesFound.strayUndoverlines };
	}
	std::vector<DieRead> orderedDice = orderedDiceResult.orderedDice;
	const float angleOfDiceKeyInRadiansNonCononicalForm = orderedDiceResult.angleInRadiansNonCononicalForm;

	for (auto &die : orderedDice) {
		if (!(die.underline.determinedIfUnderlineOrOverline || die.overline.determinedIfUnderlineOrOverline)) {
			continue;
			// Without an overline or underline to orient the die, we can't read it.
		}
		// The threshold between black pixels and white pixels is calculated as the average (mean)
		// of the threshold used for the underline and for the overline, but if the underline or overline
		// is absent, we use the threshold from the line that is present.
		const uchar whiteBlackThreshold =
			(die.underline.found && die.overline.found) ?
				uchar((uint(die.underline.whiteBlackThreshold) + uint(die.overline.whiteBlackThreshold)) / 2) :
			die.underline.found ?
				die.underline.whiteBlackThreshold :
				die.overline.whiteBlackThreshold;
		const DieFaceSpecification &underlineInferred = *die.underline.dieFaceInferred;
		const DieFaceSpecification &overlineInferred = *die.overline.dieFaceInferred;
		const DieCharactersRead charsRead = readDieCharacters(colorImage, grayscaleImage, die.center, die.inferredAngleInRadians,
			diceAndStrayUndoverlinesFound.pixelsPerMm, whiteBlackThreshold,
			outputOcrErrors ? ("" + std::string(1, dashIfNull(underlineInferred.letter)) + std::string(1, dashIfNull(overlineInferred.letter))) : "",
			outputOcrErrors ? ("" + std::string(1, dashIfNull(underlineInferred.digit)) + std::string(1, dashIfNull(overlineInferred.digit))) : ""
		);
			
		
		const float orientationInRadians = die.inferredAngleInRadians - angleOfDiceKeyInRadiansNonCononicalForm;
		const float orientationInClockwiseRotationsFloat = orientationInRadians * float(4.0 / (2.0 * M_PI));
		const uchar orientationInClockwiseRotationsFromUpright = uchar(round(orientationInClockwiseRotationsFloat) + 4) % 4;
		die.orientationAs0to3ClockwiseTurnsFromUpright = orientationInClockwiseRotationsFromUpright;
		die.ocrLetter = charsRead.lettersMostLikelyFirst;
		die.ocrDigit = charsRead.digitsMostLikelyFirst;
	}

	return { true, orderedDice, orderedDiceResult.angleInRadiansNonCononicalForm, orderedDiceResult.pixelsPerMm, {} };
}

static DiceKey diceReadToDiceKey(const std::vector<DieRead> diceRead, bool reportErrsToStdErr = false)
{
	if (diceRead.size() != 25) {
		throw std::string("A DiceKey must contain 25 dice but only has " + std::to_string(diceRead.size()));
	}
	std::vector<DieFace> dieFaces;
	for (size_t i = 0; i < diceRead.size(); i++) {
		DieRead dieRead = diceRead[i];
		const DieFaceSpecification &underlineInferred = *dieRead.underline.dieFaceInferred;
		const DieFaceSpecification &overlineInferred = *dieRead.overline.dieFaceInferred;
		const char digitRead = dieRead.ocrDigit.size() == 0 ? '\0' : dieRead.ocrDigit[0].character;
		const char letterRead = dieRead.ocrLetter.size() == 0 ? '\0' :  dieRead.ocrLetter[0].character;
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
			const int bitErrorsIfUnderlineCorrect = hammingDistance(underlineInferred.overlineCode, dieRead.overline.letterDigitEncoding);
			const int bitErrorsIfOverlineCorrect = hammingDistance(overlineInferred.underlineCode, dieRead.underline.letterDigitEncoding);
			const int minBitErrors = std::min(bitErrorsIfUnderlineCorrect, bitErrorsIfOverlineCorrect);
			// See if this error can be explained by a single bit-read error.
			// report error mismatch between undoverline and overline
			if (reportErrsToStdErr) {
				std::cerr << "Mismatch at die " << i << " between underline and overline: " <<
					dashIfNull(underlineInferred.letter) << dashIfNull(underlineInferred.digit) << " != " <<
					dashIfNull(overlineInferred.letter) << dashIfNull(overlineInferred.digit) <<
					" best explained by " << minBitErrors << " bit error in " << 
						(bitErrorsIfUnderlineCorrect < bitErrorsIfOverlineCorrect ? "overline" : "underline") <<
					" (ocr returned " << dashIfNull(letterRead) << dashIfNull(digitRead) << ")" <<
					"\n";
			}
		}
		if (letterRead == 0 && (dieRead.underline.found || dieRead.overline.found)) {
			if (reportErrsToStdErr) {
				std::cerr << "Letter at die " << i << " could not be read " <<
				"(underline=>'" << dashIfNull(underlineInferred.letter) <<
				"', overline=>'" << dashIfNull(overlineInferred.letter) << "')\n";
			}
		}
		if (digitRead == 0 && (dieRead.underline.found || dieRead.overline.found)) {
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

		dieFaces.push_back(DieFace(
			majorityOfThree(
				underlineInferred.letter, overlineInferred.letter, letterRead
			),
			majorityOfThree(
				underlineInferred.digit, overlineInferred.digit, digitRead
			),
			dieRead.orientationAs0to3ClockwiseTurnsFromUpright,
			dieRead.error()
			));
	}
	return DiceKey(dieFaces);
}


DiceKey mergeDiceResult(
	const DiceKey &previousDiceResult,
	const DiceKey &currentDiceResult	
) {
	// rotate;

	return currentDiceResult;
}
