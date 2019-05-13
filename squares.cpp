
// The "Square Detector" program.
// It loads several images sequentially and tries to find squares in
// each image

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include <iostream>

using namespace std;
using namespace cv;


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

class Rectangle {
public:
	vector<Point> points;
	double area;
	double maxCos;
	vector<double> sideLengths;
	double maxSideLength;
	double minSideLength;
	Rectangle(vector<Point> points) {
		this->points = points;
		this->area = contourArea(points);
		this->maxCos = 0;
		this->sideLengths.push_back(
			this->maxSideLength = this->minSideLength = norm(points[3] - points[0])
		);
		for (int p = 0; p < 4; p++) {
			double sideLength = norm(points[p] - points[(p + 1) % 4]);
			this->maxSideLength = MAX(this->maxSideLength, sideLength);
			this->minSideLength = MIN(this->minSideLength, sideLength);
			this->sideLengths.push_back(sideLength);
		}
		for (int j = 2; j < 5; j++)
		{
			// find the maximum cosine of the angle between joint edges
			double cosine = fabs(angle(points[j % 4], points[j - 2], points[j - 1]));
			this->maxCos = MAX(this->maxCos, cosine);
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


int thresh = 50, N = 11;
const char* wndname = "Square Detection Demo";

bool sortRectByArea(Rectangle a, Rectangle b) {
	return a.area < b.area;
}

// returns sequence of squares detected on the image.
static void findSquares(const Mat & image, vector<vector<Point> > & squares)
{
	squares.clear();

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
				// squares.push_back(approx);
			}
		}
	}
	// sort rectangles by area
	std::sort(rects.begin(), rects.end(), sortRectByArea);
	double medianArea = rects[rects.size() / 2].area;
	double minArea = 0.9 * medianArea;
	double maxArea = 1.1 * medianArea;
	for (auto& rect : rects) {
		if (rect.area >= minArea && rect.area <= maxArea) {
			squares.push_back(rect.points);
		}
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
		string fname = "img/" + filename;
		Mat image = imread(fname, IMREAD_COLOR);
		if (image.empty())
		{
			cout << "Couldn't load " << filename << endl;
			continue;
		}

		findSquares(image, squares);
		writeSquares(image, squares, "squares/" + filename);
		// drawSquares(image, squares);

		//int c = waitKey();
		//if (c == 27)
		//	break;
	}

	return 0;
}