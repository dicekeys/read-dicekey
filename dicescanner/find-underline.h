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



static bool findUnderline(cv::Mat &image, RectangleDetected &closestUnderline) {
	// Assume image is size of die (needs to be re-checked)
	const float approxPixelsPerMm = ((image.size[0] + image.size[1]) / 2) / 8.0f;
	const float mmFromDieCenterToUnderlineCenter = 2.15f;
	const float maxMmFromDieCenterToUnderlineCenter = 2.0f * mmFromDieCenterToUnderlineCenter;

	// FUTURE -- use same threshold as die rectangle to save time?

	const float maxDistanceDieCenterToUnderlineCenter = approxPixelsPerMm *
		maxMmFromDieCenterToUnderlineCenter;

	bool anUnderlineWasFound = false;
	float minLength = image.size[0] / 2.2f;
	float closestDistance = INFINITY;
	auto dieCenter = cv::Point2f(((float)image.size[0]) / 2, ((float) image.size[1]) / 2);

	for (RectangleDetected rect : findRectangles(image)) {
		if (rect.longerSideLength < minLength)
			continue;
		if (!isRectangleShapedLikelUnderline(rect))
			continue;

		auto distFromDieCenterToCandidateLine = distance2f(dieCenter, rect.center);
		
		if (distFromDieCenterToCandidateLine > maxDistanceDieCenterToUnderlineCenter) {
			// Not close enough to consider
			continue;
		}

		// FIXME -- reject candidate if line from die center to candidate center is not perpendicular (+- 25 degrees) with long edge of rectangle

		if (anUnderlineWasFound && distFromDieCenterToCandidateLine > closestDistance) {
			// A rectangle already found is closer
			continue;
		}

		// This is the current winning candidate
		anUnderlineWasFound = true;
		closestDistance = distFromDieCenterToCandidateLine;
		closestUnderline = rect;
	}

	return anUnderlineWasFound;
}
