
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

using namespace std;
using namespace cv;

template <typename T, typename U>
T vreduce(const std::vector<T>& vectorToReduce,
         const std::function<U(U,T)>& reduceFn,
		 const U initialValue) {
	U result = initialValue;
	for (auto e : vectorToReduce) {
		result = reduceFn(result, e);
	}
	return result;
}

template <typename T, typename U>
std::vector<U> vmap(const std::vector<T>& data, const std::function<U(T)> mapper) {
    std::vector<U> result;
	for (auto e : data) {
        result.push_back(mapper(e));
	};
	return result;
}

template <typename T>
std::vector<T> vfilter(const std::vector<T>& data, std::function<bool(T)> filterFn) {
    std::vector<T> result = data;
	result.erase(remove_if(result.begin(), result.end(), [filterFn](T element){
		return !filterFn(element);
	}), result.end());
    return result;
}

// helper function:
// finds a cosine of angle between vectors
// from pt0->pt1 and from pt0->pt2
static double angle(Point pt1, Point pt2, Point pt0)
{
	double dx1 = (double)pt1.x - pt0.x;
	double dy1 = (double)pt1.y - pt0.y;
	double dx2 = (double)pt2.x - pt0.x;
	double dy2 = (double)pt2.y - pt0.y;
	return (dx1 * dx2 + dy1 * dy2) / sqrt((dx1 * dx1 + dy1 * dy1) * (dx2 * dx2 + dy2 * dy2) + 1e-10);
}

static double median(const vector<double> &numbers)
{
	vector<double> sorted = numbers;
	std::sort(sorted.begin(), sorted.end(), [](double a, double b) { return a < b; });
	if (sorted.size() == 0) {
		return 0;
	} else if (sorted.size() % 2 > 0) {
		return sorted[sorted.size() / 2];
	} else {
		return (sorted[sorted.size() / 2] + sorted[1 + sorted.size() / 2]) / 2;
	}
}

static Point2f getCenter(vector<Point> &points)
{
	float x = 0;
	float y = 0;
	for (auto& point : points) {
		x += point.x;
		y += point.y;
	}
	x /= points.size();
	y /= points.size();
	return Point2f(x, y);
}



class Rectangle {
public:
	vector<Point> points;
	Point topLeft;
	Point topRight;
	Point bottomLeft;
	Point bottomRight;
	double area;
	double maxCos;
	vector<double> sideLengths;
	double maxSideLength;
	double minSideLength;
	Point2f center;
	// quality is used to determine which of two overlapping rectangles
	// is better, prefering straighter corners and more area.
	double qualityLowerIsBetter;

	Rectangle(vector<Point> &fromPoints) {
		points = fromPoints;
		std::sort(points.begin(), points.end(), [](Point a, Point b) {return a.y < b.y; });
		// The first two points are the top two.  Sort them top left and top right
		if (points[0].x > points[1].x) {
			swap(points[0], points[1]);
		}
		// The second set of two points are the bottom two.  Sort them right then left.
		if (points[2].x < points[3].x) {
			swap(points[2], points[3]);
		}
		topLeft = points[0];
		topRight = points[1];
		bottomRight = points[2];
		bottomLeft = points[3];

		center = getCenter(points);
		area = contourArea(points);
		maxCos = 0;
		sideLengths.push_back(
			maxSideLength = minSideLength = norm(points[3] - points[0])
		);
		for (int p = 1; p < 4; p++) {
			double sideLength = norm(points[p] - points[(p + 1) % 4]);
			maxSideLength = MAX(maxSideLength, sideLength);
			minSideLength = MIN(minSideLength, sideLength);
			sideLengths.push_back(sideLength);
		}
		for (int j = 2; j < 5; j++)
		{
			// find the maximum cosine of the angle between joint edges
			double cosine = fabs(angle(points[j % 4], points[j - 2], points[j - 1]));
			maxCos = MAX(maxCos, cosine);
		}
		// qualityLowerIsBetter = area > 0 ? maxCos / area : 1;
		qualityLowerIsBetter = area > 0 ? maxCos / pow(area, 5) : 1;
	}

