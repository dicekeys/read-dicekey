#pragma once

//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#include <string>
#include <vector>
#include <limits>
#include <chrono>

#include "undoverline.h"
#include "../lib-keysqr/keysqr.h"
#include "simple-ocr.h"

class FaceRead {
public:
	// Calculated purely from underline & overline.
	Undoverline underline;
	Undoverline overline;
	cv::Point2f center = cv::Point2f{ 0, 0 };
	float inferredAngleInRadians = 0;

	// Calculated after face location and angle are derived from
	// the underline and/or overline (both if possible)
	unsigned char orientationAs0to3ClockwiseTurnsFromUpright;
	std::vector<OcrResultEntry> ocrLetter;
	std::vector<OcrResultEntry> ocrDigit;

  //
	std::string toJson() const;

  char ocrLetterMostLikely() const;
  char ocrDigitMostLikely() const;

  char letter() const;
  char digit() const;

  // Return an estimate of the error in reading a face.
  // If the underline, overline, and OCR results match, the error is 0.
  // If the only error is a 1-3 bit error in either the underline or overline,
  // the result is the number of bits (hamming distance) in the underline or overline
  // that doesn't match the OCR result.
  // If the underline and overline match but matched with the OCR's second choice of
  // letter or digit, we return 2.
	FaceError error() const;
};


struct ReadFaceResult {
//	public:
	bool success;
	std::vector<FaceRead> faces;
	float angleInRadiansNonCanonicalForm;
	float pixelsPerFaceEdgeWidth;
	std::vector<FaceRead> strayFaces;
	std::vector<Undoverline> strayUndoverlines;
};

ReadFaceResult readFaces(
	const cv::Mat &grayscaleImage,
	bool outputOcrErrors = false
);
