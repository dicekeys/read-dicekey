//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#include <float.h>
#include <ctime>
#include <chrono>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include <iostream>
#include <math.h>

#include "keysqr.h"

#include "utilities/vfunctional.h"
#include "utilities/statistics.h"
#include "utilities/bit-operations.h"
#include "graphics/geometry.h"
#include "simple-ocr.h"
#include "decode-die.h"
#include "find-undoverlines.h"
#include "find-dice.h"
#include "assemble-dicekey.h"
#include "read-die-characters.h"
#include "visualize-read-results.h"

char ElementRead::ocrLetterMostLikely() const {
	return this->ocrLetter.size() == 0 ? '\0' : ocrLetter[0].character;
}
char ElementRead::ocrDigitMostLikely() const {
	return ocrDigit.size() == 0 ? '\0' : ocrDigit[0].character;
}

char ElementRead::letter() const {
	return majorityOfThree(
		underline.dieFaceInferred->letter, overline.dieFaceInferred->letter, ocrLetterMostLikely()
	);
}
char ElementRead::digit() const {
	return majorityOfThree(
		underline.dieFaceInferred->digit, overline.dieFaceInferred->digit, ocrDigitMostLikely()
	);
}

std::string ElementRead::toJson() const {
	std::ostringstream jsonStream;
	jsonStream <<
		"{" <<
			"underline: " << underline.toJson() << "," <<
			"overline: " << overline.toJson() << "," <<
			"center: {" <<
				"x: " << center.x << ", " <<
				"y: " << center.y << "" <<
			"}, " <<
			"angleInRadians: " << inferredAngleInRadians << "," <<
			"orientationAs0to3ClockwiseTurnsFromUpright: " << orientationAs0to3ClockwiseTurnsFromUpright << "," <<
			"ocrLetter: " << (ocrLetter.size > 0 ? ocrLetter[0].character : '-') << "," <<
			"ocrDigit: " << (ocrDigit.size > 0 ? ocrDigit[0].character : '-') << "" <<
		"}";
	return jsonStream.str();
}

// Return an estimate of the error in reading an element face.
// If the underline, overline, and OCR results match, the error is 0.
// If the only error is a 1-3 bit error in either the underline or overline,
// the result is the number of bits (hamming distance) in the underline or overline
// that doesn't match the OCR result.
// If the underline and overline match but matched with the OCR's second choice of
// letter or digit, we return 2.
ElementFaceError ElementRead::error() const {
		if (ocrLetter.size() == 0 || ocrDigit.size() == 0) {
			return ElementFaceErrors::WorstPossible;
		}
		unsigned char errorLocation = 0;
		unsigned int errorMagnitude = 0;
		const char ocrLetter0 = ocrLetter[0].character;
		const char ocrDigit0 = ocrDigit[0].character;
		const ElementFaceSpecification* pUnderlineFaceInferred = underline.dieFaceInferred;
		const ElementFaceSpecification* pOverlineFaceInferred = overline.dieFaceInferred;

		// Test hypothesis of no error
		if (pUnderlineFaceInferred == pOverlineFaceInferred) {
			// The underline and overline map to the same die face
			const ElementFaceSpecification& undoverlineFaceInferred = *(underline.dieFaceInferred);

			// Check for OCR errors for the letter read
			if (undoverlineFaceInferred.letter != ocrLetter[0].character) {
				errorLocation |= ElementFaceErrors::Location::OcrLetter;
				errorMagnitude += undoverlineFaceInferred.letter == ocrLetter[1].character ?
					ElementFaceErrors::Magnitude::OcrCharacterWasSecondChoice :
					ElementFaceErrors::Magnitude::OcrCharacterInvalid;
			}
			if (undoverlineFaceInferred.digit != ocrDigit[0].character) {
				errorLocation |= ElementFaceErrors::Location::OcrDigit;
				errorMagnitude += undoverlineFaceInferred.digit == ocrDigit[1].character ?
					ElementFaceErrors::Magnitude::OcrCharacterWasSecondChoice :
					ElementFaceErrors::Magnitude::OcrCharacterInvalid;
			}
			return {(unsigned char) MIN(std::numeric_limits<unsigned char>::max(), errorMagnitude), errorLocation};
		}
		const ElementFaceSpecification& underlineFaceInferred = *pUnderlineFaceInferred;
		const ElementFaceSpecification& overlineFaceInferred = *pOverlineFaceInferred;
		if (underlineFaceInferred.letter == ocrLetter0 && underlineFaceInferred.digit == ocrDigit0) {
			// The underline matches the OCR result, so the error is in the overline
			return {
					overline.found ?
						// The magnitude of the error is the hamming distance error in overline
						(unsigned char)hammingDistance(underlineFaceInferred.overlineCode, overline.letterDigitEncoding) :
						// Since the overline was not found, the magntidue is specified via a constant
						ElementFaceErrors::Magnitude::UnderlineOrOverlineMissing,
					ElementFaceErrors::Location::Overline
				};
		}
		if (overlineFaceInferred.letter == ocrLetter0 && overlineFaceInferred.digit == ocrDigit0) {
			// Since overline matches the OCR result, so the error is in the underline
			return {
				underline.found ?
					// The magnitude of the error is the hamming distance error in underline
					(unsigned char)hammingDistance(overlineFaceInferred.underlineCode, underline.letterDigitEncoding) :				
					// Since the underline was not found, the magntidue is specified via a constant
					ElementFaceErrors::Magnitude::UnderlineOrOverlineMissing,
					ElementFaceErrors::Location::Underline
			};
		}
		// No good matching.  Return max error
		return {std::numeric_limits<unsigned char>::max(), std::numeric_limits<unsigned char>::max()};
	};


