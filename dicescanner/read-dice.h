#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#pragma warning(disable : 4996)

#include <tesseract/baseapi.h>
#include <tesseract/genericvector.h>


struct DieOcrResult {
	bool success = false;
	char letter = 0;
	char digit = 0;
	float letterConfidence = 0;
	float digitConfidence = 0;
	uint orientation = 0;
};

DieOcrResult readDie(std::string path, cv::Mat &image) {
	DieOcrResult result;
	result.success = false;

	static tesseract::TessBaseAPI ocr = tesseract::TessBaseAPI();
	static bool tess_initialized = false;

	auto varsNames = GenericVector<STRING>();
	varsNames.push_back("load_system_dawg");
	varsNames.push_back("load_freq_dawg");
	varsNames.push_back("tessedit_char_whitelist");
	auto varValues = GenericVector<STRING>();
	varValues.push_back("0");
	varValues.push_back("0");
	varValues.push_back("ABCDEGHJKLMNPQRTVWXYabdfr123456");
	

	// Initialize tesseract to use English (eng) and the LSTM OCR engine.
	if (!tess_initialized) {
		// ocr.SetVariable("load_system_dawg", "false");
		// ocr.SetVariable("load_freq_dawg", "false");
		ocr.Init(path.c_str(), "eng", tesseract::OEM_TESSERACT_ONLY, NULL, 0, &varsNames, &varValues, false);
		ocr.SetVariable("tessedit_char_whitelist", "ABCDEGHJKLMNPQRTVWXYabdfr123456");
		
		ocr.SetPageSegMode(tesseract::PSM_RAW_LINE);
	}
	//
	// tess.SetImage((uchar*)sub.data, sub.size().width, sub.size().height, sub.channels(), sub.step1());

	
	auto width = image.size().width;
	auto height = image.size().height;
	auto bytesPerPixel = image.elemSize();
	auto bytesPerLine = image.step1();
	ocr.SetImage(image.data, width, height, (int) bytesPerPixel, (int) bytesPerLine);
	ocr.Recognize(NULL);
	
	

	// auto test = ocr.GetUTF8Text();

	/*
	tesseract::PageIterator* it =  ocr.AnalyseLayout();
	if (!it) {
		return result;
	}
	tesseract::Orientation orientation;
	tesseract::WritingDirection direction;
  	tesseract::TextlineOrder order;
	float deskew_angle;

	it->Orientation(&orientation, &direction, &order, &deskew_angle);
	delete it;
	 
	 */
	tesseract::PageIteratorLevel level = tesseract::RIL_SYMBOL;
	auto iterator =ocr.GetIterator();

	if (iterator == NULL) {
		return result;
	}
	char* symbol = iterator->GetUTF8Text(level);
	if (symbol == NULL || *symbol == 0) {
		return result;
	}
	result.letter = *symbol;
	delete symbol;
	result.letterConfidence = iterator->Confidence(level);

	if (!iterator->Next(level)) {
		delete iterator;
		return result;
	}

	symbol = iterator->GetUTF8Text(level);
	if (symbol == NULL || *symbol == 0) {
		delete iterator;
		return result;
	}
	result.digit =  *symbol;
	delete symbol;
	result.digitConfidence = iterator->Confidence(level);

	delete iterator;
	result.success = true;
	return result;
}