	bool overlaps(Rectangle& otherRect) {
		return
			pointPolygonTest(otherRect.points, center, false) >= 0 ||
			pointPolygonTest(points, otherRect.center, false) >= 0;
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


int thresh = 50, N = 11;
const char* wndname = "Square Detection Demo";


// returns sequence of squares detected on the image.
static vector<Rectangle> findSquares(const Mat & image)
{
	vector<Rectangle> rects;

	Mat pyr, timg, gray0(image.size(), CV_8U), gray;

	// down-scale and upscale the image to filter out the noise
	pyrDown(image, pyr, Size(image.cols / 2, image.rows / 2));
	pyrUp(pyr, timg, image.size());

	// find squares in every color plane of the image
	for (int c = 0; c < 3; c++)
	{
		int ch[] = { c, 0 };
		mixChannels(&timg, 1, &gray0, 1, ch, 1);

		// try several threshold levels
		for (int l = 0; l < N; l++)
		{
			// hack: use Canny instead of zero threshold level.
			// Canny helps to catch squares with gradient shading
			if (l == 0)
			{
				// apply Canny. Take the upper threshold from slider
				// and set the lower to 0 (which forces edges merging)
				Canny(gray0, gray, 0, thresh, 5);
				// dilate canny output to remove potential
				// holes between edge segments
				dilate(gray, gray, Mat(), Point(-1, -1));
			}
			else
			{
				// apply threshold if l!=0:
				//     tgray(x,y) = gray(x,y) < (l+1)*255/N ? 255 : 0
				gray = gray0 >= (l + 1) * 255 / N;
			}

			// find contours and store them all as a list
			vector<vector<Point>> contours;
			findContours(gray, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);

			vector<Point> approx;
			// test each contour
			for (auto& contour : contours) {
				// approximate contour with accuracy proportional
				// to the contour perimeter
				approxPolyDP(contour, approx, arcLength(contour, true) * 0.02, true);

				// square contours should have 4 vertices after approximation
				// relatively large area (to filter out noisy contours)
				// and be convex.
				// Note: absolute value of an area is used because
				// area may be positive or negative - in accordance with the
				// contour orientation
				if (approx.size() != 4 || !isContourConvex(approx))
					continue;

				auto rect = Rectangle(approx);

				// The angle indicates this isn't a square
				if (rect.maxCos > 0.25)
					continue;
				// If the longest side is more than 20% longer than the
				// shortest side, this isn't a square
				if (rect.maxSideLength > 1.20 * rect.minSideLength)
					continue;
				// If there's not pixels to read text (30x30), it's not our square
				if (rect.area < 900)
					continue;

				rects.push_back(rect);
			}
		}
	}
	if (rects.size() <= 0) {
		// There are not enough squares, so return the empty vector
		return rects;
	}

	// Remove rectangles that stray from the median
	double medianArea = median(vmap<Rectangle, double>(rects, [](Rectangle r) -> double { return r.area; }));
	double minArea = 0.9 * medianArea;
	double maxArea = 1.1 * medianArea;
	auto median_rectangles = vfilter<Rectangle>(rects, [minArea, maxArea](Rectangle r) { return  (r.area >= minArea || r.area <= maxArea); });

	// remove overlapping rectangeles
	vector<Rectangle> non_overlapping_rectangles;
	for (auto& rect : median_rectangles) {
		int overlaps_with_index = -1;
		for (int i = 0; i < non_overlapping_rectangles.size(); i++) {
			if (rect.overlaps(non_overlapping_rectangles[i])) {
				overlaps_with_index = i;
				break;
			}
		}
		if (overlaps_with_index == -1) {
			// This rectangle doesn't overlap with others
			non_overlapping_rectangles.push_back(rect);
		}
		else {
			// This recangle overlaps with another. Pick which to keep
			if (rect.qualityLowerIsBetter < non_overlapping_rectangles[overlaps_with_index].qualityLowerIsBetter) {
				// Choose the rectangle with better quality
				non_overlapping_rectangles[overlaps_with_index] = rect;
			}
		}
	}

	return non_overlapping_rectangles;
}

static double slope(const Point &a, const Point &b)
{
	return a.x == b.x ?
		DBL_MAX :
		((double)b.y - a.y) /((double)b.x - b.x);
}

static void processSquares(vector<Rectangle> &squares)
{
	vector<Rectangle> sortedSquares = squares;
	std::sort(sortedSquares.begin(), sortedSquares.end(), [](Rectangle a, Rectangle b) {
		return (a.center.x + a.center.y) < (b.center.x + b.center.y); 
	});
	auto medianTopSlope = median(vmap<Rectangle, double>(sortedSquares, [](Rectangle r) { return slope(r.topLeft, r.topRight); } ));
	auto medianBottomSlope = median(vmap<Rectangle, double>(sortedSquares, [](Rectangle r) { return slope(r.bottomLeft, r.bottomRight); } ));
	auto medianLeftSlope = median(vmap<Rectangle, double>(sortedSquares, [](Rectangle r) { return slope(r.topLeft, r.bottomLeft); } ));
	auto medianRightSlope = median(vmap<Rectangle, double>(sortedSquares, [](Rectangle r) { return slope(r.topRight, r.bottomRight); } ));
	for (auto square : squares) {

	}
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
	std::vector<std::string> names = { "1.jpg", "2.jpg", "3.jpg" };
	help(argv[0]);

	if (argc > 1)
	{
		names[0] = argv[1];
		names[1] = "0";
	}

	vector<vector<Point> > squares;

	for (auto& filename : names) {
		// string fname = "img/" + filename;
		string fname = "/Users/stuart/github/dice-scanner/img/" + filename;
		Mat image = imread(fname, IMREAD_COLOR);
		if (image.empty())
		{
			cout << "Couldn't load " << filename << endl;
			continue;
		}

		auto rects = findSquares(image);
		vector<vector<Point>> squares;
		std::transform(rects.begin(), rects.end(), std::back_inserter(squares), [](Rectangle r) {
			return r.points;
		});
		writeSquares(image, squares, "/Users/stuart/github/dice-scanner/squares/" + filename);
		// writeSquares(image, squares, "squares/" + filename);
		// drawSquares(image, squares);

		//int c = waitKey();
		//if (c == 27)
		//	break;
	}

	return 0;
}
