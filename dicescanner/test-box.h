#include <string>
#include <iostream>
#include <filesystem>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>

#include "vfunctional.h"
#include "get-dice-from-image.h"
#include "dice.h"

namespace fs = std::experimental::filesystem;


template <typename T>
static T majorityOfThree(T a, T b, T c)
{
	if (a == b || a == c) {
		return a;
	} else if (b == c) {
		return b;
	}
	return 0;
}

static std::vector<DieFace> diceReadToDiceKey(const std::vector<DieRead> diceRead)
{
	return vmap<DieRead, DieFace>(diceRead,  [](DieRead dieRead) {
		if (dieRead.underline.dieFaceInferred.letter == 0 ||
				dieRead.underline.dieFaceInferred.digit == 0 ||
				dieRead.underline.dieFaceInferred.letter != dieRead.overline.dieFaceInferred.letter ||
				dieRead.underline.dieFaceInferred.digit != dieRead.overline.dieFaceInferred.digit) {
			// report error mismatch between undoverline and overline			
		} else if (dieRead.underline.dieFaceInferred.letter != dieRead.ocrLetter.charRead) {
			// report OCR error on letter
		} else if (dieRead.underline.dieFaceInferred.digit != dieRead.ocrDigit.charRead) {
			// report OCR error on digit
		}

		return DieFace({
			majorityOfThree(
				dieRead.underline.dieFaceInferred.letter,
				dieRead.overline.dieFaceInferred.letter,
				dieRead.ocrLetter.charRead
			),
			majorityOfThree(
				dieRead.underline.dieFaceInferred.digit,
				dieRead.overline.dieFaceInferred.digit,
				dieRead.ocrDigit.charRead
			),
			dieRead.orientationAs0to3ClockwiseTurnsFromUpright
		});
	});
}


bool validateDiceRead(const std::vector<DieRead> diceRead, std::string dieAsString)
{
	if (diceRead.size() != 25) {
		return false;
	}
	const auto diceKey = diceReadToDiceKey(diceRead);
	bool isValid = true;
	for (size_t dieIndex = 0; dieIndex < diceRead.size(); dieIndex++) {
		const auto die = diceKey[dieIndex];
		const std::string dieAsString = dieAsString.substr(dieIndex * 3, 3);
		const auto letter = dieAsString[0];
		const char digit = dieAsString[1];
		const char orientationChar = dieAsString[2];
		const int orientationAs0to3ClockwiseTurnsFromUpright = orientationChar - '0';
		if (die.orientationAs0to3ClockwiseTurnsFromUpright != orientationAs0to3ClockwiseTurnsFromUpright) {
			// FIXME -- output error.
			isValid = false;
		} else if (die.letter != letter) {
			isValid = false;
		} else if (die.digit != digit) {
			isValid = false;
		}
	}
	return isValid;
}
//
//void runTests(std::string inputPath)
//{
//    for (const auto &entry : fs::recursive_directory_iterator(inputPath)) {
//			auto path = fs::path(entry.path);
//			auto fileNameStem = path.stem().generic_string();
//			auto extension = path.extension().generic_string();
//			std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
//
//			if (extension != "png" && extension != "jpg" && extension != "jpeg") {
//				// no image file to read
//				continue;
//			}
//
//			std::cout << entry.path() << std::endl;
//
//			cv::Mat image = cv::imread(path.generic_string(), cv::IMREAD_COLOR);
//			if (image.empty())
//			{
//				std::cout << "Couldn't load " << path.generic_string() << std::endl;
//				continue;
//			}
//
//			auto prefixLength = fileNameStem.find("-");
//			auto prefix = prefixLength > 0 ? fileNameStem.substr(0, prefixLength) : fileNameStem;
//			auto prefixLength = prefix.size();
//
//			switch (prefixLength) {
//				case 3:
//					break;
//				case 75:
//					// Validate an entire dice box
//					testBox(prefix, fileNameStem, image);
//					break;
//			}
//
//		}
//}
//
//
//void testBox(std::string diceAsString, std::string fileNameStem, cv::Mat image) {
//	for (int dieIndex = 0; dieIndex < 25; dieIndex++) {
//		const auto dieAsString = diceAsString.substr(3 * dieIndex, 3);
//		const auto diceRead = getDiceFromImage(image, fileNameStem, fileNameStem, 1);
//		validateDieRead(diceRead[dieIndex], dieAsString);
//	}
//
//}