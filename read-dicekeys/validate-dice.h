//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#pragma once

#include <string>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include "vfunctional.h"
#include "dicekey.h"


/*
Compare the dice read during a test to a 75 character specifiation string that contains
three-character triples of letter, digit ('0'-'6'), and orientation (as number of turns from upright,
'0'-'3').
*/
static void validateDiceRead(const std::vector<DieRead> diceRead, std::string diceAsString)
{
	const DiceKey diceKeyNonCanonical = diceReadToDiceKey(diceRead, true);
	const DiceKey diceKey = diceKeyNonCanonical.rotateToCanonicalOrientation();
	for (size_t dieIndex = 0; dieIndex < diceRead.size(); dieIndex++) {
		const auto dieFace = diceKey.faces[dieIndex];
		const std::string dieAsString = diceAsString.substr(dieIndex * 3, 3);
		const auto letter = dieAsString[0];
		const char digit = dieAsString[1];
		const char orientationChar = dieAsString[2];
		const int orientationAs0to3ClockwiseTurnsFromUpright = orientationChar - '0';
		if (dieFace.letter == '\0' && dieFace.digit == '\0') {
			throw std::string("Die ") + std::to_string(dieIndex) + " letter and digit could not be read";
		} else if (dieFace.letter == '\0') {
			throw std::string("Die ") + std::to_string(dieIndex) + " letter could not be read";
		} else if (dieFace.digit == '\0') {
			throw std::string("Die ") + std::to_string(dieIndex) + " digit could not be read";
		} else if (dieFace.orientationAs0to3ClockwiseTurnsFromUpright != orientationAs0to3ClockwiseTurnsFromUpright) {
			throw std::string("Die ") + std::to_string(dieIndex) + " (" + dashIfNull(letter) + dashIfNull(digit) +
  			") has orientation " + std::to_string(dieFace.orientationAs0to3ClockwiseTurnsFromUpright) +
				" but should be " + std::to_string(orientationAs0to3ClockwiseTurnsFromUpright);
		} else if (dieFace.letter != letter) {
			throw std::string("Die ") + std::to_string(dieIndex) + " has letter " + dashIfNull(dieFace.letter) +
				" but should be " + dashIfNull(letter);
		} else if (dieFace.digit != digit) {
			throw std::string("Die ") + std::to_string(dieIndex) + " has digit " + dashIfNull(dieFace.digit) +
				" but should be " + dashIfNull(digit);
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
