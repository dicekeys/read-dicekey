
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

using namespace std;
using namespace cv;


static double distance2d(const Point2d &a, const Point2d &b) {
	double dx = a.x - b.x;
	double dy = a.y - b.y;
	return sqrt( dx * dx + dy * dy);
}


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


static double slope(const Point &a, const Point &b)
{
	return a.x == b.x ?
		DBL_MAX :
		((double)b.y - a.y) /((double)b.x - a.x);
}

static vector<Rectangle> filterAndOrderSquares(const vector<Rectangle> &squares)
{
	const double dieSize = 8; // 8mm die size
	const double gapBetweenDiceEdges = 1.8; // 1.8mm
	const double dieSizeToDistBetweenDice = ((dieSize + gapBetweenDiceEdges) / dieSize);
	auto medianTopSlope = median(vmap<Rectangle, double>(squares, [](Rectangle r) { return slope(r.topLeft, r.topRight); }));
	auto theta = atan(medianTopSlope);
	auto medianLineLength = median(vmap<Rectangle, double>(squares, [](Rectangle r) { return (double)r.maxSideLength; }));
	auto distanceBetweenDice = medianLineLength * dieSizeToDistBetweenDice;
	double y_threshold = distanceBetweenDice  / 2;

	auto removeSlope = [medianTopSlope](Point2d point) -> Point2d {
		// y = mx + b => b = y - mx, where b is the y intercept and m is the slope
		double y_intercept = point.y - medianTopSlope * point.x;
		Point2d y_intercept_point = Point2d(0, y_intercept);
		// the length of the line from the y intercept
		double dist_from_y_intercept = distance2d(y_intercept_point, point);
		return Point2d(dist_from_y_intercept, y_intercept);
	};
	auto restoreSlope = [theta](Point2d point) -> Point2d {
		double dist_from_y_intercept = point.x;
		auto y = point.y + dist_from_y_intercept * sin(theta);
		auto x = dist_from_y_intercept * cos(theta);
		return Point2d(x, y);
	};

	vector<Rectangle> sortedSquares = squares;
	std::sort(sortedSquares.begin(), sortedSquares.end(), [removeSlope, y_threshold](const Rectangle a, const Rectangle b) {
		const auto adjusted_a_center = removeSlope(a.center);
		const auto adjusted_b_center = removeSlope(b.center);
		if (abs(adjusted_a_center.y - adjusted_b_center.y) > y_threshold) {
			// After adjusting for slope, there's a big enough difference in the Y axis
			// to sort based on the row (Y axis, or height from top to bottom)
			return adjusted_a_center.y < adjusted_b_center.y;
		} else {
			// Within the same row, sort by column (x axis)
			return adjusted_a_center.x < adjusted_b_center.x;
		}
	});

	vector<Point2d> adjustedCenters = vmap<Rectangle, Point2d>(sortedSquares, [removeSlope](Rectangle r) {
		return removeSlope(r.center);
	});
	auto medianAdjustedX = median(vmap<Point2d, double>(adjustedCenters, [](Point2d point) { return point.x; }));
	auto medianAdjustedY = median(vmap<Point2d, double>(adjustedCenters, [](Point2d point) { return point.y; }));
	auto medianAdjustedCenter = Point2d(medianAdjustedX, medianAdjustedY);
	auto medianCenter = restoreSlope(medianAdjustedCenter);

	// auto medianBottomSlope = median(vmap<Rectangle, double>(squares, [](Rectangle r) { return slope(r.bottomLeft, r.bottomRight); }));
	// auto medianLeftSlope = median(vmap<Rectangle, double>(squares, [](Rectangle r) { return slope(r.topLeft, r.bottomLeft); }));
	// auto medianRightSlope = median(vmap<Rectangle, double>(squares, [](Rectangle r) { return slope(r.topRight, r.bottomRight); }));
	// auto topBottomLineSlope = (medianTopSlope + medianBottomSlope) / 2;
	auto medianTopLineDeltaX = median(vmap<Rectangle, double>(squares, [](Rectangle r) { return (double)r.topRight.x - r.topLeft.x; }));
	auto medianTopLineDeltaY = median(vmap<Rectangle, double>(squares, [](Rectangle r) { return (double)r.topRight.y - r.topLeft.y; }));
	double delta_x = medianTopLineDeltaX * dieSizeToDistBetweenDice;
	double delta_y = medianTopLineDeltaX * dieSizeToDistBetweenDice;

	// n squared algorithm to count neighbors
	std::vector<bool> hasLeft(squares.size());
	std::vector<bool> hasRight(squares.size());
	std::vector<bool> hasAbove(squares.size());
	std::vector<bool> hasBelow(squares.size());
	double dist_threshold = medianLineLength / 4; // 2mm threshold from center
	for (uint i = 0; i < squares.size(); i++) {
		// Search for neighbor to right
		auto myCenter = squares[i].center;
		auto right = Point2d(myCenter.x + delta_x, myCenter.y + delta_y);
		auto below = Point2d(myCenter.x + delta_y, myCenter.y + delta_x);
		for (uint j = 0; j < squares.size(); j++) {
			if (j == i) continue;
			auto rightDistance = distance2d(right, squares[j].center);
			if (rightDistance < dist_threshold) {
				hasRight[i] = hasLeft[j] = true;
			}
			auto belowDistance = distance2d(below, squares[j].center);
			if (belowDistance < dist_threshold) {
				hasBelow[i] = hasAbove[j] = true;
			}	
		}
	}
	return sortedSquares;


	// If there are more than 25 squares, remove those 

	// vector<Rectangle> sortedSquares = squares;
	// std::sort(sortedSquares.begin(), sortedSquares.end(), [](Rectangle a, Rectangle b) {
	// 	return (a.center.x + a.center.y) < (b.center.x + b.center.y); 
	// });
	// vector<double> delta_xs;
	// vector<double> delta_ys;
	// for (uint i = 1; i < sortedSquares.size(); i++) {
	// 	delta_xs.push_back(sortedSquares[i].center.x - sortedSquares[i - 1].center.x);
	// 	delta_ys.push_back(sortedSquares[i].center.y - sortedSquares[i - 1].center.y);
	// }
	// const double median_delta_x = median(delta_xs);
	// const double median_delta_y = median(delta_ys);
	// const double max_delta_x = median_delta_x * 1.15;
	// const double min_delta_x = median_delta_x / 1.15;
	// const double max_delta_y = median_delta_x * 1.15;
	// const double min_delta_y = median_delta_x / 1.15;
	// bool atNewRow = true;
	// int numSquaresReadInRow = 0;

	// // Filter out squares that don't have at least one die to their right or left
	// vector<Rectangle> squaresWithNeighbors;
	// bool hasNeighborToLeft = false;
	// for (uint i = 0; i < sortedSquares.size(); i++) {
	// 	const auto center = sortedSquares[i].center;
	// 	const bool hasNeighborToRight = (
	// 		i + 1 < sortedSquares.size() &&
	// 		sortedSquares[i + 1].center.x < center.x + max_delta_x &&
	// 		sortedSquares[i + 1].center.x > center.x + min_delta_x &&
	// 		sortedSquares[i + 1].center.y < center.y + max_delta_y &&
	// 		sortedSquares[i + 1].center.y > center.y + min_delta_y
	// 		);
	// 	if (hasNeighborToLeft || hasNeighborToLeft) {
	// 		squaresWithNeighbors.push_back(sortedSquares[i]);
	// 	}
	// 	hasNeighborToLeft = hasNeighborToRight;
	// }
	
	// return squaresWithNeighbors;

}


