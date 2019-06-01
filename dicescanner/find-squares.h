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

static float angleToMod90(float angle) {
	float angleMod90 = angle;
	while (angleMod90 > 90) {
		angleMod90 -= 90;
	}
	while (angleMod90 < 0) {
		angleMod90 += 90;
	}
	return angleMod90;
}


static std::vector<RectangleDetected> removeOverlappingRectangles(std::vector<RectangleDetected> rectangles, std::function<float(RectangleDetected)> comparatorLowerIsBetter) {
	
//	float targetArea, float targetAngle, float targetShortToLongSideRatio = 1) {
	std::vector<RectangleDetected> non_overlapping_rectangles;
	for (auto& rect : rectangles) {
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
			if (
				comparatorLowerIsBetter(rect) < comparatorLowerIsBetter(non_overlapping_rectangles[overlaps_with_index])
				//rect.deviationFromNorm(targetArea, targetAngle, targetShortToLongSideRatio) <
				//non_overlapping_rectangles[overlaps_with_index].deviationFromNorm(targetArea, targetAngle, targetShortToLongSideRatio)
				) {
				// Choose the rectangle with better quality
				non_overlapping_rectangles[overlaps_with_index] = rect;
			}
		}
	}

	return non_overlapping_rectangles;
}


// returns sequence of squares detected on the image.
static std::vector<RectangleDetected> findRectangles(const cv::Mat &gray, int N = 13)
{
	std::vector<RectangleDetected> rectanglesFound;

	/*
	Future:
	   Cluster areas by binary orders of magnitude.
		(two sets: a from 2^i - 2^(i+1), the other from 1.5 * 2^i - 1.5 * 2^(i+1) so that we don't have boundary issues)
	   Take largest cluster with n (20?  25?) objects
	     Only works at one threshold.  Can do more?
	*/
	std::vector<RectangleDetected> candidateDiceSquares;
	std::vector<RectangleDetected> candidateUnderlineRectangles;

	 //cv::Mat pyr, timg, gray0(image.size(), CV_8U), gray;
	 cv::Mat grayBlur, edges;

	//cv::blur(gray, grayBlur, gray.size().width > 2048 ? cv::Size(5, 5) : cv::Size(3,3)); // was 3
	cv::medianBlur(gray, grayBlur, 3); // was 3

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
			Canny(grayBlur, edges, 253, 255, 5);  // was 0, 50, 5 -- best so far is 253, 255, 5

			// dilate canny output to remove potential
			// holes between edge segments
			dilate(edges, edges, cv::Mat(), cv::Point(-1, -1));
			// cv::imwrite(path + "contours/" + filename + "-canny" + ".png", gray);
		}
		else
		{
			// apply threshold if l!=0:
			//     tgray(x,y) = gray(x,y) < (l+1)*255/N ? 255 : 0
			edges = gray >= (l + 1) * 255 / N;
		}

		// find contours and store them all as a list
		std::vector<std::vector<cv::Point>> contours;
		findContours(edges, contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

		contours = vfilter<std::vector<cv::Point>>(contours, [](std::vector<cv::Point> contour) -> bool {
			return cv::arcLength(contour, false) > 50;
		});

		for (auto contour : contours) {
			if (cv::arcLength(contour, false) > 50) {
				rectanglesFound.push_back(RectangleDetected(contour, l == 0 ? 0 : (l + 1) * 255 / N));
			}
		}
	}
	return rectanglesFound;
};

const float underline_length_mm = 5.5f; // 5.5mm
const float underline_width_mm = 0.3f; // 0.3 mm
const float underline_rect_short_to_long_ratio = underline_width_mm / underline_length_mm;
const float min_short_to_long_ratio = underline_rect_short_to_long_ratio / 4;
const float max_short_to_long_ratio = underline_rect_short_to_long_ratio * 3;


bool isRectangleShapedLikeUnderline(RectangleDetected rect) {
	float shortToLongRatio = rect.shorterSideLength / rect.longerSideLength;
	return (
		shortToLongRatio >= min_short_to_long_ratio &&
		shortToLongRatio <= max_short_to_long_ratio
	);
}


