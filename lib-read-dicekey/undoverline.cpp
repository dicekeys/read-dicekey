//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#include <float.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include <iostream>
#include <math.h>
#include "utilities/vfunctional.h"
#include "graphics/geometry.h"
#include "die-face-specification.h"
#include "decode-die.h"
#include "undoverline.h"

Undoverline::Undoverline(
	cv::RotatedRect _fromRotatedRect,
	const Line &undoverlineStartingAtImageLeft,
	unsigned char _whiteBlackThreshold,
	const uint binaryCodingReadForwardOrBackward
) {
	fromRotatedRect = _fromRotatedRect;
	whiteBlackThreshold = _whiteBlackThreshold;
	found = true;
	const float undoverlineLength = lineLength(undoverlineStartingAtImageLeft);
	const bool isVertical =
		abs(undoverlineStartingAtImageLeft.end.x - undoverlineStartingAtImageLeft.start.x) <
		abs(undoverlineStartingAtImageLeft.end.y - undoverlineStartingAtImageLeft.start.y);

	// FIXME -- remove debugging when all works.
		//cv::imwrite("undoverlineStartingAtImageLeft-highlighted.png", highlightUndoverline(colorImage, rectEncompassingLine));
		// const cv::Point2f center = midpointOfLine(undoverlineStartingAtImageLeft);
		// const float lineLen = lineLength(undoverlineStartingAtImageLeft);
		// const float angle = angleOfLineInSignedDegrees2f(undoverlineStartingAtImageLeft);
		//cv::imwrite("underline-isolated.png", copyRotatedRectangle(
		//	grayscaleImage,
		//	center,
		//	angle,
		//	cv::Size2f( lineLen, lineLen * undoverlineWidthAsFractionOfLength )));

	const auto decoded = decodeUndoverline11Bits(binaryCodingReadForwardOrBackward, isVertical);
	if (!decoded.isValid) {
		return;
	}
	// We at least were able to determine if this was an overline or underline
	determinedIfUnderlineOrOverline = true;
	isOverline = decoded.isOverline;
	letterDigitEncoding = decoded.letterDigitEncoding;

	// If the die was up-side down, the underline would appear at the top of the die,
	// and when we scanned it from image left to right we read the bits in reverse order.
	// To determine the actual direction of the die, we will need to reverse it in situations
	// where the orientation bits reveal that we read it in reverse order.
	// This yields a line directed from the side of the die that would be on the left if it were
	// not rotated (the side on which the letter appears) to the right side of the die if it were
	// not rotated (the side on which the digit appears)
	line = decoded.wasReadInReverseOrder ?
		reverseLineDirection(undoverlineStartingAtImageLeft) :
		undoverlineStartingAtImageLeft;

	dieFaceInferred = decodeUndoverlineByte(isOverline, letterDigitEncoding);

	float upAngleInRadians = angleOfLineInSignedRadians2f(line) +
		(decoded.isOverline ? NinetyDegreesAsRadians : -NinetyDegreesAsRadians);

	// pixels per mm the length of the overline in pixels of it's length in mm,
	// or, undoverlineLength / mmDieUndoverlineLength;
	double mmToPixels = double(undoverlineLength) / DieDimensionsMm::undoverlineLength;

	float pixelsFromCenterOfUndoverlineToCenterOfDie = float(
		DieDimensionsMm::centerOfUndoverlineToCenterOfDie *
		mmToPixels
	);
	float pixelsBetweenCentersOfUndoverlines = 2 * pixelsFromCenterOfUndoverlineToCenterOfDie;

	const cv::Point2f lineCenter = midpointOfLine(undoverlineStartingAtImageLeft);
	const auto x = lineCenter.x + pixelsFromCenterOfUndoverlineToCenterOfDie * cos(upAngleInRadians);
	const auto y = lineCenter.y + pixelsFromCenterOfUndoverlineToCenterOfDie * sin(upAngleInRadians);
	inferredDieCenter = cv::Point2f(x, y);

	cv::Point2f inferredOpposingUnderlineCenter(
		fromRotatedRect.center.x + pixelsBetweenCentersOfUndoverlines * cos(upAngleInRadians),
		fromRotatedRect.center.y + pixelsBetweenCentersOfUndoverlines * sin(upAngleInRadians)
	);

	inferredOpposingUndoverlineRotatedRect = cv::RotatedRect(
		inferredOpposingUnderlineCenter,
		fromRotatedRect.size,
		fromRotatedRect.angle
	);
}

const cv::RotatedRect Undoverline::rederiveBoundaryRect() const {
	const cv::Point2f center = midpointOfLine(line);
	const float length = lineLength(line);
	const float height = undoverlineWidthAsFractionOfLength * length;
	const float angleInDegrees = angleOfLineInSignedDegrees2f(line);
	const float absAngleInDegrees = abs(angleInDegrees);
	const bool isHorizontalish = absAngleInDegrees <= 45 || absAngleInDegrees > 135;
	const cv::Size2f rectSize = isHorizontalish ? cv::Size2f(length, height) : cv::Size2f(height, length);
	return cv::RotatedRect(center, rectSize, angleInDegrees); // float(angle + M_PI/2)
};

