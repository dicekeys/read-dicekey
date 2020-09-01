//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#include <float.h>
#include <chrono>
#include <iostream>
#include <math.h>

#include "utilities/vfunctional.h"
#include "utilities/statistics.h"
#include "utilities/bit-operations.h"
#include "graphics/cv.h"
#include "graphics/geometry.h"
#include "simple-ocr.h"
#include "decode-face.h"
#include "face-read.h"
#include "json.h"

namespace FaceErrors {
  const FaceError WorstPossible = {
    FaceErrors::Magnitude::Max,
    FaceErrors::Location::All
  };

  const FaceError None = { 0, 0 };
}

FaceRead FaceRead::rotate(int clockwiseTurnsToRight) const
{
  return FaceRead(
    underline,
    overline,
    (int) orientationAs0to3ClockwiseTurnsFromUpright() == '?' ? '?' :
      clockwiseTurnsToRange0To3(orientationAs0to3ClockwiseTurnsFromUpright() + clockwiseTurnsToRight),
    ocrLetterFromMostToLeastLikely,
    ocrDigitFromMostToLeastLikely
  );
}

char FaceRead::ocrLetterMostLikely() const {
	return ocrLetterFromMostToLeastLikely.length() == 0 ? '?' : ocrLetterFromMostToLeastLikely[0];
}
char FaceRead::ocrDigitMostLikely() const {
	return ocrDigitFromMostToLeastLikely.length() == 0 ? '?' : ocrDigitFromMostToLeastLikely[0];
}

char FaceRead::letter() const {
	const char letter = majorityOfThree(
		underline.faceInferred->letter, overline.faceInferred->letter, ocrLetterMostLikely()
	);
	return letter != 0 ? letter : '?';
}
char FaceRead::digit() const {
	const char digit = majorityOfThree(
		underline.faceInferred->digit, overline.faceInferred->digit, ocrDigitMostLikely()
	);
	return digit != 0 ? digit : '?';
}

char FaceRead::orientationAs0to3ClockwiseTurnsFromUpright() const {
	return _orientationAs0to3ClockwiseTurnsFromUpright;
}

unsigned int FaceRead::errorSize() const {
	return error().magnitude;
}

std::string FaceRead::toJson() const {
	std::ostringstream jsonStream;
	jsonStream <<
		"{" <<
			"\"" << JsonKeys::FaceRead::underline << "\": " << underline.toJson() << ", " <<
			"\"" << JsonKeys::FaceRead::overline << "\": " << overline.toJson() << ", " <<
			"\"" << JsonKeys::FaceRead::center << "\": " << pointToJson(center()) << ", " <<
			"\"" << JsonKeys::FaceRead::orientationAsLowercaseLetterTrbl << "\": \"" << std::string(1, orientationAsLowercaseLetterTRBL()) << "\"," <<
			"\"" << JsonKeys::FaceRead::ocrLetterCharsFromMostToLeastLikely << "\": \"" <<
				ocrLetterFromMostToLeastLikely << "\", " <<
			"\"" << JsonKeys::FaceRead::ocrDigitCharsFromMostToLeastLikely << "\": \"" <<
				ocrDigitFromMostToLeastLikely << "\"" <<
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
		if (ocrLetterFromMostToLeastLikely.length() == 0 || ocrDigitFromMostToLeastLikely.length() == 0) {
			return FaceErrors::WorstPossible;
		}
		unsigned char errorLocation = 0;
		unsigned int errorMagnitude = 0;
		const char ocrLetter0 = ocrLetterMostLikely();
		const char ocrDigit0 = ocrDigitMostLikely();
		const FaceSpecification* pUnderlineFaceInferred = underline.faceInferred;
		const FaceSpecification* pOverlineFaceInferred = overline.faceInferred;

		// Test hypothesis of no error
		if (pUnderlineFaceInferred == pOverlineFaceInferred) {
			// The underline and overline map to the same face
			const FaceSpecification& undoverlineFaceInferred = *(underline.faceInferred);

			// Check for OCR errors for the letter read
			if (undoverlineFaceInferred.letter != ocrLetterMostLikely()) {
				errorLocation |= FaceErrors::Location::OcrLetter;
				errorMagnitude += undoverlineFaceInferred.letter == ocrLetterFromMostToLeastLikely[1] ?
					FaceErrors::Magnitude::OcrCharacterWasSecondChoice :
					FaceErrors::Magnitude::OcrCharacterInvalid;
			}
			if (undoverlineFaceInferred.digit != ocrDigitMostLikely()) {
				errorLocation |= FaceErrors::Location::OcrDigit;
				errorMagnitude += undoverlineFaceInferred.digit == ocrDigitFromMostToLeastLikely[1] ?
					FaceErrors::Magnitude::OcrCharacterWasSecondChoice :
					FaceErrors::Magnitude::OcrCharacterInvalid;
			}
			return {(unsigned char) MIN(std::numeric_limits<unsigned char>::max(), errorMagnitude), errorLocation};
		}
		const FaceSpecification& underlineFaceInferred = *pUnderlineFaceInferred;
		const FaceSpecification& overlineFaceInferred = *pOverlineFaceInferred;
		if (underlineFaceInferred.letter == ocrLetter0 && underlineFaceInferred.digit == ocrDigit0) {
			// The underline matches the OCR result, so the error is in the overline
			return {
					overline.found ?
						// The magnitude of the error is the hamming distance error in overline
						(unsigned char)hammingDistance(underlineFaceInferred.overlineCode, overline.letterDigitEncoding) :
						// Since the overline was not found, the magnitude is specified via a constant
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
					// Since the underline was not found, the magnitude is specified via a constant
					FaceErrors::Magnitude::UnderlineOrOverlineMissing,
					FaceErrors::Location::Underline
			};
		}
		// No good matching.  Return max error
		return {std::numeric_limits<unsigned char>::max(), std::numeric_limits<unsigned char>::max()};
	};
