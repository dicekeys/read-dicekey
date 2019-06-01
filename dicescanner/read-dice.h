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

class DieRead {
	public:
	uint orientationInDegrees = 0;
	char letter = 0;
	char digit = 0;
	float letterConfidence = 0;
	float digitConfidence = 0;
};

static cv::Rect cropLTWH(const cv::Size size, float _left, float _top, float _width, float _height) {
	int top = std::max( 0, (int) std::round(_top));
	int left = std::max( 0, (int) std::round(_left));
	int width = std::min( (int) std::round(_width), size.width - left );
	int height = std::min( (int) std::round(_height), size.height - top );

	return cv::Rect(left, top, width, height);
}

// static cv::Rect cropLTRB(const cv::Size size, float _left, float _top, float _right, float _bottom) {
// 	return cropLTWH(size, _left, _top, _right - _left, _bottom - _top);

// }

tesseract::TessBaseAPI* initOcr(std::string tesseractPath = "/dev/null") {
	static bool tess_initialized = false;
	static tesseract::TessBaseAPI* ocr = new tesseract::TessBaseAPI();
	if (!tess_initialized) {
		const char* alphabet = "ABCDEGHJKLMNPQRTVWXYabdfr123456";
		if (tesseractPath == "/dev/null") {
			throw "initOcr called before tesseract path provided";
		}
		auto varNames = GenericVector<STRING>();
		varNames.push_back("load_system_dawg");
		varNames.push_back("load_freq_dawg");
		varNames.push_back("tessedit_char_whitelist");
		auto varValues = GenericVector<STRING>();
		varValues.push_back("0");
		varValues.push_back("0");
		varValues.push_back(alphabet);
	
		// Initialize tesseract to use English (eng) and the LSTM OCR engine.
		ocr->Init(tesseractPath.c_str(), "eng", tesseract::OEM_TESSERACT_ONLY, NULL, 0, &varNames, &varValues, false);
		tess_initialized = true;
		ocr->SetVariable("tessedit_char_whitelist", alphabet);
		ocr->SetPageSegMode(tesseract::PSM_RAW_LINE);
	}
	return ocr;
}

bool readDie(cv::Mat &dieImageGrayscale, DieRead &dieRead, int threshold, std::string debugImagePath = "/dev/null", int debugLevel = 0) {
	const uint N = 20;
	static bool tess_initialized = false;
	static tesseract::TessBaseAPI* ocr = initOcr();
	
	dieRead.letterConfidence = 0;
	dieRead.digitConfidence = 0;

	cv::Mat dieBlur, edges;
	cv::medianBlur(dieImageGrayscale, dieBlur, 3);

	for (int l = 0; l < N; l++)
	{
		// hack: use Canny instead of zero threshold level.
		// Canny helps to catch squares with gradient shading
		if (l == 0) {
			cv::threshold(dieBlur, edges, 0, 255, cv::THRESH_BINARY + cv::THRESH_OTSU);
		} else {
			edges = dieBlur >= (l + 1) * 255 / N;;
		}

		if (!debugImagePath.find("/dev/null", 0) == 0 && debugLevel >= 2) {
		 	cv::imwrite(debugImagePath + "ocr-edges-" + std::to_string(l) + ".png", edges);
		}

		const tesseract::PageIteratorLevel level = tesseract::RIL_SYMBOL;

		auto bytesPerPixel = edges.elemSize();
		auto bytesPerLine = edges.step1();
		auto width = edges.size().width;
		auto height = edges.size().height;
		ocr->SetImage(edges.data, width, height, (int) bytesPerPixel, (int) bytesPerLine);
		ocr->Recognize(NULL);
		
		auto iterator = ocr->GetIterator();

		if (iterator == NULL) {
			continue;
		}
		char* symbol = iterator->GetUTF8Text(level);
		if (symbol == NULL || *symbol == 0) {
			delete iterator;
			continue;
		}
		float letterConfidence = iterator->Confidence(level);
		char letter = *symbol;
		delete symbol;
		static const std::string validLetters = "aAbBCdDEGHJKLMNPQrRTVWXY";
		if (letterConfidence > dieRead.letterConfidence && validLetters.find(letter) != std::string::npos ) {
			dieRead.letter = letter;
			dieRead.letterConfidence = iterator->Confidence(level);
		}

		if (!iterator->Next(level)) {
			delete iterator;
			continue;
		}

		symbol = iterator->GetUTF8Text(level);
		if (symbol == NULL || *symbol == 0) {
			delete iterator;
			continue;
		}
		float digitConfidence = iterator->Confidence(level);
		char digit = *symbol;
		delete symbol;
		if (digitConfidence > dieRead.digitConfidence && digit >= '0' && digit <= '6') {
			dieRead.digit = digit;
			dieRead.digitConfidence = iterator->Confidence(level);
		}

		delete iterator;
	}
	return dieRead.digitConfidence > 0 && dieRead.letterConfidence > 0;
}


