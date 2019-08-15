#pragma once

#include <float.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include <iostream>
#include <math.h>
#include "vfunctional.h"
#include "statistics.h"
#include "geometry.h"
#include "die-specification.h"
#include "dice.h"
#include "ocr.h"
#include "decode-die.h"
#include "find-undoverlines.h"
#include "value-clusters.h"
#include "bit-operations.h"

struct DiceAndStrayUndoverlinesFound {
	std::vector<DieRead> diceFound;
	std::vector<Undoverline> strayUndoverlines;
	float pixelsPerMm;
};

static DiceAndStrayUndoverlinesFound findDiceAndStrayUndoverlines(const cv::Mat &colorImage, const cv::Mat &grayscaleImage)
{
	const auto undoverlines = findReadableUndoverlines(colorImage, grayscaleImage);

	std::vector<Undoverline> underlines(undoverlines.underlines);
	std::vector<Undoverline> overlines(undoverlines.overlines);

	std::vector<float> underlineLengths = vmap<Undoverline, float>(underlines,
		[](Undoverline underline) { return lineLength(underline.line); });
	const float medianUnderlineLength = medianInPlace(underlineLengths);
	const float pixelsPerMm = medianUnderlineLength / DieDimensionsMm::undoverlineLength;
	const float maxDistanceBetweenInferredCenters = 2 * pixelsPerMm; // 2mm

	std::vector<Undoverline> strayUndoverlines(0);
	std::vector<DieRead> diceFound;

	for (auto underline : underlines) {
		// Search for overline with inferred die center near that of underline.
		bool found = false;
		for (size_t i = 0; i < overlines.size() && !found; i++) {
			if (distance2f(underline.inferredDieCenter, overlines[i].inferredDieCenter) <= maxDistanceBetweenInferredCenters) {
				// We have a match
				found = true;
				// Re-infer the center of the die and its angle by drawing a line from
				// the center of the to the center of the overline.
				const Line lineFromUnderlineCenterToOverlineCenter = {
					midpointOfLine(underline.line), midpointOfLine(overlines[i].line)
				};
				// The center of the die is the midpoint of that line.
				const cv::Point2f center = midpointOfLine(lineFromUnderlineCenterToOverlineCenter);
				// The angle of the die is the angle of that line, plus 90 degrees clockwise
				const float angleOfLineFromUnderlineToOverlineCenterInRadians =
					angleOfLineInSignedRadians2f(lineFromUnderlineCenterToOverlineCenter);
				float angleInRadians = angleOfLineFromUnderlineToOverlineCenterInRadians +
					NinetyDegreesAsRadians;
				if (angleInRadians > (M_PI)) {
					angleInRadians -= float(2 * M_PI);
				}
				const cv::Point2f angleAdjustedCenter = rotatePointClockwiseAroundOrigin(center, radiansFromRightAngle(angleInRadians));
				diceFound.push_back({
					underline, overlines[i], center, angleInRadians,
					// letter read (not yet set)
					{0, 0},
					// digit read (not yet set)
					{0,0},
					0
					});
				// Remove the ith element of overlines
				overlines.erase(overlines.begin() + i);
			}
		}
		if (!found) {
			strayUndoverlines.push_back(underline);
		}
	}

	strayUndoverlines.insert(strayUndoverlines.end(), overlines.begin(), overlines.end());

	return { diceFound, strayUndoverlines, pixelsPerMm };
}
