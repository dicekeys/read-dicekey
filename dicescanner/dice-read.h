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
      underline.dieFaceInferred()->letter, overline.dieFaceInferred()->letter, ocrLetterMostLikely()
    );
  }
  char digit() {
    return majorityOfThree(
      underline.dieFaceInferred()->digit, overline.dieFaceInferred()->digit, ocrDigitMostLikely()
    );
  }


  // Return an estimate of the error in reading a die face.
  // If the underline, overline, and OCR results match, the error is 0.
  // If the only error is a 1-3 bit error in either the underline or overline,
  // the result is the number of bits (hamming distance) in the underline or overline
  // that doesn't match the OCR result.
  // If the underline and overline match but matched with the OCR's second choice of
  // letter or digit, we return 2.
	int error() {
		if (ocrLetter.size() == 0 || ocrDigit.size() == 0) {
			return std::numeric_limits<int>::max();
		}
		const char ocrLetter0 = ocrLetter[0].character;
		const char ocrDigit0 = ocrDigit[0].character;
		const DieFaceSpecification& underlineFaceInferred = *(underline.dieFaceInferred());
		const DieFaceSpecification& overlineFaceInferred = *(overline.dieFaceInferred());

		// Test hypothesis of no error
		if (underlineFaceInferred.letter == overlineFaceInferred.letter &&
			underlineFaceInferred.letter == overlineFaceInferred.digit
			) {
			if (underlineFaceInferred.letter == ocrLetter0 &&
				underlineFaceInferred.digit == ocrDigit0) {
				// Underline, overline, and ocr all agree.  No errors, so return 0 error.
				return 0;
			}
			// If one character is correct but the other is the second choice
			if (
				(underlineFaceInferred.letter == ocrLetter0 &&
					underlineFaceInferred.digit == ocrDigit[1].character
					) ||
					(underlineFaceInferred.letter == ocrLetter[1].character &&
						underlineFaceInferred.digit == ocrDigit0
						)
				) {
				// We assign this as equiavelent to a two-bit error;
				return 2;
			}
		}
		if (underlineFaceInferred.letter == ocrLetter0 && underlineFaceInferred.digit == ocrDigit0) {
			return overline.found ?
				// Since the OCR matched the underline, bit error is hamming distance error in overline
				hammingDistance(underlineFaceInferred.overlineCode, overline.letterDigitEncoding) :
				// Since the overline wasn't found, we'll treat it as a two-bit error
				2;
		}
		if (overlineFaceInferred.letter == ocrLetter0 && overlineFaceInferred.digit == ocrDigit0) {
			return underline.found ?
				// Since the OCR matched the underline, bit error is hamming distance error in underline
				hammingDistance(overlineFaceInferred.underlineCode, underline.letterDigitEncoding) :
				// Since the overline wasn't found, we'll treat it as a two-bit error
				2;
		}
		// No good matching.  Return maxint
		return std::numeric_limits<int>::max();
	};

};