ReadDiceResult readDice(
	const cv::Mat &colorImage,
	bool outputOcrErrors
) {
	cv::Mat grayscaleImage;

	cv::cvtColor(colorImage, grayscaleImage, cv::COLOR_BGR2GRAY);
	DiceAndStrayUndoverlinesFound diceAndStrayUndoverlinesFound = findDiceAndStrayUndoverlines(colorImage, grayscaleImage);
	auto orderedDiceResult = orderDiceAndInferMissingUndoverlines(colorImage, grayscaleImage, diceAndStrayUndoverlinesFound);
	if (!orderedDiceResult.valid) {
		return { false, {}, 0, 0, diceAndStrayUndoverlinesFound.diceFound, diceAndStrayUndoverlinesFound.strayUndoverlines };
	}
	std::vector<ElementRead> orderedDice = orderedDiceResult.orderedDice;
	const float angleOfKeySqrInRadiansNonCononicalForm = orderedDiceResult.angleInRadiansNonCononicalForm;

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
		const ElementFaceSpecification &underlineInferred = *die.underline.dieFaceInferred;
		const ElementFaceSpecification &overlineInferred = *die.overline.dieFaceInferred;
		const DieCharactersRead charsRead = readDieCharacters(colorImage, grayscaleImage, die.center, die.inferredAngleInRadians,
			diceAndStrayUndoverlinesFound.pixelsPerFaceEdgeWidth, whiteBlackThreshold,
			outputOcrErrors ? ("" + std::string(1, dashIfNull(underlineInferred.letter)) + std::string(1, dashIfNull(overlineInferred.letter))) : "",
			outputOcrErrors ? ("" + std::string(1, dashIfNull(underlineInferred.digit)) + std::string(1, dashIfNull(overlineInferred.digit))) : ""
		);
			
		
		const float orientationInRadians = die.inferredAngleInRadians - angleOfKeySqrInRadiansNonCononicalForm;
		const float orientationInClockwiseRotationsFloat = orientationInRadians * float(4.0 / (2.0 * M_PI));
		const uchar orientationInClockwiseRotationsFromUpright = uchar(round(orientationInClockwiseRotationsFloat) + 4) % 4;
		die.orientationAs0to3ClockwiseTurnsFromUpright = orientationInClockwiseRotationsFromUpright;
		die.ocrLetter = charsRead.lettersMostLikelyFirst;
		die.ocrDigit = charsRead.digitsMostLikelyFirst;
	}

	return { true, orderedDice, orderedDiceResult.angleInRadiansNonCononicalForm, orderedDiceResult.pixelsPerFaceEdgeWidth, {} };
}