// returns sequence of squares detected on the image.
static std::vector<RectangleDetected> findCandidateDiceSquares(const cv::Mat &gray, int N = 13)
{
	/*
	Future:
	   Cluster areas by binary orders of magnitude.
		(two sets: a from 2^i - 2^(i+1), the other from 1.5 * 2^i - 1.5 * 2^(i+1) so that we don't have boundary issues)
	   Take largest cluster with n (20?  25?) objects
	     Only works at one threshold.  Can do more?
	*/
	
	 //cv::Mat pyr, timg, gray0(image.size(), CV_8U), gray;

	// auto clone = gray.clone();
	// for (uint i = 0; i < contours.size(); i++)
	// 	cv::drawContours(clone, contours, i, cv::Scalar((i * 13) % 255, (i * 41) % 255, (i * 87) % 255), 3);

	// cv::imwrite(path + "contours/" + filename + // "-" + std::to_string(c) + 
	// 	"-" + std::to_string(l) + ".png", clone);

	float min_edge_length = float(std::min(gray.size[0], gray.size[1])) / 50;
	float min_area = min_edge_length * min_edge_length;

	float min_underline_length = float(std::min(gray.size[0], gray.size[1])) / 80;
	float max_underline_length = float(std::min(gray.size[0], gray.size[1])) / 8;

	std::vector<RectangleDetected> candidateDiceSquares = vfilter<RectangleDetected>(
		findRectangles(gray, N),
		[min_area](RectangleDetected rect) -> bool { return
				// Rect area is above the minimum area to be considred a potential die
				(rect.area > min_area) &&
				// Rectangle is square enough
				(rect.shorterSideLength / rect.longerSideLength) >= 0.75f &&
				// the contour area is at least 60% of the area of the interpolated rectangle
				(rect.contourArea >= rect.area * 0.6);
		}
	);

	if (candidateDiceSquares.size() > 0) {
		// Remove rectangles that stray from the median

		float medianArea = median(vmap<RectangleDetected, float>(candidateDiceSquares, [](RectangleDetected r) -> float { return r.area; }));
		float minArea = 0.75f * medianArea;
		float maxArea = medianArea / 0.75f;
		candidateDiceSquares = vfilter<RectangleDetected>(candidateDiceSquares, [minArea, maxArea](RectangleDetected r) {
			return  (r.area >= minArea && r.area <= maxArea);
			});

		// Recalculate median for survivors
		float areaHighPercentile = percentile(
			vmap<RectangleDetected, float>(candidateDiceSquares, [](RectangleDetected r) -> float { return r.area; }),
			85
		);
		// Calculate slope of survivors
		float medianAngle = median(vmap<RectangleDetected, float>(candidateDiceSquares, [](RectangleDetected r) -> float { return r.angle; }));

		candidateDiceSquares = removeOverlappingRectangles(candidateDiceSquares, [areaHighPercentile, medianAngle](RectangleDetected r) -> float {
			return r.deviationFromNorm(areaHighPercentile, medianAngle, 1);
		});
	}

	return candidateDiceSquares;
}
	// if (candidateUnderlineRectangles.size() > 0) {
	// 	float medianLength = median(vmap<RectangleDetected, float>(candidateUnderlineRectangles, [](RectangleDetected r) -> float { return r.longerSideLength; }));
	// 	float minLength = medianLength * 0.85f;
	// 	float maxLength = medianLength * 1.1f;

	// 	float medianAngleMod90 = median(vmap<RectangleDetected, float>(candidateUnderlineRectangles, [](RectangleDetected r) -> float {
	// 		return angleToMod90(r.angle);
	// 	}));

	// 	// Remove rectangles that are outside the valid length range
	// 	candidateUnderlineRectangles = vfilter<RectangleDetected>(candidateUnderlineRectangles, [minLength, maxLength](RectangleDetected r) {
	// 		return  (r.longerSideLength >= minLength && r.longerSideLength <= maxLength);
	// 	});

	// 	// Remove overlapping underline rectangles, favoring those that better fit
	// 	// our model of underline length and the 4 valid angles (the angle mod 90)
	// 	candidateUnderlineRectangles = removeOverlappingRectangles(candidateUnderlineRectangles, [medianLength, medianAngleMod90](RectangleDetected r) -> float {
	// 		return abs((r.longerSideLength - medianLength) / medianLength) + abs(angleToMod90(r.angle) - medianAngleMod90) / 90;
	// 	});

	// }

	// result.candidateDiceSquares = candidateDiceSquares;
	// result.candidateUnderlineRectangles = candidateUnderlineRectangles;
	// return result;
// }
