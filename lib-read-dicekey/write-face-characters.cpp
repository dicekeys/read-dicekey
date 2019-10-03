//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#include <float.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

//#include <iostream>
#include "utilities/statistics.h"
#include "graphics/geometry.h"
#include "keysqr-element-face-specification.h"
#include "graphics/rotate.h"
#include "simple-ocr.h"
#include "font.h"
#include "write-face-characters.h"

void writeFaceCharacters(
	cv::Mat& imageColor,
	cv::Point2f dieCenter,
	float angleInRadians,
	float pixelsPerFaceEdgeWidth,
	char letter,
	char digit,
	Color letterColor,
	Color digitColor
) {
	const OcrFont font = getFont();
	const float textHeightDestinationPixels = ElementDimensionsFractional::textRegionHeight * pixelsPerFaceEdgeWidth;
	const float textWidthDestinationPixels = ElementDimensionsFractional::textRegionWidth * pixelsPerFaceEdgeWidth;
	const float destinationPixelsBetweenLetterAndDigit = ElementDimensionsFractional::spaceBetweenLetterAndDigit * pixelsPerFaceEdgeWidth;
	float charWidthDestinationPixels = (textWidthDestinationPixels - destinationPixelsBetweenLetterAndDigit) / 2;

	const float centerSpaceInOriginPixels = ElementDimensionsFractional::spaceBetweenLetterAndDigit * pixelsPerFaceEdgeWidth *
		font.outlineCharWidthInPixels / charWidthDestinationPixels;
	const float textTopInOriginPixels = -float(font.outlineCharHeightInPixels) / 2;
	const float letterLeftInOriginPixels = -(font.outlineCharWidthInPixels + centerSpaceInOriginPixels/2);
	const float digitLeftInOriginPixels = centerSpaceInOriginPixels / 2;
	
	const float deltaXFraction = charWidthDestinationPixels / float(font.outlineCharWidthInPixels);
	const float deltaYFraction = textHeightDestinationPixels / float(font.outlineCharHeightInPixels);

	const float deltaXFromSourceChangeInX = deltaXFraction * cos(-angleInRadians);
	const float deltaXFromSourceChangeInY = deltaYFraction * sin(-angleInRadians);
	const float deltaYFromSourceChangeInX = deltaXFraction * cos(float(-angleInRadians + M_PI / 2));
	const float deltaYFromSourceChangeInY = deltaYFraction * sin(float(-angleInRadians + M_PI / 2));

	const float letterTopLeftX = dieCenter.x +
		letterLeftInOriginPixels * deltaXFromSourceChangeInX +
		textTopInOriginPixels * deltaXFromSourceChangeInY;
	const float letterTopLeftY = dieCenter.y +
		letterLeftInOriginPixels * deltaYFromSourceChangeInX +
		textTopInOriginPixels * deltaYFromSourceChangeInY;
	const float digitTopLeftX = dieCenter.x +
		digitLeftInOriginPixels * deltaXFromSourceChangeInX +
		textTopInOriginPixels * deltaXFromSourceChangeInY;
	const float digitTopLeftY = dieCenter.y +
		digitLeftInOriginPixels * deltaYFromSourceChangeInX +
		textTopInOriginPixels * deltaYFromSourceChangeInY;

	const OcrChar* letterRecord = vreduce<OcrChar, const OcrChar*>( font.letters,
		[letter](const OcrChar* r, const OcrChar* c) -> const OcrChar* {return c->character == letter ? c : r; },
		(const OcrChar*)(NULL)
	);

	const OcrChar* digitRecord = vreduce<OcrChar, const OcrChar*>(font.digits,
		[digit](const OcrChar* r, const OcrChar* c) -> const OcrChar * {return c->character == digit ? c : r; },
		(const OcrChar*)(NULL)
		);

	if (letterRecord) {
		for (auto p : letterRecord->outlinePoints) {
			const int x = int(round(letterTopLeftX + deltaXFromSourceChangeInX * p.x + deltaXFromSourceChangeInY * p.y));
			const int y = int(round(letterTopLeftY + deltaYFromSourceChangeInX * p.x + deltaYFromSourceChangeInY * p.y));
			imageColor.at<cv::Vec3b>(y, x)[0] = letterColor.b;
			imageColor.at<cv::Vec3b>(y, x)[1] = letterColor.g;
			imageColor.at<cv::Vec3b>(y, x)[2] = letterColor.r;
		}
	}

	if (digitRecord) {
		for (auto p : digitRecord->outlinePoints) {
			const int x = int(round(digitTopLeftX + deltaXFromSourceChangeInX * p.x + deltaXFromSourceChangeInY * p.y));
			const int y = int(round(digitTopLeftY + deltaYFromSourceChangeInX * p.x + deltaYFromSourceChangeInY * p.y));
			imageColor.at<cv::Vec3b>(y, x)[0] = digitColor.b;
			imageColor.at<cv::Vec3b>(y, x)[1] = digitColor.g;
			imageColor.at<cv::Vec3b>(y, x)[2] = digitColor.r;
		}
	}
}