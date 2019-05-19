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
	vector<Rectangle> candidateDiceSquaresForAllThresholds;

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
		for (int l = 0; l < N; l++)
		{
			// hack: use Canny instead of zero threshold level.
			// Canny helps to catch squares with gradient shading
			if (l == 0)
			{
				//float otsu_threshold = cv::threshold(
				//	gray0, gray, 0, 4096, CV_THRESH_BINARY | CV_THRESH_OTSU
				//);
				//const float lower_threshold_fraction = 0.5;

				// apply Canny. Take the upper threshold from slider
				// and set the lower to 0 (which forces edges merging)
				// Canny(gray0, gray, otsu_threshold * lower_threshold_fraction, otsu_threshold, 5);  // was 0, 50, 5 -- best so far is 250, 1000
				Canny(gray0, gray, 253, 255, 5);  // was 0, 50, 5 -- best so far is 253, 255, 5

				// dilate canny output to remove potential
				// holes between edge segments
				dilate(gray, gray, Mat(), Point(-1, -1));
				// cv::imwrite(path + "contours/" + filename + "-canny" + ".png", gray);
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

			contours = vfilter<vector<Point>>(contours, [](vector<Point> contour) -> bool {
				return arcLength(contour, false) > 50;
			});

			auto clone = image.clone();
			for (uint i = 0; i < contours.size(); i++)
				cv::drawContours(clone, contours, i, Scalar((i * 13) % 255, (i * 41) % 255, (i * 87) % 255), 3);

			cv::imwrite(path + "contours/" + filename + // "-" + std::to_string(c) + 
				"-" + std::to_string(l) + ".png", clone);


			auto contourRectangles = vmap<vector<Point>, Rectangle>(contours, [](vector<Point> contour) -> Rectangle {
				return Rectangle(contour);
			});

			// Remove contours too small to be dice.
			float min_edge_length = float(min(image.size[0], image.size[1])) / 50;
			float min_area = min_edge_length * min_edge_length;
			auto candidateDiceSquares = vfilter<Rectangle>(contourRectangles, [min_area](Rectangle rect) -> bool {
				if (rect.area < min_area) return false;

				// Test if it's a rectangle by seeing if it occupies 75% are the are of its equiv min area
				if (rect.shorterSideLength < 0.75f * rect.longerSideLength) {
					return false;
				}
				if (rect.contourArea < rect.area * 0.6)
					return false;

				return true;
				});
			// test each contour
			candidateDiceSquaresForAllThresholds.insert(candidateDiceSquaresForAllThresholds.end(), candidateDiceSquares.begin(), candidateDiceSquares.end());

			
			// Remove contours except those that appear to be underlines
			float min_underline_length = float(min(image.size[0], image.size[1])) / 80;
			float max_underline_length = float(min(image.size[0], image.size[1])) / 8;
			auto candidateUnderlines = vfilter<Rectangle>(contourRectangles, [min_underline_length, max_underline_length](Rectangle rect) -> bool {
				const float underline_length_mm = 5.5f; // 5.5mm
				const float underline_width_mm = 0.3f; // 0.3 mm
				const float underline_rect_ratio = underline_length_mm / underline_width_mm;
				const float min_ratio = underline_rect_ratio / 2;
				const float max_ratio = underline_rect_ratio * 2;

				// Test if it's a rectangle by seeing if it occupies 75% are the are of its equiv min area
				float ratio = rect.longerSideLength / rect.shorterSideLength;
					
				return rect.longerSideLength > min_underline_length &&
					rect.longerSideLength < max_underline_length &&
					ratio >= min_ratio &&
					ratio <= max_ratio;
				});

		}
	}
	if (candidateDiceSquaresForAllThresholds.size() <= 0) {
		// There are not enough squares, so return the empty vector
		return candidateDiceSquaresForAllThresholds;
	}

	// Remove rectangles that stray from the median
	float medianArea = median(vmap<Rectangle, float>(candidateDiceSquaresForAllThresholds, [](Rectangle r) -> float { return r.area; }));
	float minArea = 0.75f * medianArea;
	float maxArea = medianArea / 0.75f;
	candidateDiceSquaresForAllThresholds = vfilter<Rectangle>(candidateDiceSquaresForAllThresholds, [minArea, maxArea](Rectangle r) {
		return  (r.area >= minArea && r.area <= maxArea);
		});
	
	// Recalculate median for survivors
	float areaHighPercentile = percentile(
		vmap<Rectangle, float>(candidateDiceSquaresForAllThresholds, [](Rectangle r) -> float { return r.area; }),
		85
	);
	// Calculate slop of survivors
	float medianAngle = median(vmap<Rectangle, float>(candidateDiceSquaresForAllThresholds, [](Rectangle r) -> float { return r.angle; }));


	// remove overlapping rectangeles
	vector<Rectangle> non_overlapping_rectangles;
	for (auto& rect : candidateDiceSquaresForAllThresholds) {
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
			// This rectangle is different from the other rectangles.
			// Choose the rectangle the deviates less from the norm.
			if ( rect.deviationFromNorm(areaHighPercentile, medianAngle) < non_overlapping_rectangles[overlaps_with_index].deviationFromNorm(areaHighPercentile, medianAngle) ) {
				// Choose the rectangle with better quality
				non_overlapping_rectangles[overlaps_with_index] = rect;
			}
		}
	}

	return non_overlapping_rectangles;
}
