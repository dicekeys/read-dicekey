#pragma once

#include <float.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include <iostream>
#include "vfunctional.h"
#include "rectangle.h"
#include "find-squares.h"
#include "die.h"


static uint reverseBits(uint bitsToReverse, uint lengthInBits)
{
	uint reversedBits = 0;
	for (uint i = 0; i < lengthInBits; i++) {
		reversedBits <<= 1;
		if (bitsToReverse & 1) {
			reversedBits += 1;
		}
		bitsToReverse >>= 1;
	}
	return reversedBits;
}

static float normalizeAngle(float angle) {
	const float normalizedAngle = angle - round(angle / 90) * 90;
	return normalizedAngle;
}

// returns sequence of squares detected on the image.
static std::vector<RectangleDetected> findUndoverlines(const cv::Mat &gray, int N = 13)
{
	float min_underline_length = float(std::min(gray.size[0], gray.size[1])) / 80;
	float max_underline_length = float(std::min(gray.size[0], gray.size[1])) / 8;

	std::vector<RectangleDetected> candidateUnderOverLines = vfilter<RectangleDetected>(
		findRectangles(gray, N), isRectangleShapedLikeOverUnderline);

	if (candidateUnderOverLines.size() > 25) {
		// Remove rectangles that stray from the median

		float medianArea = median(vmap<RectangleDetected, float>(candidateUnderOverLines,
			[](RectangleDetected r) -> float { return r.area; }));
		float minArea = 0.75f * medianArea;
		float maxArea = medianArea / 0.75f;
		candidateUnderOverLines = vfilter<RectangleDetected>(candidateUnderOverLines, [minArea, maxArea](RectangleDetected r) {
			return  (r.area >= minArea && r.area <= maxArea);
			});

		// Recalculate median for survivors
		float areaHighPercentile = percentile(
			vmap<RectangleDetected, float>(candidateUnderOverLines, [](RectangleDetected r) -> float { return r.area; }),
			85
		);
		// Calculate slope of survivors
		float medianAngle = median(vmap<RectangleDetected, float>(candidateUnderOverLines,
			[](RectangleDetected r) -> float { return normalizeAngle(r.angle); }));

		candidateUnderOverLines = removeOverlappingRectangles(candidateUnderOverLines, [areaHighPercentile, medianAngle](RectangleDetected r) -> float {
			float deviationFromSideRatio = (r.shorterSideLength / r.longerSideLength) / overunderline_rect_short_to_long_ratio;
			if (deviationFromSideRatio < 1 && deviationFromSideRatio > 0) {
				deviationFromSideRatio = 1 / deviationFromSideRatio;
			}
			deviationFromSideRatio -= 1;
			float devationFromSideLengthRatioPenalty = 2.0f * deviationFromSideRatio;
			float deviationFromTargetArea = r.area < areaHighPercentile ?
				// Deviation penalty for falling short of target
				((areaHighPercentile / r.area) - 1) :
				// The consequences of capturing extra area are smaller,
				// so cut the penalty in half for those.
				(((r.area / areaHighPercentile) - 1) / 2);
			// The penalty from deviating from the target angle
			float angleDiff = normalizeAngle(r.angle) - medianAngle;
			angleDiff = MIN(angleDiff, 90.0f - angleDiff);
			float deviationFromTargetAngle = 2.0f * angleDiff;

			return devationFromSideLengthRatioPenalty + deviationFromTargetArea + deviationFromTargetAngle;
		});
	}

	return candidateUnderOverLines;
}


struct undoverline {
	public:
		char letter;
		char digit;
		cv::Point2f center;
		float slope;
		float length;
};

uchar reverseCharBits(uchar b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}


