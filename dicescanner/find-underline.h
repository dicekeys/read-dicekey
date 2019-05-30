#pragma once

#include <float.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include "vfunctional.h"
#include "rectangle.h"
#include "find-squares.h"
#include "value-clusters.h"
#include "rotate.h"
#include "distance.h"

#include <iostream>


//     Mat labels(src.size(), CV_32S);     
//     vector<vector<Point>> contours;
//     vector<Vec4i> hierarchy;            
//     findContours(binary, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);
//     blobs.clear();
//     blobs.reserve(contours.size());
//     int count = 1; //0 is background
//     for (int i = 0; i < contours.size(); i++) // iterate through each contour.
//     {
//         //if contour[i] is not a hole
//         if (hierarchy[i][3] == -1)
//         {                       
//             //draw contour without holes    
//             drawContours(labels, contours, i, Scalar(count),CV_FILLED, 0, hierarchy, 2, Point());
//             Rect rect = boundingRect(contours[i]);          
//             int left = rect.x;
//             int top = rect.y;
//             int width = rect.width;
//             int height = rect.height;           
//             int x_end = left + width;
//             int y_end = top + height;
//             vector<Point> blob;                 
//             blob.reserve(width*height);
//             for (size_t x = left; x < x_end; x++)
//             {
//                 for (size_t y = top; y < y_end; y++)
//                 {
//                     Point p(x, y);
//                     if (count == labels.at<int>(p))
//                     {
//                         blob.push_back(p);                      
//                     }
//                 }
//             }
//             blobs.push_back(blob);
//             count++;
//         }

//     }
//     count--;    
//     return count;
// }


static uint medianPointInRoundedRect(cv::Mat &image, cv::RotatedRect &rect) {
	auto bounds = rect.boundingRect();
	auto left = bounds.x;
	auto top = bounds.y;

	auto contourInBoundingBox = std::vector<cv::Point> {
		cv::Point( rect.points[0].x - left, rect.points[0].y - top ),
		cv::Point( rect.points[1].x - left, rect.points[1].y - top ),
		cv::Point( rect.points[2].x - left, rect.points[2].y - top ),
	};
	std::vector<std::vector<cv::Point>> contours = { contourInBoundingBox };

	cv::Mat mask(cv::Size(bounds.width, bounds.height), uchar(0));
	drawContours(mask, contours, 0, cv::Scalar(1), cv::FILLED, 0);

	std::vector<uint> pixelValuesInRect;
	for (int x = 0; x < bounds.width; x++) {		
		for (int y = 0; y < bounds.width; y++) {
			if (mask.at<uchar>(x, y) > 0) {
				pixelValuesInRect.push_back( image.at<uchar>(left + x, top + y ) );
			}
		}
	}

	return median(pixelValuesInRect);
}


static bool findUnderline(cv::Mat &image, RectangleDetected &closestUnderline, const float approxPixelsPerMm) {
	// Assume image is size of die (needs to be re-checked)
//	const float approxPixelsPerMm = ((image.size[0] + image.size[1]) / 2) / 8.0f;
	const float lineLengthMm = 5.5f;
	const float mmFromDieCenterToUnderlineCenter = 2.15f;
	const float maxMmFromDieCenterToUnderlineCenter = 2.0f * mmFromDieCenterToUnderlineCenter;

	// FUTURE -- use same threshold as die rectangle to save time?

	// const float maxDistanceDieCenterToUnderlineCenter = approxPixelsPerMm *
	// 	maxMmFromDieCenterToUnderlineCenter;

	bool anUnderlineWasFound = false;
	float expectedLengthInPixels = lineLengthMm * approxPixelsPerMm;
	float pixelsExpectedFromDieCenterToUnderlineCenter = mmFromDieCenterToUnderlineCenter * approxPixelsPerMm;
	float minLength = 0.85 * expectedLengthInPixels;
	float maxLength = 1.15 * expectedLengthInPixels;
	float smallestDeviation = INFINITY;
	auto dieCenter = cv::Point2f(((float)image.size[0]) / 2, ((float) image.size[1]) / 2);

	for (RectangleDetected rect : findRectangles(image)) {
		if (rect.longerSideLength < minLength || rect.longerSideLength > maxLength)
			continue;
		if (!isRectangleShapedLikelUnderline(rect))
			continue;

		auto distFromDieCenterToCandidateLine = distance2f(dieCenter, rect.center);
		auto fractionalDeviationFromExpectedDistanceToCenter = 
			((float) abs( distFromDieCenterToCandidateLine - pixelsExpectedFromDieCenterToUnderlineCenter)) /
			(float) pixelsExpectedFromDieCenterToUnderlineCenter;
		// A black line should have pixel value 0
		auto fractionalDeviationFromBlackLine = ((float) medianPointInRoundedRect(image, rect.rotatedRect)) / 256.0f;
		auto deviation = fractionalDeviationFromBlackLine + fractionalDeviationFromExpectedDistanceToCenter;

		
		// if (distFromDieCenterToCandidateLine > maxDistanceDieCenterToUnderlineCenter) {
		// 	// Not close enough to consider
		// 	continue;
		// }

		// FIXME -- reject candidate if line from die center to candidate center is not perpendicular (+- 25 degrees) with long edge of rectangle

		if (anUnderlineWasFound && deviation > smallestDeviation) {
			// A rectangle already found that deviates less from our location and color expectations
			continue;
		}

		// This is the current winning candidate
		anUnderlineWasFound = true;
		smallestDeviation = deviation;
		closestUnderline = rect;
	}

	return anUnderlineWasFound;
}
