
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
#include "locate-dice.h"
#include "read-dice.h"
#define _USE_MATH_DEFINES
#include <math.h>


static void help(const char* programName)
{
	std::cout <<
		"\nA program using pyramid scaling, Canny, contours and contour simplification\n"
		"to find squares in a list of images (pic1-6.png)\n"
		"Returns sequence of squares detected on the image.\n"
		"Call:\n"
		"./" << programName << " [file_name (optional)]\n"
		"Using OpenCV version " << CV_VERSION << "\n" << std::endl;
}


const char* wndname = "Square Detection Demo";



// the function draws all the squares in the image
static void drawSquares(cv::Mat & image, const std::vector<std::vector<cv::Point> > & squares)
{
	auto clone = image.clone();
	for (size_t i = 0; i < squares.size(); i++)
	{
		const cv::Point* p = &squares[i][0];
		int n = (int)squares[i].size();
		polylines(clone, &p, &n, 1, true, cv::Scalar(0, 255, 0), 3, cv::LINE_AA);
	}

	imshow(wndname, clone);
}

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
	// std::string path = "";
	string path = "/Users/stuart/github/dice-scanner/";
	std::vector<std::string> names = { "1", "2", "3", "4", "5" };
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

		auto squaresFound = findSquares(image, path, filename);

		auto boxSlope = median(vmap<RectangleDetected, float>(squaresFound.candidateDiceSquares, [](RectangleDetected r) { return slope(r.topLeft(), r.topRight()); }));

		auto rotatedImage = rotateImageAndRectanglesFound(image, squaresFound, boxSlope);

		writeSquares(rotatedImage, squaresFound.candidateUnderlineRectangles, path + "squares/underlines" + filename + ".png");

		auto dice = filterAndOrderSquares(rotatedImage, squaresFound.candidateDiceSquares);

		for (uint i = 0; i < dice.size(); i++) {
			auto die = dice[i];
			imwrite(path + "dice/" + filename + "-" + std::to_string(i) + ".png", die);
		}
		// writeSquares(rotatedImage, diceSquares.squares, path + "squares/" + filename + ".png");

		if (dice.size() < 25) {
			std::cout << "Not enough dice in " << filename << std::endl;
			continue;
		}


		//// Rotate image by diceSquares.angle
		//cv::Mat grayPreRotation, grayPostRotation;
		//cv::cvtColor(rotatedImage, grayPreRotation, cv::COLOR_BGR2GRAY);
		//cv::Point2d center = diceSquares.squares[12].center;
		//float correctionAngle = diceSquares.angleRadians * 360 / (2 * (float) M_PI);
		//auto rotationMatrix = cv::getRotationMatrix2D(center, correctionAngle, 1.0f);

		//cv::warpAffine(grayPreRotation, grayPostRotation, rotationMatrix, grayPreRotation.size() );
		//imwrite(path + "gray/" + filename + ".png", grayPostRotation);

		// drawSquares(image, squares);

		//int c = waitKey();
		//if (c == 27)
		//	break;
	}

	return 0;
}
