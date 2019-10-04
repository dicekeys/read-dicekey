//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#include <float.h>
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
#include "decode-face.h"
#include "find-undoverlines.h"
#include "find-faces.h"
#include "assemble-keysqr.h"
#include "read-face-characters.h"
#include "visualize-read-results.h"

char FaceRead::ocrLetterMostLikely() const {
	return this->ocrLetter.size() == 0 ? '\0' : ocrLetter[0].character;
}
char FaceRead::ocrDigitMostLikely() const {
	return ocrDigit.size() == 0 ? '\0' : ocrDigit[0].character;
}

char FaceRead::letter() const {
	return majorityOfThree(
		underline.faceInferred->letter, overline.faceInferred->letter, ocrLetterMostLikely()
	);
}
char FaceRead::digit() const {
	return majorityOfThree(
		underline.faceInferred->digit, overline.faceInferred->digit, ocrDigitMostLikely()
	);
}

std::string FaceRead::toJson() const {
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
			"ocrLetter: " << (ocrLetter.size() > 0 ? ocrLetter[0].character : '-') << "," <<
			"ocrDigit: " << (ocrDigit.size() > 0 ? ocrDigit[0].character : '-') << "" <<
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
FaceError FaceRead::error() const {
		if (ocrLetter.size() == 0 || ocrDigit.size() == 0) {
			return FaceErrors::WorstPossible;
		}
		unsigned char errorLocation = 0;
		unsigned int errorMagnitude = 0;
		const char ocrLetter0 = ocrLetter[0].character;
		const char ocrDigit0 = ocrDigit[0].character;
		const ElementFaceSpecification* pUnderlineFaceInferred = underline.faceInferred;
		const ElementFaceSpecification* pOverlineFaceInferred = overline.faceInferred;

		// Test hypothesis of no error
		if (pUnderlineFaceInferred == pOverlineFaceInferred) {
			// The underline and overline map to the same face face
			const ElementFaceSpecification& undoverlineFaceInferred = *(underline.faceInferred);

			// Check for OCR errors for the letter read
			if (undoverlineFaceInferred.letter != ocrLetter[0].character) {
				errorLocation |= FaceErrors::Location::OcrLetter;
				errorMagnitude += undoverlineFaceInferred.letter == ocrLetter[1].character ?
					FaceErrors::Magnitude::OcrCharacterWasSecondChoice :
					FaceErrors::Magnitude::OcrCharacterInvalid;
			}
			if (undoverlineFaceInferred.digit != ocrDigit[0].character) {
				errorLocation |= FaceErrors::Location::OcrDigit;
				errorMagnitude += undoverlineFaceInferred.digit == ocrDigit[1].character ?
					FaceErrors::Magnitude::OcrCharacterWasSecondChoice :
					FaceErrors::Magnitude::OcrCharacterInvalid;
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
						FaceErrors::Magnitude::UnderlineOrOverlineMissing,
					FaceErrors::Location::Overline
				};
		}
		if (overlineFaceInferred.letter == ocrLetter0 && overlineFaceInferred.digit == ocrDigit0) {
			// Since overline matches the OCR result, so the error is in the underline
			return {
				underline.found ?
					// The magnitude of the error is the hamming distance error in underline
					(unsigned char)hammingDistance(overlineFaceInferred.underlineCode, underline.letterDigitEncoding) :				
					// Since the underline was not found, the magntidue is specified via a constant
					FaceErrors::Magnitude::UnderlineOrOverlineMissing,
					FaceErrors::Location::Underline
			};
		}
		// No good matching.  Return max error
		return {std::numeric_limits<unsigned char>::max(), std::numeric_limits<unsigned char>::max()};
	};


ReadFaceResult readFaces(
	const cv::Mat &grayscaleImage,
	bool outputOcrErrors
) {
	FacesAndStrayUndoverlinesFound facesAndStrayUndoverlinesFound = findFacesAndStrayUndoverlines(grayscaleImage);
	auto orderedFacesResult = orderFacesAndInferMissingUndoverlines(grayscaleImage, facesAndStrayUndoverlinesFound);
	if (!orderedFacesResult.valid) {
		return { false, {}, 0, 0, facesAndStrayUndoverlinesFound.facesFound, facesAndStrayUndoverlinesFound.strayUndoverlines };
	}
	std::vector<FaceRead> ordeeredFaces = orderedFacesResult.orderedFaces;
	const float angleOfKeySqrInRadiansNonCononicalForm = orderedFacesResult.angleInRadiansNonCononicalForm;

	for (auto &face : ordeeredFaces) {
		if (!(face.underline.determinedIfUnderlineOrOverline || face.overline.determinedIfUnderlineOrOverline)) {
			continue;
			// Without an overline or underline to orient the face, we can't read it.
		}
		// The threshold between black pixels and white pixels is calculated as the average (mean)
		// of the threshold used for the underline and for the overline, but if the underline or overline
		// is absent, we use the threshold from the line that is present.
		const uchar whiteBlackThreshold =
			(face.underline.found && face.overline.found) ?
				uchar((uint(face.underline.whiteBlackThreshold) + uint(face.overline.whiteBlackThreshold)) / 2) :
			face.underline.found ?
				face.underline.whiteBlackThreshold :
				face.overline.whiteBlackThreshold;
		const ElementFaceSpecification &underlineInferred = *face.underline.faceInferred;
		const ElementFaceSpecification &overlineInferred = *face.overline.faceInferred;
		const CharactersReadFromFaces charsRead = readCharactersOnFace(grayscaleImage, face.center, face.inferredAngleInRadians,
			facesAndStrayUndoverlinesFound.pixelsPerFaceEdgeWidth, whiteBlackThreshold,
			outputOcrErrors ? ("" + std::string(1, dashIfNull(underlineInferred.letter)) + std::string(1, dashIfNull(overlineInferred.letter))) : "",
			outputOcrErrors ? ("" + std::string(1, dashIfNull(underlineInferred.digit)) + std::string(1, dashIfNull(overlineInferred.digit))) : ""
		);
			
		
		const float orientationInRadians = face.inferredAngleInRadians - angleOfKeySqrInRadiansNonCononicalForm;
		const float orientationInClockwiseRotationsFloat = orientationInRadians * float(4.0 / (2.0 * M_PI));
		const uchar orientationInClockwiseRotationsFromUpright = uchar(round(orientationInClockwiseRotationsFloat) + 4) % 4;
		face.orientationAs0to3ClockwiseTurnsFromUpright = orientationInClockwiseRotationsFromUpright;
		face.ocrLetter = charsRead.lettersMostLikelyFirst;
		face.ocrDigit = charsRead.digitsMostLikelyFirst;
	}

	return { true, ordeeredFaces, orderedFacesResult.angleInRadiansNonCononicalForm, orderedFacesResult.pixelsPerFaceEdgeWidth, {} };
}

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


