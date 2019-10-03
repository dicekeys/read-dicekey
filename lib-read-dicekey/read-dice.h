#pragma once

//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#include <string>
#include <vector>
#include <limits>
#include "undoverline.h"
#include "../lib-dicekey/keysqr.h"
#include "simple-ocr.h"

class ElementRead {
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
	std::string toJson() const {
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
	ElementFaceError error() const;
};


struct ReadDiceResult {
//	public:
	bool success;
	std::vector<ElementRead> dice;
	float angleInRadiansNonCononicalForm;
	float pixelsPerFaceEdgeWidth;
	std::vector<ElementRead> strayDice;
	std::vector<Undoverline> strayUndoverlines;
};

ReadDiceResult readDice(
	const cv::Mat &colorImage,
	bool outputOcrErrors = false
);

KeySqr diceReadToDKeySqr(
	const std::vector<ElementRead> diceRead,
	bool reportErrsToStdErr = false
);