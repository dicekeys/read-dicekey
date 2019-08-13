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
#include "dice.h"
#include "ocr.h"
#include "decode-die.h"
#include "find-undoverlines.h"
#include "value-clusters.h"



struct DieRead {
	Undoverline underline;
	Undoverline overline;
	cv::Point2f center = cv::Point2f{ 0, 0 };
	float inferredAngleInRadians = 0;
	cv::Point2f angleAdjustedCenter{ 0, 0 };
	ReadCharacterResult ocrLetter = { 0, 0 };
	ReadCharacterResult ocrDigit = { 0, 0 };
	unsigned char orientationAs0to3ClockwiseTurnsFromUpright;
};

struct MeanDifferenceResult
{
	bool valid;
	float meanDistance;
};

static MeanDifferenceResult findAndValidateMeanDifference(const std::vector<float> unsortedValues, float minBoundEdgeRange = 5.0f)
{
	if (unsortedValues.size() < 2) {
		return {false, 0};
	}
	std::vector<float> sorted(unsortedValues);
	std::sort(sorted.begin(), sorted.end(), [](float a, float b) { return abs(a) < abs(b); });
	const float mean_difference = (sorted[sorted.size() -1] - sorted[0]) / float(sorted.size() - 1);
	const float abs_mean_difference = abs(mean_difference);
		// Ensure all delta_x and delta_y values are within 5% of the mean, though always
		// allow up to a minimum error since vertical/horizontal lines will have no delta_x/delta_y,
		// but could have a few pixel variation due to measurement errors.
		const float mean_bound_low = MIN(abs_mean_difference - minBoundEdgeRange, abs_mean_difference * 0.95);
		const float mean_bound_high = MAX(abs_mean_difference + minBoundEdgeRange, abs_mean_difference * 1.05);
		// Ensure all the delta_x and delta_y values are close to the mean
		bool allDistancesAreCloseToTheMeanDistance = true;
		for (int d = 1; d < sorted.size() && allDistancesAreCloseToTheMeanDistance; d++) {
			float difference = abs(sorted[d] - sorted[d-1]);
			float abs_difference = abs(difference);
			allDistancesAreCloseToTheMeanDistance &= 
				(mean_bound_low < abs_mean_difference) && (abs_mean_difference < mean_bound_high);
		}
		return {allDistancesAreCloseToTheMeanDistance, mean_difference};
}

struct DiceGrid {
	bool success = false;
	float rowDeltaX = 0, rowDeltaY = 0;
	float colDeltaX = 0, colDeltaY = 0;
	cv::Point2f gridTopLeft = {0, 0};
};

