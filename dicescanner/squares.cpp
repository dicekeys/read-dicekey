
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

class ValueCluster {
	public:
	std::vector<double> samples;
	double median;

	ValueCluster(double firstSample) {
		samples.push_back(firstSample);
	}

	void addSample(double sample) {
		samples.push_back(sample);
		// Re-sort list
		for (uint i = samples.size() - 2; i > 0 && samples[i+1] < samples[i]; i--) {
			swap(samples[i], samples[i-1]);
		}

		if (samples.size() % 2 == 1) {
			median = samples[samples.size()/2];
		} else {
			auto c = samples.size()/2;
			median = (samples[c] + samples[c-1])/2;
		}

	}
};

class ValueClusters {
	double proximityThreshold;

	public:
	std::list<ValueCluster> clusters;

	ValueClusters(double proximityThreshold) {
		this->proximityThreshold = proximityThreshold;
	}

	void addSample(double sample) {
		for (auto it = clusters.begin(); it != clusters.end(); ++it) {
			if (abs(sample - it->median) < proximityThreshold) {
				// The sample is within close enough proximity to the mean for this
				// cluster to include it in the mean
				it->addSample(sample);
				return;
			} else {
				if (sample < it->median) {
					// The sample comes before this cluster,
					// so we need to create a new cluster for it
					clusters.insert(it, ValueCluster(sample));
					return;
				}
			}
		}
		// The sample comes after all of the existing clusters,
		// and so should in a new cluster at the end of the list.
		if (clusters.empty()) {
			clusters.push_back(ValueCluster(sample));
		}
	}
};


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

	//
	// Calculate the median slope, and create functions to remove and restore the slope
	//
	auto medianTopSlope = median(vmap<Rectangle, double>(squares, [](Rectangle r) { return slope(r.topLeft, r.topRight); }));
	auto theta = atan(medianTopSlope);

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


	//
	// Sort dice based on their slope-adjusted location
	//
	auto medianLineLength = median(vmap<Rectangle, double>(squares, [](Rectangle r) { return (double)r.maxSideLength; }));
	auto distanceBetweenDice = medianLineLength * dieSizeToDistBetweenDice;
	double y_threshold = distanceBetweenDice  / 2;


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

	//
	// Find the median distance between dice by taking the mean distance between
	// squares and their horizontal neighbors
	///
	vector<double> distancesBetweenCenters;
	for (uint i = 1; i < sortedSquares.size(); i++) {
		if (sortedSquares[i].center.x < sortedSquares[i-1].center.x) {
			distancesBetweenCenters.push_back( distance2d(sortedSquares[i].center, sortedSquares[i-1].center ));
		}
	}
	if (distancesBetweenCenters.size() > 0) {
		distanceBetweenDice = median(distancesBetweenCenters);
	}

	// Cluster the adjusted x and y values so we can look for
	// outliers, and re-create squares that the algorithm failed
	// to find.
	ValueClusters xClusters(distanceBetweenDice * 0.33);
	ValueClusters yClusters(distanceBetweenDice * 0.33);
	for (auto rect : sortedSquares) {
		auto adjustedCenter = removeSlope(rect.center);
		xClusters.addSample(adjustedCenter.x);
		xClusters.addSample(adjustedCenter.y);
	}

	// FIXME -- more work to adjust for square-generation errors

	// If more than 5 clusters, pick group of five that has most samples
	// Create locations of 25 die centers.
	// Ensure there is a square for each center, removing squares not associated with die location
	// Fill in missing squares.


	return sortedSquares;
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
