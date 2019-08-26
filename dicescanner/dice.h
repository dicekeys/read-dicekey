#pragma once

#include <string>
#include <vector>
#include <limits>
#include "die-specification.h"
#include "decode-die.h"
#include "find-undoverlines.h"
#include "simple-ocr.h"
#include "bit-operations.h"

struct DieFace {
    char letter = 0;
    char digit = 0;
    unsigned char orientationAs0to3ClockwiseTurnsFromUpright = 0;
};
typedef std::vector<DieFace> DiceKey;

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
typedef std::vector<DieRead> DiceRead;



std::vector<std::vector<unsigned char>> rotationIndexes = {
  {
     0,  1,  2,  3,  4,
     5,  6,  7,  8,  9,
    10, 11, 12, 13, 14,
    15, 16, 17, 18, 19,
    20, 21, 22, 23, 24
  },
  {
     20, 15, 10,  5,  0,
     21, 16, 11,  6,  1,
     22, 17, 12,  7,  2,
     23, 18, 13,  8,  3,
     24, 19, 14,  9,  4
  },
  {
     24, 23, 22, 21, 20,
     19, 18, 17, 16, 15,
     14, 13, 12, 11, 10,
      9,  8,  7,  6,  5,
      4,  3,  2,  1,  0
  },
  {
     4,  9, 14, 19, 24,
     3,  8, 13, 18, 23,
     2,  7, 12, 17, 22,
     1,  6, 11, 16, 21,
     0,  5, 10, 15, 20,
  }
 };


std::vector<DieFace> rotateDiceKey(std::vector<DieFace> diceKey, int clockwiseTurns)
{
  if (diceKey.size() < 25) {
	  throw std::string("A DiceKey must have 25 dice, but only found " + std::to_string(diceKey.size()));
  }
  if (clockwiseTurns < 0) {
    clockwiseTurns = 4 - ((-clockwiseTurns) % 4);
  }
  const unsigned clockwiseTurns0to3 = clockwiseTurns % 4;
  std::vector<unsigned char> &indexToMoveDieFaceFrom = rotationIndexes[clockwiseTurns0to3];
  std::vector<DieFace> rotatedDiceKey;
  for (size_t i = 0; i < indexToMoveDieFaceFrom.size(); i++) {
    DieFace dieFace = diceKey[indexToMoveDieFaceFrom[i]];
    dieFace.orientationAs0to3ClockwiseTurnsFromUpright = (dieFace.orientationAs0to3ClockwiseTurnsFromUpright + clockwiseTurns) % 4;
    rotatedDiceKey.push_back(dieFace);
  }
  return rotatedDiceKey;
}

unsigned char clockwiseRotationsToCanonicalForm(std::vector<DieFace> diceKey)
{
  if (diceKey.size() < 25) {
	  throw std::string("A DiceKey must have 25 dice, but only found " + std::to_string(diceKey.size()));
  }
  unsigned char clockwiseRotationsRequired = 0;
  for (unsigned char candidateRotationRequirement = 1; candidateRotationRequirement < 4; candidateRotationRequirement++) {
    // If the candidate rotation would result in the square having a top-left letter
    // that is earlier in sort order (lower unicode character) than the current rotation,
    // replace the current rotation with the candidate rotation.
    if (diceKey[rotationIndexes[candidateRotationRequirement][0]].letter < diceKey[rotationIndexes[clockwiseRotationsRequired][0]].letter) {
      clockwiseRotationsRequired = candidateRotationRequirement;
    }    
  }
  return clockwiseRotationsRequired;
}

std::vector<DieFace> rotateToCanonicalDiceKey(std::vector<DieFace> diceKey)
{
  return rotateDiceKey(diceKey, clockwiseRotationsToCanonicalForm(diceKey));
}
