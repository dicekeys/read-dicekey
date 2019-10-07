//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#include <float.h>
#include <chrono>
#include <iostream>
#include <math.h>

#include "utilities/bit-operations.h"
#include "graphics/cv.h"
#include "keysqr.h"
#include "read-faces.h"
#include "read-keysqr.h"
#include "visualize-read-results.h"



KeySqr facesReadToKeySqr(
	const std::vector<FaceRead> facesRead,
	bool reportErrsToStdErr
) {
	if (facesRead.size() != 25) {
		throw std::string("A KeySqr must contain 25 faces but only has " + std::to_string(facesRead.size()));
	}
	std::vector<ElementFace> faces;
	for (size_t i = 0; i < facesRead.size(); i++) {
		FaceRead faceRead = facesRead[i];
		const ElementFaceSpecification &underlineInferred = *faceRead.underline.faceInferred;
		const ElementFaceSpecification &overlineInferred = *faceRead.overline.faceInferred;
		const char digitRead = faceRead.ocrDigit.size() == 0 ? '\0' : faceRead.ocrDigit[0].character;
		const char letterRead = faceRead.ocrLetter.size() == 0 ? '\0' :  faceRead.ocrLetter[0].character;
		if (!faceRead.underline.found) {
			if (reportErrsToStdErr) {
				std::cerr << "Underline for face " << i << " not found\n";
			}
		}
		if (!faceRead.overline.found) {
			if (reportErrsToStdErr) {
				std::cerr << "Overline for face " << i << " not found\n";
			}
		}
		if ((faceRead.underline.found && faceRead.overline.found) &&
			(
				underlineInferred.letter != overlineInferred.letter ||
				underlineInferred.digit != overlineInferred.digit) 
			) {
			const int bitErrorsIfUnderlineCorrect = hammingDistance(underlineInferred.overlineCode, faceRead.overline.letterDigitEncoding);
			const int bitErrorsIfOverlineCorrect = hammingDistance(overlineInferred.underlineCode, faceRead.underline.letterDigitEncoding);
			const int minBitErrors = std::min(bitErrorsIfUnderlineCorrect, bitErrorsIfOverlineCorrect);
			// report error mismatch between undoverline and overline
			if (reportErrsToStdErr) {
				std::cerr << "Mismatch at face " << i << " between underline and overline: " <<
					dashIfNull(underlineInferred.letter) << dashIfNull(underlineInferred.digit) << " != " <<
					dashIfNull(overlineInferred.letter) << dashIfNull(overlineInferred.digit) <<
					" best explained by "  << minBitErrors <<
					" bit error in " << 
						(bitErrorsIfUnderlineCorrect < bitErrorsIfOverlineCorrect ? "overline" : "underline") <<
					" (ocr returned " << dashIfNull(letterRead) << dashIfNull(digitRead) << ")" <<
					"\n";
			}
		}
		if (letterRead == 0 && (faceRead.underline.found || faceRead.overline.found)) {
			if (reportErrsToStdErr) {
				std::cerr << "Letter at face " << i << " could not be read " <<
				"(underline=>'" << dashIfNull(underlineInferred.letter) <<
				"', overline=>'" << dashIfNull(overlineInferred.letter) << "')\n";
			}
		}
		if (digitRead == 0 && (faceRead.underline.found || faceRead.overline.found)) {
			if (reportErrsToStdErr) {
				std::cerr << "Digit at face " << i << " could not be read " <<
				"(underline=>'" << dashIfNull(underlineInferred.digit) <<
				"', overline=>'" << dashIfNull(overlineInferred.digit) << "')\n";
			}
		}
		if (letterRead && (faceRead.underline.found || faceRead.overline.found) &&
			letterRead != underlineInferred.letter && letterRead != overlineInferred.letter) {
			// report OCR error on letter
			if (reportErrsToStdErr) {
				std::cerr << "Mismatch at face " << i << " between ocr letter, '" << letterRead <<
				"', the underline ('" << dashIfNull(underlineInferred.letter) <<
				"'), and overline ('" << dashIfNull(overlineInferred.letter) << "')\n";
			}
		}
		if (digitRead && (faceRead.underline.found || faceRead.overline.found) &&
			digitRead != underlineInferred.digit && digitRead != overlineInferred.digit) {
			// report OCR error on digit
			if (reportErrsToStdErr) {
				std::cerr << "Mismatch at face " << i << " between ocr digit, '" << digitRead <<
				"', the underline ('" << dashIfNull(underlineInferred.digit) <<
				"'), and overline ('" << dashIfNull(overlineInferred.digit) << ")'\n";
			}
		}

		faces.push_back(ElementFace(
			majorityOfThree(
				underlineInferred.letter, overlineInferred.letter, letterRead
			),
			majorityOfThree(
				underlineInferred.digit, overlineInferred.digit, digitRead
			),
			faceRead.orientationAs0to3ClockwiseTurnsFromUpright,
			faceRead.error()
			));
	}
	return KeySqr(faces);
}


KeySqr readKeySqr(
	const cv::Mat &grayscaleImage,
	bool outputErrors
) {
  const ReadFaceResult facesRead = readFaces(grayscaleImage, outputErrors);
  if (!facesRead.success) {
    return KeySqr();
  }
  try {
    return facesReadToKeySqr(facesRead.faces, outputErrors);
  } catch (...) {
    return KeySqr();
  }
}

std::string readKeySqrJson(
	const cv::Mat &grayscaleImage
) {
  KeySqr keySqr = readKeySqr(grayscaleImage, false);
  return keySqr.toJson();
}


std::string readKeySqrJson (
	int width,
	int height,
	size_t bytesPerRow,
	void* data
) {
  const cv::Mat grayscaleImage(cv::Size(width, height), CV_8UC1, data, bytesPerRow);
  KeySqr keySqr = readKeySqr(grayscaleImage, false);
  return keySqr.toJson();
}

const unsigned int maxCorrectableError = 2;
const int millisecondsToTryToRemoveCorrectableErrors = 4000;

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
 * method of the keySqr field (result.keySqr.toJson()) to get a
 * std::string in JSON format that can be returned back to any
 * consumer that can parse JSON format.
 **/
bool scanAndAugmentKeySqrImage(
	cv::Mat &sourceColorImageBGR_CV_8UC3,
	ResultOfScanAndAugmentKeySqrImage* result
) {
	const ReadFaceResult facesRead = readFaces(sourceColorImageBGR_CV_8UC3, false);

	const KeySqr latestKeySqr = facesRead.success ? facesReadToKeySqr(facesRead.faces, false) : KeySqr();
	
	const KeySqr mergedKeySqr = (!result->initialized) ? latestKeySqr :
		latestKeySqr.initialized ?
			latestKeySqr.mergePrevious(result->keySqr) :
			latestKeySqr;

	result->whenLastRead = std::chrono::system_clock::now();
	result->whenFirstRead = (result->initialized) ? result->whenFirstRead : result->whenLastRead;
	result->whenLastImproved =
		(!result->initialized || result->keySqr.totalError() > mergedKeySqr.totalError()) ?
			result->whenLastRead : result->whenLastImproved;

	// We're done when we either have an error-free scan, or no face that can't be improved after
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

	result->keySqr = mergedKeySqr;
	result->augmentedColorImage_BGR_CV_8UC3 = visualizeReadResults(sourceColorImageBGR_CV_8UC3, facesRead, false);
	return terminate;
}