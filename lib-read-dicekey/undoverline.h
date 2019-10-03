//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#pragma once

#include <float.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include "graphics/geometry.h"
#include "keysqr-element-face-specification.h"

const float undoverlineWidthAsFractionOfLength = ElementDimensionsMm::undoverlineThickness / ElementDimensionsMm::undoverlineLength;

class Undoverline {
public:
	bool found = false;
  bool determinedIfUnderlineOrOverline = false;
	cv::RotatedRect fromRotatedRect = cv::RotatedRect();
	Line line  = { {0, 0}, {0, 0} };
	bool isOverline = false;
	unsigned char letterDigitEncoding = 0;
	unsigned char whiteBlackThreshold = 0;
	cv::Point2f inferredDieCenter = {0, 0};
  const ElementFaceSpecification *dieFaceInferred = &NullElementFaceSpecification;
	cv::RotatedRect inferredOpposingUndoverlineRotatedRect = cv::RotatedRect();

  Undoverline() {
    //
  }

  Undoverline(
    cv::RotatedRect _fromRotatedRect,
    const Line &undoverlineStartingAtImageLeft,
    unsigned char _whiteBlackThreshold,
    const uint binaryCodingReadForwardOrBackward
  );

	const cv::RotatedRect rederiveBoundaryRect() const;
};

