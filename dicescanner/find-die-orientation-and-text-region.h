
#include <float.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include "vfunctional.h"
#include "rectangle.h"
#include "find-squares.h"
#include "value-clusters.h"
#include "rotate.h"
#include "distance.h"
#include "find-underline.h";
#include "read-dice.h";

#include <iostream> // FIXME after this works


// struct DieOrientationAndTextRegion {
// 	cv::Mat dieImageGrayscale;
// 	cv::Mat textRegionImageGrayscale;
// 	bool underlineFound;
// 	RectangleDetected underline;
// 	int orientationInDegrees;
// 	char letter;
// 	char digit;
// 	float letterConfidence;
// 	float digitConfidence;
// };

static bool findDieOrientationAndTextRegion(std::string tesseractPath, std::string debugImagePath, cv::Mat &dieImageGrayscale, DieRead &dieRead, float approxPixelsPerMm, int dieIndex = -1) {
	cv::Mat textRegionImageGrayscale;
	float centerX = dieImageGrayscale.size[0] / 2;
	float centerY = dieImageGrayscale.size[1] / 2;

	// const float approxPixelsPerMm =
	// 	( (dieImageGrayscale.size[0] + dieImageGrayscale.size[1]) / 2 ) / // pixels for size of die
	// 	8; // 8mm dice

	RectangleDetected underline;
	bool underlineFound = findUnderline(dieImageGrayscale, underline);
		
	const float distx = abs(underline.center.x - centerX);
	const float disty = abs(underline.center.y - centerY);
	if (disty > distx) {
		// The underline is above or below the center,
		// indicating horizontal lettering (rotated 0 or 180)
		dieRead.orientationInDegrees = (underline.center.y < centerY) ? 180 : 0;
		std::cout << "Die " << dieIndex << " at angle " << dieRead.orientationInDegrees << "\n";
		float left = underline.center.x - underline.longerSideLength / 2.0f;
		float height = 2.0f * (disty - 0.2f);
		float top = dieRead.orientationInDegrees == 180 ?
			// Select safely below the underline
			underline.center.y + 0.3f * approxPixelsPerMm :
			// Select safely above the underline
			underline.center.y - (0.3f * approxPixelsPerMm) - height;
		float width = std::max(underline.longerSideLength, 5.5f * approxPixelsPerMm);
		cv::Rect myROI((int) left, (int) top, (int) width, (int) height);
		cv::Mat dieImage = dieImageGrayscale(myROI);
		if (dieRead.orientationInDegrees == 180) {
			// The underline is above the die center.  It's up-side down.
			// Rotate 180
			cv::rotate(dieImage, dieImage, cv::ROTATE_180);
		}
		textRegionImageGrayscale = dieImage;
	} else {
		dieRead.orientationInDegrees = underline.center.x < centerX ? 270 : 90;
		std::cout << "Die " << dieIndex << " at angle " << dieRead.orientationInDegrees << "\n";
		float top = underline.center.y - underline.longerSideLength / 2.0f;
		float width = 2.0f * (distx - 0.2f);
		float left = dieRead.orientationInDegrees == 90 ?
			// Select safely to right of funderline
			underline.center.x + 0.3f * approxPixelsPerMm :
			// Select safely to left of underline and the width of the area to be read
			underline.center.x - (0.3f * approxPixelsPerMm) - width;
		float height = std::max(underline.longerSideLength, 5.5f * approxPixelsPerMm);
		cv::Rect myROI((int) left, (int) top, (int) width, (int) height);
		cv::Mat dieImage = dieImageGrayscale(myROI);
		cv::Mat rotated;
		cv::rotate(dieImage, rotated, dieRead.orientationInDegrees == 90 ? cv::ROTATE_90_COUNTERCLOCKWISE : cv::ROTATE_90_CLOCKWISE);
	}

	char letter, digit;
	float letterConfidence, digitConfidence;
	readDie(tesseractPath, textRegionImageGrayscale, dieRead);

	if (letterConfidence > 0 && letterConfidence > dieRead.letterConfidence) {
		dieRead.letter = letter;
		dieRead.letterConfidence = letterConfidence;
	}
	if (digitConfidence > 0 && digitConfidence > dieRead.digitConfidence) {
		dieRead.digit = digit;
		dieRead.digitConfidence = digitConfidence;
	}

	return underlineFound && dieRead.letterConfidence > 0 && dieRead.digitConfidence > 0;
}
