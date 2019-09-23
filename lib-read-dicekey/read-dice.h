#pragma once

//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#include <string>
#include <vector>
#include <limits>
#include "undoverline.h"
#include "../lib-dicekey/dicekey.h"
#include "simple-ocr.h"

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
  char ocrLetterMostLikely() const;
  char ocrDigitMostLikely() const;

  char letter() const;
  char digit() const;

  // Return an estimate of the error in reading a die face.
  // If the underline, overline, and OCR results match, the error is 0.
  // If the only error is a 1-3 bit error in either the underline or overline,
  // the result is the number of bits (hamming distance) in the underline or overline
  // that doesn't match the OCR result.
  // If the underline and overline match but matched with the OCR's second choice of
  // letter or digit, we return 2.
	DieFaceError error() const;
};


struct ReadDiceResult {
//	public:
	bool success;
	std::vector<DieRead> dice;
	float angleInRadiansNonCononicalForm;
	float pixelsPerMm;
	std::vector<DieRead> strayDice;
	std::vector<Undoverline> strayUndoverlines;
};

ReadDiceResult readDice(
	const cv::Mat &colorImage,
	bool outputOcrErrors = false
);

DiceKey diceReadToDiceKey(
	const std::vector<DieRead> diceRead,
	bool reportErrsToStdErr = false
);