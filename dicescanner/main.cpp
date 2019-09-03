#include <float.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include <iostream>
#ifdef _WIN32
#include <filesystem>
#endif
#include "vfunctional.h"
#include "rectangle.h"
#include "rotate.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include "read-dice.h"
#include "validate-dice.h"
#include "visualize-read-results.h"

static void help(const char* programName)
{
	std::cout <<
		"Using OpenCV version " << CV_VERSION << "\n" << std::endl;
}

int main(int argc, char** argv)
{
	// help(argv[0]);

#ifdef _WIN32
	for (const auto& entry : std::filesystem::directory_iterator("img/")) {
		if (!entry.is_regular_file()) {
	 		continue;
		}
		const auto filepath = entry.path().generic_string();
#else
	for (int argi = 1; argi < argc; argi++) {
		const std::string filepath = argv[argi];
#endif
		cv::Mat image = cv::imread(filepath, cv::IMREAD_COLOR);
		if (image.empty()) {
			std::cout << "Couldn't load " << filepath << std::endl;
			// return -1;
			continue;
		}
		
		const size_t indexOfLastSlash = filepath.find_last_of("/") + 1;
		const std::string filename = filepath.substr(indexOfLastSlash);

		//if (filename[0] != 'E') {
		//	continue;
		//}

		std::cerr << "Reading " << filename << "\n";

		try {
			const auto diceRead = readDice(image, true);
			const cv::Mat dieReadOutput = visualizeReadResults(image, diceRead, true);
			
			cv::imwrite("out/" + filename.substr(0, filename.length() - 4) + "-results.png", dieReadOutput);


			validateDiceRead(diceRead.dice, filename.substr(0, 75));
			std::cerr << "Validated " << filename << "\n";
		} catch (std::string errStr) {
			std::cerr << "Exception in " << filename << "\n  " << errStr << "\n";
		}
	}


// 	for (const auto& entry : std::filesystem::directory_iterator(path)) {
// 		if (!entry.is_regular_file()) {
// 			continue;
// 		}
// 		const auto filepath = entry.path().generic_string();
// 		const auto filename = entry.path().filename().generic_string();
// 		if (filename.length() < 5) {
// 			continue;
// 		}
// 		const auto extension = filename.substr(filename.length() - 4);
// 		if (extension != ".jpg" && extension != ".JPG") {
// 			continue;
// 		}
				
// //		std::string fname = path + "img/" + filename + ".jpg";
// 		cv::Mat image = cv::imread(filepath, cv::IMREAD_COLOR);
// 		if (image.empty()) {
// 			std::cout << "Couldn't load " << filename << std::endl;
// 			continue;
// 		}

// 		const auto dice = readDice(image);

// 		try {
// 			validateDiceRead(dice, filename.substr(0, 75));
// 			std::cerr << "Validated " << filename << "\n";
// 		} catch (std::string strErr) {
// 			std::cerr << "Exception: " << strErr << "\n";
// 		}
// 	}
	return 0;
}
