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

static char dashIfNull(char l) {return l == 0 ? '-' : l;}

static std::vector<DieFace> diceReadToDiceKey(const std::vector<DieRead> diceRead, bool reportErrsToStdErr = false)
{
	if (diceRead.size() != 25) {
		throw std::string("A DiceKey must contain 25 dice but only has " + std::to_string(diceRead.size()));
	}
	std::vector<DieFace> diceKey;
	for (size_t i=0; i < diceRead.size(); i++) {
		DieRead dieRead = diceRead[i];
		const DieFaceSpecification &underlineInferred = dieRead.underline.dieFaceInferred;
		const DieFaceSpecification &overlineInferred = dieRead.overline.dieFaceInferred;
		const char digitRead = dieRead.ocrDigit.charRead;
		const char letterRead = dieRead.ocrLetter.charRead;
		if (underlineInferred.letter == 0 ||
				underlineInferred.digit == 0 ||
				underlineInferred.letter != overlineInferred.letter ||
				underlineInferred.digit != overlineInferred.digit) {
			// report error mismatch between undoverline and overline
			if (reportErrsToStdErr) {
				std::cerr << "Mismatch between underline and overline: " <<
					dashIfNull(underlineInferred.letter) << dashIfNull(underlineInferred.digit) << " != " <<
					dashIfNull(overlineInferred.letter) << dashIfNull(overlineInferred.digit);
			}
		} else if (underlineInferred.letter != letterRead) {
			// report OCR error on letter
			if (reportErrsToStdErr) {
				std::cerr << "Mismatch between underline and ocr letter: " <<
					dashIfNull(underlineInferred.letter) << " != " << dashIfNull(letterRead);
			}
		} else if (underlineInferred.digit != digitRead) {
			// report OCR error on digit
			if (reportErrsToStdErr) {
				std::cerr << "Mismatch between underline and ocr digit: " <<
					dashIfNull(underlineInferred.digit) << " != " << dashIfNull(digitRead);
			}
		}

		diceKey.push_back(DieFace({
			majorityOfThree(
				underlineInferred.letter, overlineInferred.letter, dieRead.ocrLetter.charRead
			),
			majorityOfThree(
				underlineInferred.digit, overlineInferred.digit, dieRead.ocrDigit.charRead
			),
			dieRead.orientationAs0to3ClockwiseTurnsFromUpright
		}));
	}
	return diceKey;
}


static void validateDiceRead(const std::vector<DieRead> diceRead, std::string diceAsString)
{
	const auto diceKeyNonCanonical = diceReadToDiceKey(diceRead, true);
	const auto diceKey = rotateToCanonicalDiceKey(diceKeyNonCanonical);
	for (size_t dieIndex = 0; dieIndex < diceRead.size(); dieIndex++) {
		const auto die = diceKey[dieIndex];
		const std::string dieAsString = diceAsString.substr(dieIndex * 3, 3);
		const auto letter = dieAsString[0];
		const char digit = dieAsString[1];
		const char orientationChar = dieAsString[2];
		const int orientationAs0to3ClockwiseTurnsFromUpright = orientationChar - '0';
		if (die.orientationAs0to3ClockwiseTurnsFromUpright != orientationAs0to3ClockwiseTurnsFromUpright) {
			throw std::string("Die ") + std::to_string(dieIndex) + " has orientation " + std::to_string(die.orientationAs0to3ClockwiseTurnsFromUpright) +
				" but should be" + std::to_string(orientationAs0to3ClockwiseTurnsFromUpright);
		} else if (die.letter != letter) {
			throw std::string("Die ") + std::to_string(dieIndex) + " has letter " + dashIfNull(die.letter) +
				" but should be" + dashIfNull(letter);
		} else if (die.digit != digit) {
			throw std::string("Die ") + std::to_string(dieIndex) + " has digit " + dashIfNull(die.digit) +
				" but should be" + dashIfNull(digit);
		}
	}
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