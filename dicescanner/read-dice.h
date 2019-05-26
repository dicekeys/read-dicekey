#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#pragma warning(disable : 4996)

#include <tesseract/baseapi.h>
#include <tesseract/genericvector.h>

#include "find-underline.h"

struct DieRead {	
	uint orientationInDegrees;
	char letter;
	char digit;
	float letterConfidence;
	float digitConfidence;
};

bool readDie(std::string path, cv::Mat &dieImageGrayscale, DieRead &dieRead, int threshold) {
	dieRead.letterConfidence = 0;
	dieRead.digitConfidence = 0;

	cv::Mat dieBlur, edges;
	blur(dieImageGrayscale, dieBlur, cv::Size(3,3));
	// Canny(dieBlur, edges, 10, 50, 5);
	if (threshold == 0) {
		edges = dieBlur >= 65;
	} else {
		edges = dieBlur >= threshold;
	}


	static tesseract::TessBaseAPI ocr = tesseract::TessBaseAPI();
	static bool tess_initialized = false;
	const tesseract::PageIteratorLevel level = tesseract::RIL_SYMBOL;

	if (!tess_initialized) {
		auto varNames = GenericVector<STRING>();
		varNames.push_back("load_system_dawg");
		varNames.push_back("load_freq_dawg");
		varNames.push_back("tessedit_char_whitelist");
		auto varValues = GenericVector<STRING>();
		varValues.push_back("0");
		varValues.push_back("0");
		varValues.push_back("ABCDEGHJKLMNPQRTVWXYabdfr123456");
	

		// Initialize tesseract to use English (eng) and the LSTM OCR engine.
		ocr.Init(path.c_str(), "eng", tesseract::OEM_TESSERACT_ONLY, NULL, 0, &varNames, &varValues, false);
		ocr.SetVariable("tessedit_char_whitelist", "ABCDEGHJKLMNPQRTVWXYabdfr123456");
	
		ocr.SetPageSegMode(tesseract::PSM_RAW_LINE);
	}
	
	auto bytesPerPixel = edges.elemSize();
	auto bytesPerLine = edges.step1();
	ocr.SetImage(edges.data, edges.size().width, edges.size().height, (int) bytesPerPixel, (int) bytesPerLine);
	ocr.Recognize(NULL);
	
	auto iterator =ocr.GetIterator();

	if (iterator == NULL) {
		return false;
	}
	char* symbol = iterator->GetUTF8Text(level);
	if (symbol == NULL || *symbol == 0) {
		return false;
	}
	dieRead.letter = *symbol;
	delete symbol;
	dieRead.letterConfidence = iterator->Confidence(level);

	if (!iterator->Next(level)) {
		delete iterator;
		return false;
	}

	symbol = iterator->GetUTF8Text(level);
	if (symbol == NULL || *symbol == 0) {
		delete iterator;
		return false;
	}
	dieRead.digit =  *symbol;
	delete symbol;
	dieRead.digitConfidence = iterator->Confidence(level);

	delete iterator;
	return true;
}



static bool orientAndReadDie(std::string tesseractPath, std::string debugImagePath, cv::Mat &dieImageGrayscale, DieRead &dieRead, float approxPixelsPerMm, int dieIndex = -1) {
	cv::Mat textRegionImageGrayscale;
	float centerX = ((float) dieImageGrayscale.size[0]) / 2;
	float centerY = ((float) dieImageGrayscale.size[1]) / 2;

	// const float approxPixelsPerMm =
	// 	( (dieImageGrayscale.size[0] + dieImageGrayscale.size[1]) / 2 ) / // pixels for size of die
	// 	8; // 8mm dice

	RectangleDetected underline;
	bool underlineFound = findUnderline(dieImageGrayscale, underline);

	if (underlineFound) {
			
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

		char letter = 0, digit = 0;
		float letterConfidence = 0, digitConfidence = 0;

		readDie(tesseractPath, textRegionImageGrayscale, dieRead, underline.foundAtThreshold );
		if (letterConfidence > 0 && letterConfidence > dieRead.letterConfidence) {
			dieRead.letter = letter;
			dieRead.letterConfidence = letterConfidence;
		}
		if (digitConfidence > 0 && digitConfidence > dieRead.digitConfidence) {
			dieRead.digit = digit;
			dieRead.digitConfidence = digitConfidence;
		}

	}

	return underlineFound && dieRead.letterConfidence > 0 && dieRead.digitConfidence > 0;
}
