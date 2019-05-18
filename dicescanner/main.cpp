
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
#include "locate-dice.h"
#define _USE_MATH_DEFINES
#include <math.h>

using namespace std;
using namespace cv;


static void help(const char* programName)
{
	cout <<
		"\nA program using pyramid scaling, Canny, contours and contour simplification\n"
		"to find squares in a list of images (pic1-6.png)\n"
		"Returns sequence of squares detected on the image.\n"
		"Call:\n"
		"./" << programName << " [file_name (optional)]\n"
		"Using OpenCV version " << CV_VERSION << "\n" << endl;
}


const char* wndname = "Square Detection Demo";



// the function draws all the squares in the image
static void drawSquares(Mat & image, const vector<vector<Point> > & squares)
{
	auto clone = image.clone();
	for (size_t i = 0; i < squares.size(); i++)
	{
		const Point* p = &squares[i][0];
		int n = (int)squares[i].size();
		polylines(clone, &p, &n, 1, true, Scalar(0, 255, 0), 3, LINE_AA);
	}

	imshow(wndname, clone);
}

// the function draws all the squares in the image
static void writeSquares(Mat& image, const vector<vector<Point> >& squares, string name)
{
	for (size_t i = 0; i < squares.size(); i++)
	{
		const Point* p = &squares[i][0];
		int n = (int)squares[i].size();
		polylines(image, &p, &n, 1, true, Scalar(0, 255, 0), 3, LINE_AA);
	}

	// imshow(wndname, image);
	imwrite(name, image);
}


int main(int argc, char** argv)
{
	string path = "";
	// string path = "/Users/stuart/github/dice-scanner/squares/"
	std::vector<std::string> names = { "1", "2", "3", "4", "5" };
	help(argv[0]);

	if (argc > 1)
	{
		names[0] = argv[1];
		names[1] = "0";
	}

	vector<vector<Point> > squares;

	for (auto& filename : names) {
		string fname = path + "img/" + filename + ".jpg";
		Mat image = imread(fname, IMREAD_COLOR);
		if (image.empty())
		{
			cout << "Couldn't load " << filename << endl;
			continue;
		}

		auto diceSquares = filterAndOrderSquares(findSquares(image, path, filename));

		auto squares = vmap<Rectangle, std::vector<Point>>(diceSquares.squares, [](Rectangle r) {
			return r.points;
			});
		writeSquares(image, squares, path + "squares/" + filename + ".png");

		if (diceSquares.squares.size() < 13) {
			cout << "Not enough squares in " << filename << endl;
			continue;
		}

		// Rotate image by diceSquares.angle
		cv::Mat grayPreRotation, grayPostRotation;
		cv::cvtColor(image, grayPreRotation, cv::COLOR_BGR2GRAY);
		Point2d center = diceSquares.squares[12].center;
		double correctionAngle = diceSquares.angleRadians * 360 / (2 * M_PI);
		auto rotationMatrix = cv::getRotationMatrix2D(center, correctionAngle, 1.0);

		cv::warpAffine(grayPreRotation, grayPostRotation, rotationMatrix, grayPreRotation.size() );
		imwrite(path + "gray/" + filename + ".png", grayPostRotation);

		// drawSquares(image, squares);

		//int c = waitKey();
		//if (c == 27)
		//	break;
	}

	return 0;
}
