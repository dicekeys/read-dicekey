#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#pragma warning(disable : 4996)

#include <tesseract/baseapi.h>
#include <tesseract/genericvector.h>
#include "die-specification.h"
#include "geometry.h"

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
		// cv::imwrite("ocr-input.png", edges);

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
				// cv::imwrite(std::string(isDigit ? "digit" : "letter") + "-edges.png", edges);
			}
		}
	}
	return result;
}

struct DieCharactersRead {
	const ReadCharacterResult letter;
	const ReadCharacterResult digit;
};

static DieCharactersRead readDieCharacters(
	cv::Mat imageColor,
	cv::Mat grayscaleImage,
	cv::Point2f dieCenter,
	float angleRadians,
	float mmToPixels,
	char writeErrorUnlessThisLetterIsRead = 0,
	char writeErrorUnlessThisSigitIsRead = 0
) {
	// Rotate to remove the angle of the die
	const float degreesToRotateToRemoveAngleOfDie = radiansToDegrees(angleRadians);
	int textHeightPixels = int(ceil(DieDimensionsMm::textRegionHeight * mmToPixels));
	int textWidthPixels = int(ceil(DieDimensionsMm::textRegionWidth * mmToPixels));
	// Use an even text region width so we can even split it in two at the center;
	if ((textWidthPixels % 2) == 1) {
		textWidthPixels += 1;
	}
	cv::Size textRegionSize = cv::Size(textWidthPixels, textHeightPixels);

	const auto textImage = copyRotatedRectangle(grayscaleImage, dieCenter, degreesToRotateToRemoveAngleOfDie, textRegionSize);
	// Setup a rectangle to define your region of interest
	const cv::Rect letterRect(0, 0, textRegionSize.width / 2, textRegionSize.height);
	const cv::Rect digitRect( textRegionSize.width / 2, 0, textRegionSize.width / 2, textRegionSize.height);
	auto letterImage = textImage(letterRect);
	auto digitImage = textImage(digitRect);

	// FIXME -- remove after development debugging
	cv::imwrite("text-region.png", textImage);
	cv::imwrite("letter.png", letterImage);
	cv::imwrite("digit.png", digitImage);

	const ReadCharacterResult letter = readCharacter(letterImage, false);
	const ReadCharacterResult digit = readCharacter(digitImage, true);

	// FIXME -- remove after development debugging
	static int error = 1;
	if (writeErrorUnlessThisLetterIsRead != 0 && writeErrorUnlessThisLetterIsRead != letter.charRead) {
		std::string errBase = "error-" + std::to_string(error++) + "-read-" + std::string(1, writeErrorUnlessThisLetterIsRead) + "-as-" + std::string(1, letter.charRead);
		cv::imwrite(errBase + ".png", letterImage);
	}
	if (writeErrorUnlessThisSigitIsRead != 0 && writeErrorUnlessThisSigitIsRead != digit.charRead) {
		cv::imwrite("error-" + std::to_string(error++) + "-read-" + std::string(1, writeErrorUnlessThisSigitIsRead) + "-as-" + std::string(1, digit.charRead) + ".png", digitImage);
	}

	return {letter, digit};
}
