//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#include <float.h>
#include "utilities/statistics.h"
#include "graphics/geometry.h"
#include "graphics/cv.h"
#include "graphics/rotate.h"
#include "dicekey-face-specification.h"
#include "simple-ocr.h"
#include "font.h"
#include "write-face-characters.h"

void writeFaceCharacters(
	cv::Mat& imageColor,
	cv::Point2f faceCenter,
	float angleInRadians,
	float pixelsPerFaceEdgeWidth,
	char letter,
	char digit,
	Color letterColor,
	Color digitColor
) {
	const OcrFont &font = *getFont();
	const float textHeightDestinationPixels = FaceDimensionsFractional::textRegionHeight * pixelsPerFaceEdgeWidth;
	const float textWidthDestinationPixels = FaceDimensionsFractional::textRegionWidth * pixelsPerFaceEdgeWidth;
	const float destinationPixelsBetweenLetterAndDigit = FaceDimensionsFractional::spaceBetweenLetterAndDigit * pixelsPerFaceEdgeWidth;
	float charWidthDestinationPixels = (textWidthDestinationPixels - destinationPixelsBetweenLetterAndDigit) / 2;

	const float centerSpaceInOriginPixels = FaceDimensionsFractional::spaceBetweenLetterAndDigit * pixelsPerFaceEdgeWidth *
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

	const float letterTopLeftX = faceCenter.x +
		letterLeftInOriginPixels * deltaXFromSourceChangeInX +
		textTopInOriginPixels * deltaXFromSourceChangeInY;
	const float letterTopLeftY = faceCenter.y +
		letterLeftInOriginPixels * deltaYFromSourceChangeInX +
		textTopInOriginPixels * deltaYFromSourceChangeInY;
	const float digitTopLeftX = faceCenter.x +
		digitLeftInOriginPixels * deltaXFromSourceChangeInX +
		textTopInOriginPixels * deltaXFromSourceChangeInY;
	const float digitTopLeftY = faceCenter.y +
		digitLeftInOriginPixels * deltaYFromSourceChangeInX +
		textTopInOriginPixels * deltaYFromSourceChangeInY;

	const OcrChar* letterRecord = vreduce<OcrChar, const OcrChar*>( font.letters.characters,
		[letter](const OcrChar* r, const OcrChar* c) -> const OcrChar* {return c->character == letter ? c : r; },
		(const OcrChar*)(NULL)
	);

	const OcrChar* digitRecord = vreduce<OcrChar, const OcrChar*>(font.digits.characters,
		[digit](const OcrChar* r, const OcrChar* c) -> const OcrChar * {return c->character == digit ? c : r; },
		(const OcrChar*)(NULL)
		);

	if (letterRecord) {
		for (auto p : letterRecord->outlinePoints) {
			const int x = int(round(letterTopLeftX + deltaXFromSourceChangeInX * p.x + deltaXFromSourceChangeInY * p.y));
			const int y = int(round(letterTopLeftY + deltaYFromSourceChangeInX * p.x + deltaYFromSourceChangeInY * p.y));
			if (x >= 0 || y >= 0 || x < imageColor.cols || y < imageColor.rows) {
				imageColor.at<cv::Vec4b>(y, x) = letterColor.scalarRGBA;
			}
	}
	}

	if (digitRecord) {
		for (auto p : digitRecord->outlinePoints) {
			const int x = int(round(digitTopLeftX + deltaXFromSourceChangeInX * p.x + deltaXFromSourceChangeInY * p.y));
			const int y = int(round(digitTopLeftY + deltaYFromSourceChangeInX * p.x + deltaYFromSourceChangeInY * p.y));
			if (x >= 0 || y >= 0 || x < imageColor.cols || y < imageColor.rows) {
				imageColor.at<cv::Vec4b>(y, x) = digitColor.scalarRGBA;
			}
		}
	}
}