static void readUndoverline(cv::Mat image, RectangleDetected line)
{
	cv::Point2f start, end;
	const float lineHeight = MAX(line.bottomLeft.y, line.bottomRight.y) - MIN(line.topLeft.y, line.topRight.y);
	const float lineWidth = MAX(line.topRight.x, line.bottomRight.x) - MIN(line.topLeft.x, line.topRight.x);
	bool vertical = lineHeight > lineWidth;
	if ( vertical ) {
		// the line is closer to vertical than horizontal
		// start from half way between top left and top right and proceed to half way from bottom left and bottom right
		start = cv::Point2f(
			float(line.topLeft.x + line.topRight.x) / 2.0f,
			float(line.topLeft.y + line.topRight.y) / 2.0f
		);
		end = cv::Point2f(
			float(line.bottomLeft.x + line.bottomRight.x) / 2.0f,
			float(line.bottomLeft.y + line.bottomRight.y) / 2.0f
		);
	} else {
		// the line is closer to horizontal than vertical.
		// start from half way between top left and bottom left and proceed from half way between top right and bottom right
		start = cv::Point2f(
			float(line.topLeft.x + line.bottomLeft.x) / 2.0f,
			float(line.topLeft.y + line.bottomLeft.y) / 2.0f
		);
		end = cv::Point2f(
			float(line.topRight.x + line.bottomRight.x) / 2.0f,
			float(line.topRight.y + line.bottomRight.y) / 2.0f
		);
	}
	const float length_x = end.x - start.x;
	const float length_y = end.y - start.y;
	const float length = sqrt(length_x * length_x + length_y * length_y);
	const float width = length / 6.0f;
	// 6mm, line, eges of 0.25, leaving 5.5mm for dots 11 dots, or 0.5mm per dot
	// the center of the first one starts 0.25 + 0.5/2 = 0.5mm from edge
	const size_t numDots = 11;
	cv::Point2f dotCenters[numDots];
	std::vector<uchar> dotValues = std::vector<uchar>(11);
	for (size_t i=0; i < numDots; i++) {
		const float dotFraction = (0.5f + (i * 0.5f)) / 6.0f;
		const cv::Point2f center = cv::Point2f(start.x + (dotFraction * length_x), start.y + (dotFraction * length_y));
		dotCenters[i] = center;
		const int x = int(round(center.x));
		const int y = int(round(center.y));
		const std::vector<uchar> valuesAroundCenter = {
			image.at<uchar>(cv::Point(x, y)),
			image.at<uchar>(cv::Point(x + 1, y)),
			image.at<uchar>(cv::Point(x - 1, y)),
			image.at<uchar>(cv::Point(x, y + 1)),
			image.at<uchar>(cv::Point(x, y - 1))
		};
		const uchar medianValue	= median(valuesAroundCenter);
		dotValues[i] = medianValue;
	}

	// Get value at each center
	uchar threshold = bimodalThreshold(dotValues);

	// The binary coding has 11 bits,
	// from most significant (10) to least (0)
	//   Bit 10:   always 1
	//   Bit  9:   1 if overline, 0 if underline
	//   Bits 8-4: letter code (1-25, never 0, or 26-31)
	//   Bits 3-1: digit code  (1-6, never 0, 7)
	//   Bit  0:   always 0 
	uint binaryCodingReadForwardOrBackward = 0;
	for (size_t i = 0; i < numDots; i++) {
		// 1s are white, or lower values)
		binaryCodingReadForwardOrBackward <<= 1;
		if (dotValues[i] > threshold) {
			binaryCodingReadForwardOrBackward += 1;
		}
	}

	// First, we need to determine if we read the code in the
	// forward order (the first bit is 1) or the reverse order
	// (such that the last bit is 1).
	const bool firstBitRead = ( binaryCodingReadForwardOrBackward >> (numDots - 1) ) == 1;
	const bool lastBitRead = binaryCodingReadForwardOrBackward & 1;
	if ( (firstBitRead ^ lastBitRead) != 1 ) {
		// The direction bit is a white box at the left (starting) edge
		// of the underline.
		// Thus, either the first or last bit should be set to 1,
		// but never neither or both.
		// FIXME
		return;
	}

	const bool readInReverseOrder = lastBitRead;
	// Get the bits in the correct order, with the lowest-order
	// bit 0 (the always-black last bit in the underline)
	const uint binaryEncoding = readInReverseOrder ?
		reverseBits(binaryCodingReadForwardOrBackward, 11) : binaryCodingReadForwardOrBackward;
	const bool isOverline = (binaryEncoding >> 9) & 1;
	const uchar letterCode = ((binaryEncoding >> 4) & 0x1F);
	const uchar digitCode = ((binaryEncoding >> 1) & 0x7);
	if ( (letterCode < 1) || (letterCode > DieLetters.length() ) )  {
		// FIXME
		return;
	} else if ( (digitCode < 1) || (digitCode > DieDigits.length() ) ) {
		// FIXME
		return;
	}

	char letter = DieLetters[letterCode - 1];
	char digit = DieDigits[digitCode - 1];
	uchar rotationStep = vertical ? (
			// vertical
			readInReverseOrder ? 3 : 1
		) : (
			// horizontal
			readInReverseOrder ? 2 : 0
	);
	uint rotation = rotationStep * 90;

	std::string result = std::string("") + letter + digit + char('0' + rotationStep);
	
	// line.angle;
}