#include <string>
#include <iostream>
#include <filesystem>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>

#include "get-dice-from-image.h"

namespace fs = std::experimental::filesystem;

bool validateDieRead(const DieRead& dieRead, std::string dieAsString)
{
	const auto letter = dieAsString[0];
	const char digit = dieAsString[1];
	const char orientationChar = dieAsString[2];
	const int orientationInDegrees = 90 * (int)(orientationChar - '0');
	if (dieRead.orientationInDegrees != orientationInDegrees)
		return false;
	if (dieRead.letter != letter) {
		return false;
	}
	if (dieRead.digit != digit) {
		return false;
	}
	return true;
}

void runTests(std::string inputPath)
{
    for (const auto &entry : fs::recursive_directory_iterator(inputPath)) {
			auto path = fs::path(entry.path);
			auto fileNameStem = path.stem().generic_string();
			auto extension = path.extension().generic_string();
			std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

			if (extension != "png" && extension != "jpg" && extension != "jpeg") {
				// no image file to read
				continue;
			}

			std::cout << entry.path() << std::endl;

			cv::Mat image = cv::imread(path.generic_string(), cv::IMREAD_COLOR);
			if (image.empty())
			{
				std::cout << "Couldn't load " << path.generic_string() << std::endl;
				continue;
			}

			auto prefixLength = fileNameStem.find("-");
			auto prefix = prefixLength > 0 ? fileNameStem.substr(0, prefixLength) : fileNameStem;
			auto prefixLength = prefix.size();

			switch (prefixLength) {
				case 3:
					break;
				case 75:
					// Validate an entire dice box
					testBox(prefix, fileNameStem, image);
					break;
			}

		}
}


void testBox(std::string diceAsString, std::string fileNameStem, cv::Mat image) {
	for (int dieIndex = 0; dieIndex < 25; dieIndex++) {
		const auto dieAsString = diceAsString.substr(3 * dieIndex, 3);
		const auto diceRead = getDiceFromImage(image, fileNameStem, fileNameStem, 1);
		validateDieRead(diceRead[dieIndex], dieAsString);
	}

}