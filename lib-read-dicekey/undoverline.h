//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#pragma once

#include <float.h>
#include "graphics/cv.h"
#include "graphics/geometry.h"
#include "../lib-dicekey/externally-generated/dicekey-face-specification.h"

const float undoverlineWidthAsFractionOfLength = FaceDimensionsFractional::undoverlineThickness / FaceDimensionsFractional::undoverlineLength;

class Undoverline {
public:
	bool found = false;
  bool determinedIfUnderlineOrOverline = false;
	cv::RotatedRect fromRotatedRect = cv::RotatedRect();
	Line line  = { {0, 0}, {0, 0} };
  cv::Point2f center;
	bool isOverline = false;
	unsigned char letterDigitEncoding = 0;
	unsigned char whiteBlackThreshold = 0;
  cv::Point2f inferredCenterOfFace = { 0, 0 };
  cv::Point2f inferredOpposingUndoverlineCenter = { 0, 0 };
  const FaceSpecification *faceInferred = &NullFaceSpecification;
	cv::RotatedRect inferredOpposingUndoverlineRotatedRect = cv::RotatedRect();

  Undoverline() {
    //
  }

  const std::string toJson() const;

  Undoverline(
    cv::RotatedRect _fromRotatedRect,
    const Line &undoverlineStartingAtImageLeft,
    unsigned char _whiteBlackThreshold,
    const unsigned int binaryCodingReadForwardOrBackward
  );

	const cv::RotatedRect rederiveBoundaryRect() const;
};

