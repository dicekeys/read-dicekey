#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#pragma warning(disable : 4996)

#include <tesseract/baseapi.h>
#include <tesseract/genericvector.h>
#include "die.h"

static tesseract::TessBaseAPI* initCharOcr(std::string alphabet, std::string tesseractPath = "/dev/null")
{
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
	varValues.push_back(alphabet.c_str());

	// Initialize tesseract to use English (eng) and the LSTM OCR engine.
	tesseract::TessBaseAPI* ocr = new tesseract::TessBaseAPI();
	ocr->Init(tesseractPath.c_str(), "eng", tesseract::OEM_TESSERACT_ONLY, NULL, 0, &varNames, &varValues, false);
	ocr->SetVariable("tessedit_char_whitelist", alphabet.c_str());
	ocr->SetPageSegMode(tesseract::PSM_SINGLE_CHAR);
	return ocr;
}

struct OcrApis {
	tesseract::TessBaseAPI *letters, *digits;
};

OcrApis initOcr(std::string tesseractPath = "/dev/null") {
	static bool tess_initialized = false;
	static OcrApis apis;
	if (!tess_initialized) {
		apis.letters = initCharOcr(DieLetters, tesseractPath);
		apis.digits = initCharOcr(DieDigits, tesseractPath);
		tess_initialized = true;
	}
	return apis;
}

struct ReadCharacterResult {
	char charRead;
	float confidence;
};

ReadCharacterResult readCharacter(cv::Mat& imageGrayscale, bool isDigit, std::string debugImagePath = "/dev/null", int debugLevel = 0) {
	ReadCharacterResult result;
	result.charRead = 0;
	result.confidence = 0;

	const uint N = 20;
	auto ocrApi = initOcr();
	tesseract::TessBaseAPI* ocr = isDigit ? ocrApi.digits : ocrApi.letters;

	cv::Mat dieBlur, edges;
	cv::medianBlur(imageGrayscale, dieBlur, 3);

	for (int l = 0; l < N; l++)
	{
		// hack: use Canny instead of zero threshold level.
		// Canny helps to catch squares with gradient shading
		if (l == 0) {
			cv::threshold(dieBlur, edges, 0, 255, cv::THRESH_BINARY + cv::THRESH_OTSU);
		}
		else {
			edges = dieBlur >= (l + 1) * 255 / N;;
		}

		if (!debugImagePath.find("/dev/null", 0) == 0 && debugLevel >= 2) {
			cv::imwrite(debugImagePath + "ocr-edges-" + std::to_string(l) + ".png", edges);
		}
		cv::imwrite("ocr-input.png", edges);

		const tesseract::PageIteratorLevel level = tesseract::RIL_SYMBOL;

		auto bytesPerPixel = edges.elemSize();
		auto bytesPerLine = edges.step1();
		auto width = edges.size().width;
		auto height = edges.size().height;
		ocr->SetImage(edges.data, width, height, (int)bytesPerPixel, (int)bytesPerLine);
		ocr->Recognize(NULL);

		float confidence = float(ocr->MeanTextConf());
		if (confidence > result.confidence) {
			char* symbol = ocr->GetBoxText(0);
			if (symbol && *symbol) {
				result.confidence = confidence;
				result.charRead = *symbol;
				delete symbol;
				cv::imwrite(std::string(isDigit ? "digit" : "letter") + "-edges.png", edges);
			}
		}
	}
		

	//	auto iterator = ocr->GetIterator();

	//	if (iterator == NULL) {
	//		continue;
	//	}
	//	float confidence = iterator->Confidence(tesseract::RIL_SYMBOL);
	//	if (confidence > result.confidence) {
	//		char* symbol = iterator->GetUTF8Text(tesseract::RIL_SYMBOL);
	//		if (symbol == NULL || *symbol == 0) {
	//			delete iterator;
	//			continue;
	//		}
	//		result.confidence = confidence;
	//		result.charRead = *symbol;
	//		delete symbol;
	//		cv::imwrite(std::string(isDigit ? "digit" : "letter") + "-edges.png", edges);
	//	}
	//	delete iterator;
	//}
	return result;
}
