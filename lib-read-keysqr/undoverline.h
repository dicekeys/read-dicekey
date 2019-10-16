//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#pragma once

#include <float.h>
#include "graphics/cv.h"
#include "graphics/geometry.h"
#include "keysqr-element-face-specification.h"

const float undoverlineWidthAsFractionOfLength = ElementDimensionsFractional::undoverlineThickness / ElementDimensionsFractional::undoverlineLength;

class Undoverline {
public:
	bool found = false;
  bool determinedIfUnderlineOrOverline = false;
	cv::RotatedRect fromRotatedRect = cv::RotatedRect();
	Line line  = { {0, 0}, {0, 0} };
	bool isOverline = false;
	unsigned char letterDigitEncoding = 0;
	unsigned char whiteBlackThreshold = 0;
	cv::Point2f inferredCenterOfFace = {0, 0};
  const ElementFaceSpecification *faceInferred = &NullElementFaceSpecification;
	cv::RotatedRect inferredOpposingUndoverlineRotatedRect = cv::RotatedRect();

  Undoverline() {
    //
  }

  const std::string toJson() const;

  Undoverline(
    cv::RotatedRect _fromRotatedRect,
    const Line &undoverlineStartingAtImageLeft,
    unsigned char _whiteBlackThreshold,
    const uint binaryCodingReadForwardOrBackward
  );

	const cv::RotatedRect rederiveBoundaryRect() const;
};

