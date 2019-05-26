#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#pragma warning(disable : 4996)

#include <tesseract/baseapi.h>
#include <tesseract/genericvector.h>

struct DieRead {	
	uint orientationInDegrees;
	char letter;
	char digit;
	float letterConfidence;
	float digitConfidence;
};

bool readDie(std::string path, cv::Mat &dieImageGrayscale, DieRead &dieRead) {
	dieRead.letterConfidence = 0;
	dieRead.digitConfidence = 0;

	cv::Mat dieBlur, edges;
	blur(dieImageGrayscale, dieBlur, cv::Size(3,3));
	// Canny(dieBlur, edges, 10, 50, 5);
	edges = dieBlur >= 65;


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
