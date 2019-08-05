#pragma once

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
#include "geometry.h"
#include "die-specification.h"
//#include "rotate.h"
//#include "sample-point.h"
#include "ocr.h"
#include "decode-die.h"
#include "find-undoverlines.h"
#include "value-clusters.h"




static std::vector<DieRead> readDice(cv::Mat colorImage, cv::Mat grayscaleImage, std::vector<RectangleDetected> candidateUndoverlineRects)
{
	FindDiceResult findDiceResult = findDice(colorImage, grayscaleImage, candidateUndoverlineRects);
	std::vector<DieRead>& diceFound = findDiceResult.diceFound;
	const float pixelsPerMm = findDiceResult.pixelsPerMm;
	const float halfDieSize = DieDimensionsMm::size * pixelsPerMm / 2.0f;

	std::vector<float> dieAnglesInRadians = vmap<DieRead, float>(diceFound,
		[](DieRead d) -> float { return normalizeAngleSignedRadians(d.inferredAngleInRadians); });
	// Get the angle of all dice
	float angleOfDiceInRadians = findPointOnCircularSignedNumberLineClosestToCenterOfMass(
		dieAnglesInRadians, FortyFiveDegreesAsRadians);


	for (auto &die : diceFound) {
		// Average the angle of the underline and overline
		const auto charsRead = readDieCharacters(colorImage, grayscaleImage, die.center, die.inferredAngleInRadians, findDiceResult.pixelsPerMm);
		const float orientationInRadians = die.inferredAngleInRadians - angleOfDiceInRadians;
		const float orientationInClockwiseRotationsFloat = orientationInRadians * float(4.0 / (2.0 * M_PI));
		const uchar orientationInClockwiseRotationsFromUpright = uchar(round(orientationInClockwiseRotationsFloat) + 4) % 4;
		die.orientationInClockswiseTurnsFromUpright = orientationInClockwiseRotationsFromUpright;
		die.ocrLetter = charsRead.letter;
		die.ocrDigit = charsRead.digit;
	}
	// calculate the average angle mod 90 so we can generate a rotation function
	for (size_t i = 0; i < diceFound.size(); i++) {
		diceFound[i].angleAdjustedCenter = rotatePointAroundOrigin(diceFound[i].center, angleOfDiceInRadians);
	}

	// Sort the dice based on their positions after adjusting the angle
	std::sort(diceFound.begin(), diceFound.end(), [halfDieSize](DieRead a, DieRead b) {
		if (a.angleAdjustedCenter.y < (b.angleAdjustedCenter.y - halfDieSize)) {
			// Die a is at least a half die above die b, and therefore comes before it
			return true;
		}
		else if (b.angleAdjustedCenter.y < (a.angleAdjustedCenter.y - halfDieSize)) {
			// Die b is at least a half die above die a, and therefore comes before it
			return false;
		}
		// Die a and die b are roughly the same from top to bottom, so order left to right
		return a.angleAdjustedCenter.x < b.angleAdjustedCenter.x;
	});

	// Search for missing dice
	if (diceFound.size() < 25) {
		// FIXME
		// Add function for this dirty work.
	}

	return diceFound;
}
