#include <float.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include <iostream>
#include "vfunctional.h"
#include "rectangle.h"
#include "find-squares.h"
#include "value-clusters.h"
#include "rotate.h"
#include "find-dice.h"
#include "read-dice.h"
#include "slope.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include "get-dice-from-image.h"

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
	//std::string tesseractPath = "/usr/local/Cellar/tesseract/4.0.0_1/share/tessdata/";
	std::string tesseractPath = "C:\\Users\\stuar\\github\\dice-scanner\\dicescanner";
	initOcr(tesseractPath);
	std::string path = "";
	std::string intermediateImagePath = path + "progress/";
	std::vector<std::string> names = {
		"1", "2", "3", "4", "5",
		"6", "7", "8", "9"
	};
	help(argv[0]);

	if (argc > 1)
	{
		names[0] = argv[1];
		names[1] = "0";
	}

	// Make param1 input directory, param2 output directory

	// List directory for box-*.[jpg|png] files, where * is correct value
  
	// Run full box tesst

	// List directory for die-*.[jpg|png] files, where * is letter/digit

	// Run die test


	for (auto& filename : names) {
		std::string fname = path + "img/" + filename + ".jpg";
		cv::Mat image = cv::imread(fname, cv::IMREAD_COLOR);
		if (image.empty())
		{
			std::cout << "Couldn't load " << filename << std::endl;
			continue;
		}

		getDiceFromImage(image, filename, intermediateImagePath + filename, 1);
	}
	return 0;
}
