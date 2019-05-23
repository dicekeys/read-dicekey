#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include <tesseract/baseapi.h>



struct DieOcrResult {
	bool success;
	char letter;
	char digit;
	float letterConfidence;
	float digitConfidence;
	uint orientation;
};

DieOcrResult readDie(cv::Mat &image) {
	DieOcrResult result;
	result.success = false;

	static tesseract::TessBaseAPI ocr = tesseract::TessBaseAPI();
	static bool tess_initialized = false;
	
	// Initialize tesseract to use English (eng) and the LSTM OCR engine.
	if (!tess_initialized) {
		ocr.Init("/usr/local/Cellar/tesseract/4.0.0_1/share/tessdata/", "eng", tesseract::OEM_TESSERACT_ONLY);
		ocr.SetVariable("tessedit_char_whitelist", "ABCDEGHJKLMNPQRTVWXYabdfr123456");
		ocr.SetPageSegMode(tesseract::PSM_AUTO_OSD);
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
