#pragma once

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
static vector<Rectangle> findSquares(const Mat &image, string path, string filename, int thresh = 50, int N = 20)
{
	/*
	Next step:
	   Cluster areas by binary orders of magnitude.
		(two sets: a from 2^i - 2^(i+1), the other from 1.5 * 2^i - 1.5 * 2^(i+1) so that we don't have boundary issues)
	   Take largest cluster with n (20?  25?) objects
	     Only works at one threshold.  Can do more?
	*/
	vector<Rectangle> rects;

	Mat pyr, timg, gray0(image.size(), CV_8U), gray;

	blur(image, timg, Size(5, 5)); // was 3

	// down-scale and upscale the image to filter out the noise
	//pyrDown(image, pyr, Size(image.cols / 2, image.rows / 2));
	//pyrUp(pyr, timg, image.size());

	// find squares in every color plane of the image
	// for (int c = 0; c < 3; c++)
	{
		//int ch[] = { c, 0 };
		// mixChannels(&timg, 1, &gray0, 1, ch, 1);
		cv::cvtColor(timg, gray0, CV_BGR2GRAY);
		cv::imwrite(path + "contours/" + filename + "-gray" + ".png", gray0);

		// try several threshold levels
		for (int l = -1; l < N; l++)
		{
			// hack: use Canny instead of zero threshold level.
			// Canny helps to catch squares with gradient shading
			if (l == 0)
			{
				//double otsu_threshold = cv::threshold(
				//	gray0, gray, 0, 4096, CV_THRESH_BINARY | CV_THRESH_OTSU
				//);
				//const double lower_threshold_fraction = 0.5;

				// apply Canny. Take the upper threshold from slider
				// and set the lower to 0 (which forces edges merging)
				// Canny(gray0, gray, otsu_threshold * lower_threshold_fraction, otsu_threshold, 5);  // was 0, 50, 5 -- best so far is 250, 1000
				Canny(gray0, gray, 250, 1000, 5);  // was 0, 50, 5 -- best so far is 250, 1000

				// dilate canny output to remove potential
				// holes between edge segments
				dilate(gray, gray, Mat(), Point(-1, -1));
				cv::imwrite(path + "contours/" + filename + "-canny" + ".png", gray);
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

			// Remove contours too small to be dice.
			double min_edge_length = double(min(image.size[0], image.size[1])) / 50;
			double min_area = min_edge_length * min_edge_length;
			contours = vfilter<vector<Point>>(contours, [min_area](vector<Point> contour) -> bool {
				auto area = cv::contourArea(contour, false);
				if (area < min_area) return false;
				
				auto rrect = cv::minAreaRect(contour);
				// Test if it's a rectangle by seeing if it occupies 75% are the are of its equiv min area
				double shorter_side = min(rrect.size.height, rrect.size.width);
				double longer_side = max(rrect.size.height, rrect.size.width);
				if (shorter_side < 0.7 * longer_side) {
					return false;
				}
				double square_area = longer_side * longer_side;
				if (area < square_area * 0.5)
					return false;

				return true;
			});


			//vector<double> arc_lengths = vmap<vector<Point>, double>(contours, [](vector<Point> contour) -> double {
			//	return cv::arcLength(contour, false);
			//});


			//double min_size = min_edge_length * 4;
			//// Filter out contours that aren't at least 100 pixels in length
			//arc_lengths = vfilter<double>(arc_lengths, [min_size](double arc_length) -> bool {
			//	return arc_length > min_size;
			//});

			//// Remove contours that have an arc length too far from the median arc length
			//auto median_arc_length = median(arc_lengths);
			//auto min_arc_length = median_arc_length * 0.5;
			//auto max_arc_length = median_arc_length * 1.5;
			//contours = vfilter<vector<Point>>(contours, [min_arc_length, max_arc_length](vector<Point> contour) -> bool {
			//	auto arc_length = cv::arcLength(contour, false);
			//	return (arc_length >= min_arc_length && arc_length <= max_arc_length);
			//});

			auto clone = image.clone();
			cv::drawContours(clone, contours, -1, Scalar(0, 255, 0), 3);

			cv::imwrite(path + "contours/" + filename + // "-" + std::to_string(c) + 
				"-" + std::to_string(l) + ".png", clone);

			// Convert contours to squares
			contours = vmap<vector<Point>, vector<Point>>(contours, [](vector<Point> contour) {
				Point2f points[4];
				auto rrect = cv::minAreaRect(contour);
				rrect.points(points);
				vector<Point> newContour = {
					Point(points[0]), Point(points[1]), Point(points[2]), Point(points[3]),
				};
				return newContour;
				});

			vector<Point> approx;
			// test each contour
			for (auto& contour : contours) {
				approx = contour;
				// approximate contour with accuracy proportional
				// to the contour perimeter
				// approxPolyDP(contour, approx, arcLength(contour, true) * 0.02, true);

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
	double minArea = 0.75 * medianArea;
	double maxArea = medianArea / 0.75;
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
