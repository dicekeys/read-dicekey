//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#include <float.h>
#include <iostream>
#include <math.h>
#include "utilities/vfunctional.h"
#include "graphics/cv.h"
#include "graphics/geometry.h"
#include "keysqr-face-specification.h"
#include "decode-face.h"
#include "undoverline.h"
#include "json.h"

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

	// If the face was up-side down, the underline would appear at the top of the face,
	// and when we scanned it from image left to right we read the bits in reverse order.
	// To determine the actual direction of the face, we will need to reverse it in situations
	// where the orientation bits reveal that we read it in reverse order.
	// This yields a line directed from the side of the face that would be on the left if it were
	// not rotated (the side on which the letter appears) to the right side of the face if it were
	// not rotated (the side on which the digit appears)
	line = decoded.wasReadInReverseOrder ?
		reverseLineDirection(undoverlineStartingAtImageLeft) :
		undoverlineStartingAtImageLeft;

	faceInferred = decodeUndoverlineByte(isOverline, letterDigitEncoding);

	float upAngleInRadians = angleOfLineInSignedRadians2f(line) +
		(decoded.isOverline ? NinetyDegreesAsRadians : -NinetyDegreesAsRadians);

	// calculate the number of pixels that a face is long/wide (same since square)
	double pixelsPerElementEdgeLength = double(undoverlineLength) / FaceDimensionsFractional::undoverlineLength;

	float pixelsFromCenterOfUndoverlineToCenterOfFace = float(
		FaceDimensionsFractional::centerOfUndoverlineToCenterOfFace *
		pixelsPerElementEdgeLength
	);
	float pixelsBetweenCentersOfUndoverlines = 2 * pixelsFromCenterOfUndoverlineToCenterOfFace;

	const cv::Point2f lineCenter = midpointOfLine(undoverlineStartingAtImageLeft);
	const auto x = lineCenter.x + pixelsFromCenterOfUndoverlineToCenterOfFace * cos(upAngleInRadians);
	const auto y = lineCenter.y + pixelsFromCenterOfUndoverlineToCenterOfFace * sin(upAngleInRadians);
	inferredCenterOfFace = cv::Point2f(x, y);

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

const std::string Undoverline::toJson() const {
	if (!found || !determinedIfUnderlineOrOverline) {
		return "null";
	}
	const cv::Point2f center = midpointOfLine(line);
	std::ostringstream jsonStream;
	jsonStream <<
	"{" <<
		JsonKeys::Undoverline::code << ": " << letterDigitEncoding << "," <<
		JsonKeys::Undoverline::line << ": " << lineToJson(line) <<
		// "center: {" <<
		// 	"x: " << center.x << ", " <<
		// 	"y: " << center.y << "" <<
		// "}, " <<
		// "angleInRadians: " << angleOfLineInSignedRadians2f(line) << "," <<
		// "lengthInPixels: " << lineLength(line) << "," <<
		// "letterDigitEncoding: " << letterDigitEncoding << "," <<
		// "whiteBlackThreshold: " << whiteBlackThreshold << "" <<
	"}";
	return jsonStream.str();
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

