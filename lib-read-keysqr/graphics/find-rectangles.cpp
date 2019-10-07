//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#include <float.h>
#include <iostream>
#include "../utilities/vfunctional.h"
#include "cv.h"
#include "rectangle.h"
#include "find-rectangles.h"

std::vector<RectangleDetected> removeOverlappingRectangles(
	std::vector<RectangleDetected> rectangles,
	std::function<float(RectangleDetected)> comparatorLowerIsBetter
) {
	
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
std::vector<RectangleDetected> findRectangles(
	const cv::Mat &gray,
	uint N,
	double minPerimeter
) {
	std::vector<RectangleDetected> rectanglesFound;

	 //cv::Mat pyr, timg, gray0(image.size(), CV_8U), gray;
	 cv::Mat grayBlur, edges;

	//cv::blur(gray, grayBlur, gray.size().width > 2048 ? cv::Size(5, 5) : cv::Size(3,3)); // was 3
	cv::medianBlur(gray, grayBlur, 3); // was 3

	// try several threshold levels
	for (uint l = 0; l < N; l++)
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
		cv::findContours(edges, contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

		contours = vfilter<std::vector<cv::Point>>(contours, [minPerimeter](const std::vector<cv::Point> *contour) -> bool {
			return cv::arcLength(*contour, false) >= minPerimeter;
		});

		for (auto contour : contours) {
			rectanglesFound.push_back(RectangleDetected(contour, l == 0 ? 0 : (l + 1) * 255 / N));
		}
	}
	return rectanglesFound;
};
