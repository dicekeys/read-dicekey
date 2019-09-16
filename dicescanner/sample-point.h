//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#pragma once

#include <float.h>
#include <math.h>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include "vfunctional.h"

const std::vector<cv::Point> SampleOffsetsHorizontalFirst = {
	cv::Point(0, 0),
	// 1
	cv::Point(+1, 0),
	cv::Point(-1, 0),
	cv::Point(0, +1),
	cv::Point(0, -1),
	// 5
	cv::Point(+1, +1),
	cv::Point(-1, -1),
	cv::Point(+1, -1),
	cv::Point(-1, +1),
	// 9
	cv::Point(+2, 0),
	cv::Point(-2, 0),
	cv::Point(0, +2),
	cv::Point(0, -2),
	// 13
	cv::Point(+2, -1),
	cv::Point(-2, -1),
	cv::Point(-1, +2),
	cv::Point(-1, -2),
	cv::Point(+2, +1),
	cv::Point(-2, +1),
	cv::Point(+1, +2),
	cv::Point(+1, -2),
	// 21
};

const std::vector<cv::Point> SampleOffsetsVerticalFirst = {
	cv::Point(0, 0),
	// 1
	cv::Point(0, +1),
	cv::Point(0, -1),
	// 3
};

/*
Return the suggested number of samples to collect around a point when sampling
a point.

Takes as input the expected width of the area being read.  For example, for a circle
of radius 4 or square of size 4, pass 4.0f.
*/
static size_t getNumberOfPixelsToSample(float physicalPixelWidthPerLogicalPixelWidth) {
	return
		physicalPixelWidthPerLogicalPixelWidth < 2.0f ? 1 :
		physicalPixelWidthPerLogicalPixelWidth < 3.0f ? 3 :
		physicalPixelWidthPerLogicalPixelWidth < 4.0f ? 5 :
		physicalPixelWidthPerLogicalPixelWidth < 5.0 ? 9 :
		physicalPixelWidthPerLogicalPixelWidth < 5.5 ? 13 :
		21;
}

/*
Sample the point(s) at and around a position in a matrix to get the median
pixel darkness at that location.

Requires an OpenCV image matrix with only a single, 8-bit value at each location.

The third parameter, the number of points to sample, can be obtained via a call to
getNumberOfPixelsToSample.
*/
static uchar samplePoint(
	const cv::Mat &grayscaleImage,
	const cv::Point2i point,
	size_t samplesPerPoint = SampleOffsetsHorizontalFirst.size(),
	const bool favorAboveAndBelowOverSides = false
) {
	const std::vector<cv::Point>& SampleOffsets = (samplesPerPoint == 3 && favorAboveAndBelowOverSides) ?
		SampleOffsetsVerticalFirst : SampleOffsetsHorizontalFirst;
	samplesPerPoint = std::min(samplesPerPoint, SampleOffsets.size());
	std::vector<uchar> pixelsAroundSamplePoint = std::vector<uchar>();
	for (size_t s = 0; s < samplesPerPoint; s++) {
		cv::Point2i samplePoint = cv::Point2i(point.x + SampleOffsets[s].x, point.y + SampleOffsets[s].y);
		if (samplePoint.x < 0 || samplePoint.y < 0 || samplePoint.x >= grayscaleImage.cols || samplePoint.y >= grayscaleImage.rows) {
			// Sample is outside valid image region, so don't collect this sample.
			continue;
		}
		pixelsAroundSamplePoint.push_back(grayscaleImage.at<uchar>(samplePoint));
	}
	if (pixelsAroundSamplePoint.size() == 0) {
		// Just in case all samples were outside image.
		return 0;
	}
	return medianInPlace(pixelsAroundSamplePoint);
}

/*
Read samples of points that are space along a line.

The first parameter is the image, which should ge grayscale with values 0-255.

The 2nd and 3rd parameters specified the line, from start to end.

The fourth parameters specifies the location of the points, as fractions of the
distance from start to end.

The final, optional, parmeters specifies the number of pixels to sample
at each point, which can be obtained using getNumberOfPixelsToSample().
*/
static std::vector<uchar> samplePointsAlongLine(
	const cv::Mat &grayscaleImage,
	const cv::Point2f start,
	const cv::Point2f end,
	const std::vector<float> pointsAsFractionsOfDistanceFromStartToEnd,
	size_t samplesPerPoint = 0) {
	if (samplesPerPoint <= 0) {
		samplesPerPoint = getNumberOfPixelsToSample(distance2f(start, end) / pointsAsFractionsOfDistanceFromStartToEnd.size());
	}
	const float deltaX = end.x - start.x;
	const float deltaY = end.y - start.y;
	const bool isMoreHorizontalThanVertical = abs(deltaY) < abs(deltaX);
	const bool ifSamplingThreePointsUseThoseAboveAndBelowRatherThanThoseOnSides = isMoreHorizontalThanVertical;
	return vmap<float, uchar>(
		pointsAsFractionsOfDistanceFromStartToEnd,
		[grayscaleImage, start, deltaX, deltaY, samplesPerPoint,
		 ifSamplingThreePointsUseThoseAboveAndBelowRatherThanThoseOnSides
		](const float* dotFraction) {
			return samplePoint(
				grayscaleImage,
				cv::Point2i(
					int(round(start.x + ((*dotFraction) * deltaX))),
					int(round(start.y + ((*dotFraction) * deltaY)))
				),
				samplesPerPoint,
				ifSamplingThreePointsUseThoseAboveAndBelowRatherThanThoseOnSides
			);
		}
	);
}

/*
Convert anvector of sampled points, stored as unsigned bytes represented scales of gray,
into bits where the most-significant bit in the number is the first vector in the vector
and the least-significant is the last entry in the vector.

Samples strictly above the treshold are one bits and those at or below the treshold are zero bits.
*/
static unsigned int sampledPointsToBits(std::vector<uchar> sampledPoints, uchar thresholdAboveWhichPointIsOneBit)
{
	unsigned int resultBits = 0;
	for (size_t i = 0; i < sampledPoints.size(); i++) {
		resultBits <<= 1;
		if (sampledPoints[i] > thresholdAboveWhichPointIsOneBit) {
			resultBits += 1;
		}
	}
	return resultBits;
}
