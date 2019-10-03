#pragma once

//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#include <string>
#include <vector>
#include <limits>
#include <chrono>

#include "undoverline.h"
#include "../lib-dicekey/keysqr.h"
#include "simple-ocr.h"

class FaceRead {
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
	std::string toJson() const;

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
	FaceError error() const;
};


struct ReadDiceResult {
//	public:
	bool success;
	std::vector<FaceRead> dice;
	float angleInRadiansNonCononicalForm;
	float pixelsPerFaceEdgeWidth;
	std::vector<FaceRead> strayDice;
	std::vector<Undoverline> strayUndoverlines;
};

ReadDiceResult readFaces(
	const cv::Mat &colorImage,
	bool outputOcrErrors = false
);

KeySqr diceReadToKeySqr(
	const std::vector<FaceRead> diceRead,
	bool reportErrsToStdErr = false
);


static const std::chrono::time_point<std::chrono::system_clock> minTimePoint =
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
bool scanAndAugmentKeySqrImage(
	cv::Mat& sourceColorImageBGR_CV_8UC3,
	ResultOfScanAndAugmentKeySqrImage* result
)