// the function draws all the squares in the image
static void drawSquares(Mat & image, const vector<vector<Point> > & squares)
{
	for (size_t i = 0; i < squares.size(); i++)
	{
		const Point* p = &squares[i][0];
		int n = (int)squares[i].size();
		polylines(image, &p, &n, 1, true, Scalar(0, 255, 0), 3, LINE_AA);
	}

	imshow(wndname, image);
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
	std::vector<std::string> names = { "1.jpg", "2.jpg", "3.jpg" };
	help(argv[0]);

	if (argc > 1)
	{
		names[0] = argv[1];
		names[1] = "0";
	}

	vector<vector<Point> > squares;

	for (auto& filename : names) {
		string fname = path + "img/" + filename;
		Mat image = imread(fname, IMREAD_COLOR);
		if (image.empty())
		{
			cout << "Couldn't load " << filename << endl;
			continue;
		}

		auto rects = filterAndOrderSquares(findSquares(image));
		vector<vector<Point>> squares;
		std::transform(rects.begin(), rects.end(), std::back_inserter(squares), [](Rectangle r) {
			return r.points;
		});
		writeSquares(image, squares, path + "squares/" + filename);
		// drawSquares(image, squares);

		//int c = waitKey();
		//if (c == 27)
		//	break;
	}

	return 0;
}
