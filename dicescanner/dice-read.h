//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#pragma once

#include <string>
#include <vector>
#include <limits>
#include "die-specification.h"
#include "decode-die.h"
#include "find-undoverlines.h"
#include "simple-ocr.h"
#include "bit-operations.h"


class DieRead {
public:
	// Calculated purely from underline & overline.
	Undoverline underline;
	Undoverline overline;
	cv::Point2f center = cv::Point2f{ 0, 0 };
	float inferredAngleInRadians = 0;

	// Calculated after die location and angle are derived from
	// the underline and/or overline (both if possible)
	unsigned char orientationAs0to3ClockwiseTurnsFromUpright;
	std::vector<OcrResultEntry> ocrLetter;
	std::vector<OcrResultEntry> ocrDigit;

  //
  char ocrLetterMostLikely() {
    return this->ocrLetter.size() == 0 ? '\0' : ocrLetter[0].character;
  }
  char ocrDigitMostLikely() {
    return ocrDigit.size() == 0 ? '\0' : ocrDigit[0].character;
  }

  char letter() {
    return majorityOfThree(
      underline.dieFaceInferred->letter, overline.dieFaceInferred->letter, ocrLetterMostLikely()
    );
  }
  char digit() {
    return majorityOfThree(
      underline.dieFaceInferred->digit, overline.dieFaceInferred->digit, ocrDigitMostLikely()
    );
  }

  // Return an estimate of the error in reading a die face.
  // If the underline, overline, and OCR results match, the error is 0.
  // If the only error is a 1-3 bit error in either the underline or overline,
  // the result is the number of bits (hamming distance) in the underline or overline
  // that doesn't match the OCR result.
  // If the underline and overline match but matched with the OCR's second choice of
  // letter or digit, we return 2.
	DieFaceError error() {
		if (ocrLetter.size() == 0 || ocrDigit.size() == 0) {
			return DieFaceErrors::WorstPossible;
		}
		unsigned char errorLocation = 0;
		unsigned int errorMagnitude = 0;
		const char ocrLetter0 = ocrLetter[0].character;
		const char ocrDigit0 = ocrDigit[0].character;
		const DieFaceSpecification* pUnderlineFaceInferred = underline.dieFaceInferred;
		const DieFaceSpecification* pOverlineFaceInferred = overline.dieFaceInferred;

		// Test hypothesis of no error
		if (pUnderlineFaceInferred == pOverlineFaceInferred) {
			// The underline and overline map to the same die face
			const DieFaceSpecification& undoverlineFaceInferred = *(underline.dieFaceInferred);

			// Check for OCR errors for the letter read
			if (undoverlineFaceInferred.letter != ocrLetter[0].character) {
				errorLocation |= DieFaceErrors::Location::OcrLetter;
				errorMagnitude += undoverlineFaceInferred.letter == ocrLetter[1].character ?
					DieFaceErrors::Magnitude::OcrCharacterWasSecondChoice :
					DieFaceErrors::Magnitude::OcrCharacterInvalid;
			}
			if (undoverlineFaceInferred.digit != ocrDigit[0].character) {
				errorLocation |= DieFaceErrors::Location::OcrDigit;
				errorMagnitude += undoverlineFaceInferred.digit == ocrDigit[1].character ?
					DieFaceErrors::Magnitude::OcrCharacterWasSecondChoice :
					DieFaceErrors::Magnitude::OcrCharacterInvalid;
			}
			return {(unsigned char) MIN(std::numeric_limits<unsigned char>::max(), errorMagnitude), errorLocation};
		}
		const DieFaceSpecification& underlineFaceInferred = *pUnderlineFaceInferred;
		const DieFaceSpecification& overlineFaceInferred = *pOverlineFaceInferred;
		if (underlineFaceInferred.letter == ocrLetter0 && underlineFaceInferred.digit == ocrDigit0) {
			// The underline matches the OCR result, so the error is in the overline
			return {
					overline.found ?
						// The magnitude of the error is the hamming distance error in overline
						(unsigned char)hammingDistance(underlineFaceInferred.overlineCode, overline.letterDigitEncoding) :
						// Since the overline was not found, the magntidue is specified via a constant
						DieFaceErrors::Magnitude::UnderlineOrOverlineMissing,
					DieFaceErrors::Location::Overline
				};
		}
		if (overlineFaceInferred.letter == ocrLetter0 && overlineFaceInferred.digit == ocrDigit0) {
			// Since overline matches the OCR result, so the error is in the underline
			return {
				underline.found ?
					// The magnitude of the error is the hamming distance error in underline
					(unsigned char)hammingDistance(overlineFaceInferred.underlineCode, underline.letterDigitEncoding) :				
					// Since the underline was not found, the magntidue is specified via a constant
					DieFaceErrors::Magnitude::UnderlineOrOverlineMissing,
					DieFaceErrors::Location::Underline
			};
		}
		// No good matching.  Return max error
		return {std::numeric_limits<unsigned char>::max(), std::numeric_limits<unsigned char>::max()};
	};

};
