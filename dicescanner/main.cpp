
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

std::vector<DieRead> getDiceFromImage(cv::Mat image, std::string fileIdentifier, std::string intermediateImagePath = "/dev/null", int boxDebugLevel = 0)
{
	cv::Mat gray;
	cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);

	//
	// Stage one: identify the dice box by identifying the 25 squares formed by the 25 dice
	//
	auto candidateDiceSquares = findCandidateDiceSquares(gray);

	//
	// Step two: find and remove the slope (angle) so that the dice are horizonally aligned
	// Future: should simply unskew image to have same effect but also remove kew
	//
	auto boxSlope = median(vmap<RectangleDetected, float>(candidateDiceSquares, [](RectangleDetected r) { return slope(r.topLeft(), r.topRight()); }));
	auto rotatedImage = rotateImageAndRectanglesFound(gray, candidateDiceSquares, boxSlope);

	// Display the rotated dice for debuggin purposes.
	cv::Mat rotatedColor;
	cv::cvtColor(rotatedImage, rotatedColor, cv::COLOR_GRAY2BGR);
	if (!intermediateImagePath.find("/dev/null") == 0 && boxDebugLevel >= 1) {
		writeSquares(rotatedColor, candidateDiceSquares, intermediateImagePath + "-dice-squares" + ".png");
	}

	//
	// Find the dice within the flattened image and calculate the number of pixesl per millimeter
	// in the process
	//
	float approxPixelsPerMm;
	auto diceGrayscaleImages = findDice(rotatedImage, candidateDiceSquares, approxPixelsPerMm);

	//
	// Read each of the 25 dice indivudally by finding the underline to determine the rotation
	// and then reading the text above the underline.
	// auto diceRead = orientAndReadDice(dice, approxPixelsPerMm, intermediateImagePath);
	std::vector<DieRead> diceRead;
	for (uint i = 0; i < diceGrayscaleImages.size(); i++) {
		DieRead dieRead;
		int dieDebugLevel =
			((fileIdentifier.size() > 0) && (fileIdentifier[fileIdentifier.size() -1] == '1') && (i == 16)) ? 2 :
			boxDebugLevel;

		orientAndReadDie(intermediateImagePath + + "-" + std::to_string(i) + "-", diceGrayscaleImages[i], dieRead, approxPixelsPerMm, dieDebugLevel);
		//
		auto dieRead = diceRead[i];
		std::string identifier = "-" + std::to_string(i);
		if (dieRead.letterConfidence > 0 && dieRead.digitConfidence > 0) {
			identifier += " ";
			identifier += dieRead.letter;
			identifier += dieRead.digit;
			identifier += " " + std::to_string(dieRead.orientationInDegrees) + "";
			std::cout << "File " << fileIdentifier << "Die " << i << " at angle " << dieRead.orientationInDegrees <<
				" read as " << std::string(1, dieRead.letter) << std::string(1, dieRead.digit) <<
				" with confidence " << dieRead.letterConfidence << ", " << dieRead.digitConfidence <<
				"\n";
		} else {
			identifier += "READ-ERROR";
			std::cout << "File " << fileIdentifier << " Die " << i << " at angle " << dieRead.orientationInDegrees << " could not be read.\n";
		}
		if (!intermediateImagePath.rfind("/dev/null", 0) == 0 && dieDebugLevel >= 1) {
			cv::imwrite(intermediateImagePath + identifier + "-die.png", diceGrayscaleImages[i]);
		}

	}
	
	// writeSquares(rotatedImage, diceSquares.squares, path + "squares/" + filename + ".png");

	if (diceGrayscaleImages.size() < 25) {
		std::cout << "Not enough dice in found in " << intermediateImagePath << std::endl;
	}

	return diceRead;
}


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
