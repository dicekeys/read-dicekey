#include <float.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include <iostream>
// #include <filesystem>
#include "vfunctional.h"
#include "rectangle.h"
#include "rotate.h"
//#include "find-dice.h"
//#include "read-dice.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include "ocr.h"
#include "read-dice.h"
#include "validate-dice.h"

static void help(const char* programName)
{
	std::cout <<
		"Using OpenCV version " << CV_VERSION << "\n" << std::endl;
}


// const char* wndname = "Dice Reading";
// the function draws all the squares in the image
//static void drawSquares(cv::Mat & image, const std::vector<std::vector<cv::Point> > & squares)
//{
//	auto clone = image.clone();
//	for (size_t i = 0; i < squares.size(); i++)
//	{
//		const cv::Point* p = &squares[i][0];
//		int n = (int)squares[i].size();
//		polylines(clone, &p, &n, 1, true, cv::Scalar(0, 255, 0), 3, cv::LINE_AA);
//	}
//
//	imshow(wndname, clone);
//}

int main(int argc, char** argv)
{
	std::string tesseractPath = "/usr/local/Cellar/tesseract/4.0.0_1/share/tessdata/";
	// std::string tesseractPath = "C:\\Users\\stuar\\github\\dice-scanner\\dicescanner";
	initOcr(tesseractPath);
	//std::string path = "img/";
	// std::string intermediateImagePath = path + "progress/";
	help(argv[0]);

	for (int argi = 1; argi < argc; argi++) {
		const std::string filepath = argv[argi];

		cv::Mat image = cv::imread(filepath, cv::IMREAD_COLOR);
		if (image.empty()) {
			std::cout << "Couldn't load " << filepath << std::endl;
			return -1;
		}

		const size_t indexOfLastSlash = filepath.find_last_of("/") + 1;
		const std::string filename = filepath.substr(indexOfLastSlash);
		try {
			const auto dice = readDice(image);
			validateDiceRead(dice, filename.substr(0, 75));
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