static DiceGrid findDiceGrid(std::vector<Undoverline> lines, float pixelPromityRequirement) {
	for (int i = 0; i < lines.size(); i++) {
		// We can build a model of the grid based on this die if we can
		// find four others in the same row and four others in the same column.
		Undoverline &undoverline = lines[i];
		GridProximity gridModel(undoverline.inferredDieCenter, undoverline.line);
		std::vector<Undoverline&> sameRow = {}, sameColumn;
		for (int j = 0; j < lines.size() && (sameRow.size() < 4 || sameColumn.size() < 4); j++) {
			if (i==j) {
				continue;
			}
			Undoverline &candidateUndoverline = lines[j];
			if (gridModel.pixelDistanceFromColumn(candidateUndoverline.inferredDieCenter) < pixelPromityRequirement) {
				sameColumn.push_back(candidateUndoverline);
			} else if (gridModel.pixelDistanceFromRow(candidateUndoverline.inferredDieCenter) < pixelPromityRequirement) {
				sameRow.push_back(candidateUndoverline);
			}
		}
		if (sameRow.size() < 4 and sameColumn.size() < 4) {
			continue;
		}
		// Add this undoverline to both the row and the colum so we have all 5 of each
		sameRow.push_back(undoverline);
		sameColumn.push_back(undoverline);

		// Now check that our row has near-constant distances
		std::vector<float> rowXValues, rowYValues, colXValues, colYValues;
		for (const Undoverline u: sameRow) {
			rowXValues.push_back(u.inferredDieCenter.x);
			rowYValues.push_back(u.inferredDieCenter.y);
		}
		for (const Undoverline u: sameColumn) {
			colXValues.push_back(u.inferredDieCenter.x);
			colYValues.push_back(u.inferredDieCenter.y);
		}
		const auto meanRowXDistance = findAndValidateMeanDifference(rowXValues);
		const auto meanRowYDistance = findAndValidateMeanDifference(rowYValues);
		const auto meanColXDistance = findAndValidateMeanDifference(colXValues);
		const auto meanColYDistance = findAndValidateMeanDifference(colYValues);
		if (!( meanRowXDistance.valid && meanRowYDistance.valid && meanColXDistance.valid && meanColYDistance.valid)) {
			// Model is violated
			continue;
		}

		if ( abs(meanRowXDistance.meanDistance - meanColYDistance.meanDistance) > 5 ||
		 		 abs(meanRowYDistance.meanDistance - meanColXDistance.meanDistance) > 5 ) {
			// If we assume square pixels, this violates model.
			// let's not for now.
		}

		// Figure out which index this die is at.
		int row = 0, col = 0;
		for (float x: rowXValues) {
			if (x < undoverline.inferredDieCenter.x) {
				row++;
			}
		}
		for (float y: colYValues) {
			if (y < undoverline.inferredDieCenter.y) {
				col++;
			}
		}

		cv::Point2f gridTopLeft(
			undoverline.inferredDieCenter.x - (row * meanRowXDistance.meanDistance) - (col * meanColXDistance.meanDistance),
			undoverline.inferredDieCenter.y - (row * meanRowYDistance.meanDistance) - (col * meanColYDistance.meanDistance)
		);

		return {
			true, // valid
			meanRowXDistance.meanDistance, meanRowYDistance.meanDistance,
			meanColXDistance.meanDistance, meanColYDistance.meanDistance,
			gridTopLeft
		};
	}
	// Made it to end without finding a grid.  Return invalid.
	return {false};
}

struct FindDiceResult {
	std::vector<DieRead> diceFound;
	std::vector<Undoverline> strayUndoverlines;
	float pixelsPerMm;
};

static FindDiceResult findDice(const cv::Mat &colorImage, const cv::Mat &grayscaleImage)
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
					underline, overlines[i], center, angleInRadians, angleAdjustedCenter,
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


static std::vector<DieRead> readDice(const cv::Mat &colorImage, bool outputOcrErrors = false)
{
	cv::Mat grayscaleImage;

	cv::cvtColor(colorImage, grayscaleImage, cv::COLOR_BGR2GRAY);
	FindDiceResult findDiceResult = findDice(colorImage, grayscaleImage);
	std::vector<DieRead>& diceFound = findDiceResult.diceFound;
	const float pixelsPerMm = findDiceResult.pixelsPerMm;
	const float halfDieSize = DieDimensionsMm::size * pixelsPerMm / 2.0f;

	std::vector<float> dieAnglesInRadians = vmap<DieRead, float>(diceFound,
		[](DieRead d) -> float { return radiansFromRightAngle(d.inferredAngleInRadians); });
	// Get the angle of all dice
	float angleOfDiceInRadians = findPointOnCircularSignedNumberLineClosestToCenterOfMass(
		dieAnglesInRadians, FortyFiveDegreesAsRadians);


	for (auto &die : diceFound) {
		// Average the angle of the underline and overline
		const auto charsRead = readDieCharacters(colorImage, grayscaleImage, die.center, die.inferredAngleInRadians,
			findDiceResult.pixelsPerMm,
			// The threshold between black pixels and white pixels is calculated as the average (mean)
			// of the threshold used for the underline and for the overline.
			uchar( (uint(die.underline.whiteBlackThreshold) + uint(die.overline.whiteBlackThreshold))/2 ),
			outputOcrErrors ? die.underline.dieFaceInferred.letter : '\0',
			outputOcrErrors ? die.underline.dieFaceInferred.digit : '\0'
		);
		const float orientationInRadians = die.inferredAngleInRadians - angleOfDiceInRadians;
		const float orientationInClockwiseRotationsFloat = orientationInRadians * float(4.0 / (2.0 * M_PI));
		const uchar orientationInClockwiseRotationsFromUpright = uchar(round(orientationInClockwiseRotationsFloat) + 4) % 4;
		die.orientationAs0to3ClockwiseTurnsFromUpright = orientationInClockwiseRotationsFromUpright;
		die.ocrLetter = charsRead.letter;
		die.ocrDigit = charsRead.digit;
	}
	// calculate the average angle mod 90 so we can generate a rotation function
	for (size_t i = 0; i < diceFound.size(); i++) {
		diceFound[i].angleAdjustedCenter = rotatePointClockwiseAroundOrigin(diceFound[i].center, angleOfDiceInRadians);
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
		// FUTURE -- search for stray underlines/overlines at locations where
		// missing dice should be
	}

	return diceFound;
}

