
#include <float.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include <iostream>
#include "vfunctional.h"
#include "rectangle.h"

using namespace std;
using namespace cv;


// returns sequence of squares detected on the image.
static vector<Rectangle> findSquares(const Mat & image, int thresh = 50, int N = 11)
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
		for (uint i = 0; i < non_overlapping_rectangles.size(); i++) {
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
