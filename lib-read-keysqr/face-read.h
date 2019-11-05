//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#pragma once


#include <string>
#include <vector>
#include "graphics/cv.h"
#include "graphics/geometry.h"

#include "undoverline.h"
#include "keysqr.h"
#include "face.h"
#include "simple-ocr.h"

class FaceUndoverlines {
  public:
	Undoverline underline;
	Undoverline overline;

	FaceUndoverlines() :
		underline({}),
		overline({})
  {}

	FaceUndoverlines(const FaceUndoverlines &copyFrom) :
		underline(copyFrom.underline),
		overline(copyFrom.overline)
  {}

  FaceUndoverlines(
		const Undoverline _underline,
		const Undoverline _overline
  ) :
		underline(_underline),
		overline(_overline)
  {}

  cv::Point2f center() const {
    return (underline.found && overline.found) ?
      midpoint2f(midpointOfLine(underline.line), midpointOfLine(overline.line)) :
    underline.found ?
      underline.inferredCenterOfFace :
    overline.found ?
      overline.inferredCenterOfFace :
      cv::Point2f(0, 0);
  }

  float inferredAngleInRadians() const {
    if (underline.found && overline.found) {
      const Line lineFromUnderlineCenterToOverlineCenter = {
        midpointOfLine(underline.line), midpointOfLine(overline.line)
      };
      // The angle of the face is the angle of that line, plus 90 degrees clockwise
      const float angleOfLineFromUnderlineToOverlineCenterInRadians =
        angleOfLineInSignedRadians2f(lineFromUnderlineCenterToOverlineCenter);
      float angleInRadians = angleOfLineFromUnderlineToOverlineCenterInRadians +
        NinetyDegreesAsRadians;
      if (angleInRadians > (M_PI)) {
        angleInRadians -= float(2 * M_PI);
      }
      return angleInRadians;
    } else if (underline.found) {
      return angleOfLineInSignedRadians2f(underline.line);
    } else if (overline.found) {
      return angleOfLineInSignedRadians2f(overline.line);
    } else {
      return 0;
    }
  }
};


class FaceUndoverlinesCenterOnly : public FaceUndoverlines {
private:
  cv::Point2f _center;

public:
  cv::Point2f center() const { return _center; };

  FaceUndoverlinesCenterOnly(cv::Point2f __center) :
    FaceUndoverlines(),
    _center(__center)
    {}
};


struct FaceError {
	unsigned char magnitude;
	unsigned char location;
};

namespace FaceErrors {
  namespace Location {
    const unsigned char Underline = 1;
    const unsigned char Overline = 2;
    const unsigned char OcrLetter = 4;
    const unsigned char OcrDigit = 8;
    const unsigned char All = Underline + Overline + OcrLetter + OcrDigit;
  }

  namespace Magnitude {
    const unsigned char OcrCharacterWasSecondChoice = 2;
    const unsigned char UnderlineOrOverlineMissing = 2;
    const unsigned char OcrCharacterInvalid = 8;
    const unsigned char Max = std::numeric_limits<unsigned char>::max();
  }

  extern const FaceError WorstPossible;
  extern const FaceError None;
}

class FaceRead: public FaceUndoverlines, public IFace, public Rotateable<FaceRead> {
private:
	char _orientationAs0to3ClockwiseTurnsFromUpright;
  std::string ocrLetterFromMostToLeastLikely;
  std::string ocrDigitFromMostToLeastLikely;
public:

	FaceRead() :
    FaceUndoverlines(),
		_orientationAs0to3ClockwiseTurnsFromUpright(0),
    ocrLetterFromMostToLeastLikely(""),
    ocrDigitFromMostToLeastLikely("")
	{}

  FaceRead(
		const FaceUndoverlines undoverlines,
		const char __orientationAs0to3ClockwiseTurnsFromUpright,
    const std::string _ocrLetterFromMostToLeastLikely,
    const std::string _ocrDigitFromMostToLeastLikely
	) :
    FaceUndoverlines(undoverlines.underline, undoverlines.overline),
		_orientationAs0to3ClockwiseTurnsFromUpright(__orientationAs0to3ClockwiseTurnsFromUpright),
    ocrLetterFromMostToLeastLikely(_ocrLetterFromMostToLeastLikely),
    ocrDigitFromMostToLeastLikely(_ocrDigitFromMostToLeastLikely)

  {}

	FaceRead(
		const Undoverline _underline,
		const Undoverline _overline,
		const int __orientationAs0to3ClockwiseTurnsFromUpright,
    const std::string _ocrLetterFromMostToLeastLikely,
    const std::string _ocrDigitFromMostToLeastLikely
	) :
    FaceUndoverlines(_underline, _overline),
		_orientationAs0to3ClockwiseTurnsFromUpright(__orientationAs0to3ClockwiseTurnsFromUpright),
    ocrLetterFromMostToLeastLikely(_ocrLetterFromMostToLeastLikely),
    ocrDigitFromMostToLeastLikely(_ocrDigitFromMostToLeastLikely)
  {}


  FaceRead rotate(int clockwiseTurnsToRight) const;

	// Calculated after face location and angle are derived from
	// the underline and/or overline (both if possible)

  //
	std::string toJson() const;

  char ocrLetterMostLikely() const;
  char ocrDigitMostLikely() const;

  char letter() const;
  char digit() const;
	char orientationAs0to3ClockwiseTurnsFromUpright() const;

  // Return an estimate of the error in reading a face.
  // If the underline, overline, and OCR results match, the error is 0.
  // If the only error is a 1-3 bit error in either the underline or overline,
  // the result is the number of bits (hamming distance) in the underline or overline
  // that doesn't match the OCR result.
  // If the underline and overline match but matched with the OCR's second choice of
  // letter or digit, we return 2.
	FaceError error() const;

	unsigned int errorSize() const;
};