static std::vector<DieFace> diceReadToDiceKey(const std::vector<DieRead> diceRead, bool reportErrsToStdErr = false)
{
	if (diceRead.size() != 25) {
		throw std::string("A DiceKey must contain 25 dice but only has " + std::to_string(diceRead.size()));
	}
	std::vector<DieFace> diceKey;
	for (size_t i = 0; i < diceRead.size(); i++) {
		DieRead dieRead = diceRead[i];
		const DieFaceSpecification& underlineInferred = dieRead.underline.dieFaceInferred;
		const DieFaceSpecification& overlineInferred = dieRead.overline.dieFaceInferred;
		const char digitRead = dieRead.ocrDigit.charRead;
		const char letterRead = dieRead.ocrLetter.charRead;
		if (underlineInferred.letter == 0 ||
			underlineInferred.digit == 0 ||
			underlineInferred.letter != overlineInferred.letter ||
			underlineInferred.digit != overlineInferred.digit) {
			const int bitErrorsIfUnderlineCorrect = hammingDistance(dieRead.underline.dieFaceInferred.overlineCode, dieRead.overline.decoded.letterDigitEncoding);
			const int bitErrorsIfOverlineCorrect = hammingDistance(dieRead.overline.dieFaceInferred.underlineCode, dieRead.underline.decoded.letterDigitEncoding);
			const int minBitErrors = MIN(bitErrorsIfUnderlineCorrect, bitErrorsIfOverlineCorrect);
			// See if this error can be explained by a single bit-read error.
			// report error mismatch between undoverline and overline
			if (reportErrsToStdErr) {
				std::cerr << "Mismatch at die " << i << " between underline and overline: " <<
					dashIfNull(underlineInferred.letter) << dashIfNull(underlineInferred.digit) << " != " <<
					dashIfNull(overlineInferred.letter) << dashIfNull(overlineInferred.digit) <<
					" best explained by " << minBitErrors << " bit error in " << 
						(bitErrorsIfUnderlineCorrect < bitErrorsIfOverlineCorrect ? "overline" : "underline") <<
					" (ocr returned " << dashIfNull(dieRead.ocrLetter.charRead) << dashIfNull(dieRead.ocrDigit.charRead) << ")" <<
					"\n";
			}
		} else if (underlineInferred.letter != letterRead) {
			// report OCR error on letter
			if (reportErrsToStdErr) {
				std::cerr << "Mismatch at die " << i << " between underline and ocr letter: " <<
					dashIfNull(underlineInferred.letter) << " != " << dashIfNull(letterRead) << "\n";
			}
		} else if (underlineInferred.digit != digitRead) {
			// report OCR error on digit
			if (reportErrsToStdErr) {
				std::cerr << "Mismatch at die " << i << " between underline and ocr digit: " <<
					dashIfNull(underlineInferred.digit) << " != " << dashIfNull(digitRead) << "\n";
			}
		}

		diceKey.push_back(DieFace({
			majorityOfThree(
				underlineInferred.letter, overlineInferred.letter, dieRead.ocrLetter.charRead
			),
			majorityOfThree(
				underlineInferred.digit, overlineInferred.digit, dieRead.ocrDigit.charRead
			),
			dieRead.orientationAs0to3ClockwiseTurnsFromUpright
			}));
	}
	return diceKey;
}