static bool orientAndReadDie(std::string debugImagePath, cv::Mat &dieImageGrayscale, DieRead &dieRead, float approxPixelsPerMm, int debugLevel = 0) {
	float centerX = ((float) dieImageGrayscale.size[0]) / 2;
	float centerY = ((float) dieImageGrayscale.size[1]) / 2;

	// const float approxPixelsPerMm =
	// 	( (dieImageGrayscale.size[0] + dieImageGrayscale.size[1]) / 2 ) / // pixels for size of die
	// 	8; // 8mm dice

	bool underlineFound;
	RectangleDetected underline;
	underlineFound = findUnderline(dieImageGrayscale, underline, approxPixelsPerMm);
	if (underlineFound) {
		cv::Mat textRegionImageGrayscale;
		if (debugImagePath.size() > 0) {
			auto underlineImage = dieImageGrayscale.clone();
			std::vector<cv::Point> points = vmap<cv::Point2f, cv::Point>(underline.points, [](cv::Point2f point) -> cv::Point {
				return cv::Point(point);
			});
			const cv::Point* p = points.data();
			int n = (int)points.size();
			polylines(underlineImage, &p, &n, 1, true, cv::Scalar(0, 255, 0), 3, cv::LINE_AA);
			if (!debugImagePath.find("/dev/null", 0) == 0 && debugLevel >= 1) {
				cv::imwrite(debugImagePath + "underline.png", underlineImage);
			}
		}

		const float distx = abs(underline.center.x - centerX);
		const float disty = abs(underline.center.y - centerY);
		if (disty > distx) {
			// The underline is above or below the center,
			// indicating horizontal lettering (rotated 0 or 180)
			dieRead.orientationInDegrees = (underline.center.y < centerY) ? 180 : 0;
			// std::cout << "Die " << dieIndex << " at angle " << dieRead.orientationInDegrees << "\n";
			float width = std::max(underline.longerSideLength, 5.5f * approxPixelsPerMm);
			float left = underline.center.x - width / 2.0f;
			float height = 2.0f * (disty - 0.2f);
			float top =   dieRead.orientationInDegrees == 180 ?
					// Select safely below the underline
					underline.center.y + 0.3f * approxPixelsPerMm :
					// Select safely above the underline
					underline.center.y - (0.3f * approxPixelsPerMm) - height;
			auto cropRect = cropLTWH(dieImageGrayscale.size(), left, top, width, height);
			cv::Mat dieImage = dieImageGrayscale(cropRect);
			if (dieRead.orientationInDegrees == 180) {
				// The underline is above the die center.  It's up-side down.
				// Rotate 180
				cv::rotate(dieImage, textRegionImageGrayscale, cv::ROTATE_180);
			} else {
				textRegionImageGrayscale = dieImage;
			}
		} else {
			dieRead.orientationInDegrees = underline.center.x < centerX ? 90 : 270;
			// std::cout << "Die " << dieIndex << " at angle " << dieRead.orientationInDegrees << "\n";
			float height = std::max(underline.longerSideLength, 5.5f * approxPixelsPerMm);
			float top = underline.center.y - height / 2.0f;
			float width = 2.0f * (distx - 0.2f);
			float left = dieRead.orientationInDegrees == 90 ?
				// Select safely to right of funderline
				underline.center.x + 0.3f * approxPixelsPerMm :
				// Select safely to left of underline and the width of the area to be read
				underline.center.x - (0.3f * approxPixelsPerMm) - width
			;
			float right = left + width;
			auto cropRect = cropLTWH(dieImageGrayscale.size(), left, top, width, height);
			cv::Mat dieImage = dieImageGrayscale(cropRect);
			cv::rotate(dieImage, textRegionImageGrayscale, dieRead.orientationInDegrees == 90 ? cv::ROTATE_90_COUNTERCLOCKWISE : cv::ROTATE_90_CLOCKWISE);
		}

		readDie(textRegionImageGrayscale, dieRead, underline.foundAtThreshold, debugImagePath, debugLevel);
	}

	return underlineFound && dieRead.letterConfidence > 0 && dieRead.digitConfidence > 0;
}



// static std::vector<DieRead> orientAndReadDice(std::vector<cv::Mat> &dieGrayscaleImages, float approxPixelsPerMm, std::string debugImagePath)
// {
// 	std::vector<DieRead> result;
// 	for (uint i = 0; i < dieGrayscaleImages.size(); i++) {
// 		DieRead dieRead;
// 		orientAndReadDie(debugImagePath + + "-" + std::to_string(i) + "-", dieGrayscaleImages[i], dieRead, approxPixelsPerMm, dieDebugLevel);
// 		result.push_back(dieRead);
// 	}
// 	return result;
// }