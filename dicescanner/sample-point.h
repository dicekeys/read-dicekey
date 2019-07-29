#pragma once

#include <float.h>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include "vfunctional.h"

const	std::vector<cv::Point>	SampleOffsets = {
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

static uchar samplePoint(
	const cv::Mat image,
	const cv::Point2i point,
	size_t samplesPerPoint = SampleOffsets.size()
) {
	samplesPerPoint = MIN(samplesPerPoint, SampleOffsets.size());
	std::vector<uchar> pixelsAroundSamplePoint = std::vector<uchar>(samplesPerPoint);
	for (size_t s = 0; s < samplesPerPoint; s++) {
		cv::Point2i samplePoint = cv::Point2i(point.x + SampleOffsets[s].x, point.y + SampleOffsets[s].y);
		if (samplePoint.x < 0 || samplePoint.y < 0 || samplePoint.x >= image.cols || samplePoint.y >= image.rows) {
			// Sample is outside valid image region, so don't collect this sample.
			continue;
		}
		pixelsAroundSamplePoint.push_back(image.at<uchar>(samplePoint));
	}
	if (pixelsAroundSamplePoint.size() == 0) {
		// Just in case all samples were outside image.
		return 0;
	}
	return medianInPlace(pixelsAroundSamplePoint);
}

static std::vector<uchar> samplePointsAlongLine(
	const cv::Mat image,
	const cv::Point2f start,
	const cv::Point2f end,
	const std::vector<float> pointsAsFractionsOfDistanceFromStartToEnd,
	const size_t samplesPerPoint = SampleOffsets.size()
) {
	std::vector<uchar> samplesRead(pointsAsFractionsOfDistanceFromStartToEnd.size());
	float deltaX = end.x - start.x;
	float deltaY = end.y - start.y;
	return vmap<float, uchar>(
		pointsAsFractionsOfDistanceFromStartToEnd,
		[image, start, deltaX, deltaY, samplesPerPoint](float dotFraction) {
			return samplePoint(
				image,
				cv::Point2i(
					int(round(start.x + (dotFraction * deltaX))),
					int(round(start.y + (dotFraction * deltaY)))
				),
				samplesPerPoint
			);
		}
	);
}