KeySqr diceReadToKeySqr(
	const std::vector<ElementRead> diceRead,
	bool reportErrsToStdErr
) {
	if (diceRead.size() != 25) {
		throw std::string("A KeySqr must contain 25 dice but only has " + std::to_string(diceRead.size()));
	}
	std::vector<ElementFace> dieFaces;
	for (size_t i = 0; i < diceRead.size(); i++) {
		ElementRead dieRead = diceRead[i];
		const ElementFaceSpecification &underlineInferred = *dieRead.underline.dieFaceInferred;
		const ElementFaceSpecification &overlineInferred = *dieRead.overline.dieFaceInferred;
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
			// report error mismatch between undoverline and overline
			if (reportErrsToStdErr) {
				std::cerr << "Mismatch at die " << i << " between underline and overline: " <<
					dashIfNull(underlineInferred.letter) << dashIfNull(underlineInferred.digit) << " != " <<
					dashIfNull(overlineInferred.letter) << dashIfNull(overlineInferred.digit) <<
					" best explained by "  << minBitErrors <<
					" bit error in " << 
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

		dieFaces.push_back(ElementFace(
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
	return KeySqr(dieFaces);
}



const unsigned int maxCorrectableError = 2;
const int millisecondsToTryToRemoveCorrectableErrors = 4000;


const std::chrono::time_point<std::chrono::system_clock> minTimePoint =
	std::chrono::time_point<std::chrono::system_clock>::min();

/**
 * This structure is used as the second parameter to scanAndAugmentKeySqrImage,
 * and is used both to input the result of the prior call and to return results
 * from the current call.
 **/
struct ResultOfScanAndAugmentKeySqrImage {
	// This value is true if the result was returned from a call to readKeySqr and
	// is false when a default result is constructed by the caller and a pointer is
	// passed to it.
	bool initialized = false;
	// This value is set the first time scanAndAugmentKeySqrImage is called
	std::chrono::time_point<std::chrono::system_clock> whenFirstRead = minTimePoint;
	// The value is set the first time scanAndAugmentKeySqrImage is called
	// and updated every time a scan reduces the number of errors that
	// have the be resolved before we can return the result.
	std::chrono::time_point<std::chrono::system_clock> whenLastImproved = minTimePoint;
	// The value is set every time scanAndAugmentKeySqrImage is called.
	std::chrono::time_point<std::chrono::system_clock> whenLastRead = minTimePoint;
	// The KeySqr that has been read is stored in this field, which also
	// keeps track of any errors that you have to be resolved during reading.
	KeySqr diceKey = KeySqr();
	// After each call, the image passed to scanAndAugmentKeySqrImage
	// is augmented with the scan results and copied here. 
	cv::Mat augmentedColorImage_BGR_CV_8UC3 = cv::Mat();
	// This field is set to true if we've reached the termination condition
	// for the scanning loop.  This is the same value returned as the
	// result of the scanAndAugmentKeySqrImage function.
	bool terminate = false;
};

/**
 * This function is the base for an augmented reality loop in which
 * we scan images from the camera repeatedly until we have successfully
 * scanned a KeySqr.
 * 
 * The first parameter should be the last frame from the camera,
 * as an OpenCV Mat (image) in 8-bit unsigned BGR (blue, green, red)
 * format.
 * 
 * Your event loop should allocate a ResultOfScanAndAugmentKeySqrImage
 * struct and pass it to every call.  This function will consume your
 * previous result to compare it with the new scan, then write the
 * new result over the old one.
 * 
 * The function will return when there is a KeySqr to return to
 * the caller and false if scanning should continue.
 * 
 * If generating an API for callers that can consume the
 * ResultOfScanAndAugmentKeySqrImage struct directly, simply
 * return that struct on terminating. 
 * For APIs that cannot consume the struct directly, call the toJson()
 * method of the diceKey field (result.diceKey.toJson()) to get a
 * std::string in JSON format that can be returned back to any
 * consumer that can parse JSON format.
 **/
bool scanAndAugmentKeySqrImage(
	cv::Mat &sourceColorImageBGR_CV_8UC3,
	ResultOfScanAndAugmentKeySqrImage* result
) {
	const ReadDiceResult diceRead = readDice(sourceColorImageBGR_CV_8UC3, false);

	const KeySqr latestKeySqr = diceRead.success ? diceReadToKeySqr(diceRead.dice, false) : KeySqr();
	
	const KeySqr mergedKeySqr = (!result->initialized) ? latestKeySqr :
		latestKeySqr.initialized ?
			latestKeySqr.mergePrevious(result->diceKey) :
			latestKeySqr;

	result->whenLastRead = std::chrono::system_clock::now();
	result->whenFirstRead = (result->initialized) ? result->whenFirstRead : result->whenLastRead;
	result->whenLastImproved =
		(!result->initialized || result->diceKey.totalError() > mergedKeySqr.totalError()) ?
			result->whenLastRead : result->whenLastImproved;

	// We're done when we either have an error-free scan, or no die that can't be improved after
	const bool terminate = (
			// We have an error free scan, or...
			mergedKeySqr.totalError() == 0
		) || (
			// We have only correctable errors, and we've our budget of time hoping more
			// scanning will remove those errors.
			mergedKeySqr.maxError() <= maxCorrectableError &&
			std::chrono::duration_cast<std::chrono::milliseconds>(result->whenLastRead - result->whenLastImproved).count() >
				millisecondsToTryToRemoveCorrectableErrors
		);
	// auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(foo - now).count;

	result->diceKey = mergedKeySqr;
	result->augmentedColorImage_BGR_CV_8UC3 = visualizeReadResults(sourceColorImageBGR_CV_8UC3, diceRead, false);
	return terminate;
}