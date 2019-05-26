
// The "Square Detector" program.
// It loads several images sequentially and tries to find squares in
// each image

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


static void help(const char* programName)
{
	std::cout <<
		"Using OpenCV version " << CV_VERSION << "\n" << std::endl;
}


const char* wndname = "Square Detection Demo";



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

// the function draws all the squares in the image
static void writeSquares(cv::Mat& image, const std::vector<RectangleDetected>& rects, std::string name)
{
	auto clone = image.clone();
	for (auto rect : rects)
	{
		std::vector<cv::Point> points = vmap<cv::Point2f, cv::Point>(rect.points, [](cv::Point2f point) -> cv::Point {
			return cv::Point(point);
		});
		const cv::Point* p = points.data();
		int n = (int)points.size();
		polylines(clone, &p, &n, 1, true, cv::Scalar(0, 255, 0), 3, cv::LINE_AA);
	}

	// cv::imshow(wndname, image);
	cv::imwrite(name, clone);
}


int main(int argc, char** argv)
{
	//std::string tesseractPath = "/usr/local/Cellar/tesseract/4.0.0_1/share/tessdata/";
	std::string tesseractPath = "C:\\Users\\stuar\\github\\dice-scanner\\dicescanner";
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

	for (auto& filename : names) {
		std::string fname = path + "img/" + filename + ".jpg";
		cv::Mat image = cv::imread(fname, cv::IMREAD_COLOR);
		if (image.empty())
		{
			std::cout << "Couldn't load " << filename << std::endl;
			continue;
		}

		cv::Mat gray;
		cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);

		auto candidateDiceSquares = findCandidateDiceSquares(gray, path, filename);

		auto boxSlope = median(vmap<RectangleDetected, float>(candidateDiceSquares, [](RectangleDetected r) { return slope(r.topLeft(), r.topRight()); }));

		auto rotatedImage = rotateImageAndRectanglesFound(gray, candidateDiceSquares, boxSlope);

		cv::Mat rotatedColor;
		cv::cvtColor(rotatedImage, rotatedColor, cv::COLOR_GRAY2BGR);

		writeSquares(rotatedColor, candidateDiceSquares, intermediateImagePath + "dice-squares-" + filename + ".png");

		float approxPixelsPerMm;
		auto dice = findDice(rotatedImage, candidateDiceSquares, approxPixelsPerMm);

		for (uint i = 0; i < dice.size(); i++) {
			auto die = dice[i];
			char letter = 0, digit = 0;
			float letterConfidence = 0, digitConfidence = 0;
			DieRead dieRead;
			auto diereadSuccessfully = orientAndReadDie(tesseractPath, intermediateImagePath + filename + "-" + std::to_string(i) + "-", die, dieRead, approxPixelsPerMm, i);
			std::string identifier = filename + "-" + std::to_string(i);
			if (diereadSuccessfully) {
				identifier += " ";
				identifier += dieRead.letter;
				identifier += dieRead.digit;
				identifier += " " + std::to_string(dieRead.orientationInDegrees) + " ";
				std::cout << "Die " << i << " at angle " << dieRead.orientationInDegrees << " read as " << std::string(1, dieRead.letter) << std::string(1, dieRead.digit) + "\n";
			} else {
				identifier += "READ-ERROR";
				std::cout << "Die " << i << " at angle " << dieRead.orientationInDegrees << " could not be read.\n";
			}
			imwrite(intermediateImagePath + identifier + ".png", die);
			

			// imwrite(intermediateImagePath + "dice-edges-" + identifier + ".png", edges);
		}
		// writeSquares(rotatedImage, diceSquares.squares, path + "squares/" + filename + ".png");

		if (dice.size() < 25) {
			std::cout << "Not enough dice in " << filename << std::endl;
			continue;
		}

	}
	return 0;
}
