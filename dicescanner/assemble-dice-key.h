#pragma once

#include <float.h>
#include <math.h>
#include "vfunctional.h"
#include "statistics.h"
#include "geometry.h"
#include "die-specification.h"
#include "dice.h"
#include "ocr.h"
#include "find-undoverlines.h"
#include "find-dice.h"


class DiceKeyGridModel {
public:
	float valid = false;
	float distanceBetweenRows = 0;
	float distanceBetweenColumns = 0;
	float angleInRadians = NAN;
	cv::Point2f centerPoint = {0, 0};
	cv::Point2f topLeftPointRotatedClockwise = {0, 0};

	DiceKeyGridModel() {
	}

	DiceKeyGridModel(
		const float _distanceBetweenRows,
		const float _distanceBetweenColumns,
		const float _angleInRadians,
		const cv::Point2f _centerPoint
	) {
		valid = true;
		distanceBetweenRows = _distanceBetweenRows;
		distanceBetweenColumns = _distanceBetweenColumns;
		angleInRadians = _angleInRadians;
		centerPoint = _centerPoint;
		topLeftPointRotatedClockwise = cv::Point2f(
			centerPoint.x - 2 * distanceBetweenColumns,
			centerPoint.y - 2 * distanceBetweenRows
		);
	}

	/*
		Returns the index [0, 24] for a die center point, or -1 if the point provided
	 	Is not sufficiently close to a center of the grid in the DiceKey model
	*/
	const int getDieIndex(const cv::Point2f candidateDieCenter, const float maxFractionFromCenter = 0.1f) {
		const cv::Point2f rotatedPoint = rotatePointClockwise(candidateDieCenter, centerPoint, angleInRadians);
		const float xDistanceFromLeft = rotatedPoint.x - topLeftPointRotatedClockwise.x;
		const float yDistanceFromTop = rotatedPoint.y - topLeftPointRotatedClockwise.y;
		const float approxColumn = xDistanceFromLeft / distanceBetweenColumns;
		const float approxRow = yDistanceFromTop / distanceBetweenRows;
		if (approxColumn < -maxFractionFromCenter ||
				approxColumn > (4 + maxFractionFromCenter) ||
				approxRow < -maxFractionFromCenter ||
				approxRow > (4 + maxFractionFromCenter)
		) {
			// The approximate column/row is not within the grid (e.g., row -3 or 7 is not in range 0 - 4)
			return -1;
		}
		if (
			distanceInModCircularRangeFromNegativeNToN(0.0f, approxColumn, 0.5f) > maxFractionFromCenter ||
			distanceInModCircularRangeFromNegativeNToN(0.0f, approxRow, 0.5f) > maxFractionFromCenter
		) {
			// The approximate column/row is within grid, but not close enough to a die center
			// (e.g. column 1.4 does not approximate column 1 well enough)
			return -1;
		}
		const int column = (int) round(approxColumn);
		const int row = (int) round(approxRow);
		const int dieIndex = (row * 5) + column;
		return dieIndex;
	}
};

static DiceKeyGridModel calculateDiceKeyGrid(
	const DiceAndStrayUndoverlinesFound &diceAndStrayUndoverlinesFound,
	float maxMmFromRowOrColumnLine = 1.0f // 1 mm
) {
	const std::vector<DieRead> &diceFound = diceAndStrayUndoverlinesFound.diceFound;
	const std::vector<Undoverline> &strayUndoverlines = diceAndStrayUndoverlinesFound.strayUndoverlines;
	const	float maxPixelsFromRowOrColumnLine = maxMmFromRowOrColumnLine * diceAndStrayUndoverlinesFound.pixelsPerMm;

	for (int i = 0; i < diceFound.size(); i++) {
		// We can build a model of the grid based on this die if we can
		// find four others in the same row and four others in the same column.
		const DieRead &candidateIntersectionDie = diceFound[i];
		GridProximity gridModel(candidateIntersectionDie.center, candidateIntersectionDie.inferredAngleInRadians);
		
		std::vector<cv::Point2f> candidatePoints, sameColumn, sameRow;
		for (int j = 0; j < diceFound.size(); j++) {
			if (j!=i) candidatePoints.push_back(diceFound[j].center);
		}
		for (const Undoverline &undoverline: strayUndoverlines) {
			candidatePoints.push_back(undoverline.inferredDieCenter);
		}
		for (const cv::Point2f &point: candidatePoints) {
			const float distanceFromColumn = gridModel.pixelDistanceFromColumn(point);
			const float distanceFromRow = gridModel.pixelDistanceFromRow(point);
			if (distanceFromColumn <= maxPixelsFromRowOrColumnLine) {
				sameColumn.push_back(point);
			} else if (distanceFromRow <= maxPixelsFromRowOrColumnLine) {
				sameRow.push_back(point);
			}
		}
		if (sameRow.size() < 4 || sameColumn.size() < 4) {
			continue;
		}
		// Add this undoverline to both the row and the colum so we have all 5 of each
		sameRow.push_back(candidateIntersectionDie.center);
		sameColumn.push_back(candidateIntersectionDie.center);

		// Sort the elements in rows by x and in columns by y
		std::sort(sameRow.begin(), sameRow.end(), [](cv::Point2f a, cv::Point2f b) { return a.x < b.x; });
		std::sort(sameColumn.begin(), sameColumn.end(), [](cv::Point2f a, cv::Point2f b) { return a.y < b.y; });

		
		// Now check that our row has near-constant distances
		std::vector<float> xValuesOfRowsWithinColumn = vmap<cv::Point2f, float>(sameColumn, [](cv::Point2f p) { return p.x; });
		std::vector<float> yValuesOfRowsWithinColumn = vmap<cv::Point2f, float>(sameColumn, [](cv::Point2f p) { return p.y; });
		std::vector<float> xValuesOfColumnsWithinRow = vmap<cv::Point2f, float>(sameRow, [](cv::Point2f p) { return p.x; });
		std::vector<float> yValuesOfColumnsWithinRow = vmap<cv::Point2f, float>(sameRow, [](cv::Point2f p) { return p.y; });
		const auto meanXDistanceBetweenColumns = findAndValidateMeanDifference(xValuesOfColumnsWithinRow);
		const auto meanYDistanceBetweenColumns = findAndValidateMeanDifference(yValuesOfColumnsWithinRow);
		const auto meanXDistanceBetweenRows = findAndValidateMeanDifference(xValuesOfRowsWithinColumn);
		const auto meanYDistanceBetweenRows = findAndValidateMeanDifference(yValuesOfRowsWithinColumn);
		if (
			isnan(meanXDistanceBetweenColumns) || isnan(meanYDistanceBetweenColumns) ||
			isnan(meanXDistanceBetweenRows) || isnan(meanYDistanceBetweenRows)
		) {
			// Model is violated because the distances between rows or columns aren't consistent
			continue;
		}
		
		if ( abs(meanXDistanceBetweenColumns - meanYDistanceBetweenRows) > 5 ||
				abs(meanYDistanceBetweenColumns - meanXDistanceBetweenRows) > 5 ) {
			// If we assume square pixels, this violates model.
			// let's not for now.
		}
		
		// Figure out the row and column of the intersection die.
		// Since the arrayys of row Y values and column X values are sorted, we can do a linear serach.
		int rowOfIntersectionDie = 0, columnOfIntersectionDie = 0;
		while (rowOfIntersectionDie < yValuesOfRowsWithinColumn.size() && candidateIntersectionDie.center.y > yValuesOfRowsWithinColumn[rowOfIntersectionDie]) {
			rowOfIntersectionDie++;
		}
		while (columnOfIntersectionDie < xValuesOfColumnsWithinRow.size() && candidateIntersectionDie.center.x > xValuesOfColumnsWithinRow[columnOfIntersectionDie]) {
			columnOfIntersectionDie++;
		}
		
		// Find the center die's center point by moving to the third row
		// (row 2 with 0-based indexing) and third column (col 2).
		float centerX = candidateIntersectionDie.center.x
			+ (( 2 - rowOfIntersectionDie) * meanXDistanceBetweenRows)
			+ (( 2 - columnOfIntersectionDie) * meanXDistanceBetweenColumns);
		float centerY = candidateIntersectionDie.center.y
			+ (( 2 - rowOfIntersectionDie) * meanYDistanceBetweenRows)
			+ (( 2 - columnOfIntersectionDie) * meanYDistanceBetweenColumns);

		// Get the angle of the dice box as the angle of the row to which
		// the intersectio die belonged.
		const float angleInRadians = angleOfLineInSignedRadians2f({0, 0}, {meanXDistanceBetweenColumns, meanYDistanceBetweenColumns});
		// Average the distance between the dice in rows and columsn to get the mean distanc
		const float distanceBetweenRows = distance2f(meanXDistanceBetweenColumns, meanYDistanceBetweenColumns);
		const float distanceBetweenColumns = distance2f(meanXDistanceBetweenRows, meanYDistanceBetweenRows);

		return DiceKeyGridModel(
			distanceBetweenRows,
			distanceBetweenColumns,
			angleInRadians,
			cv::Point2f(centerX, centerY)
		);
	}
	// Made it to end without finding a grid.  Return invalid.
	return DiceKeyGridModel();
}

struct DiceOrderdWithMissingDiceInferredFromUnderlines {
	bool valid = false;
	std::vector<DieRead> orderedDice = {};
	// The angle of what was read on the page, without any conversion to have
	// the top left be the corner with the earliest letter in the alphabet
	float angleInRadiansNonCononicalForm = NAN;
};
static DiceOrderdWithMissingDiceInferredFromUnderlines orderDiceAndInferMissingUndoverlines(
	const DiceAndStrayUndoverlinesFound& diceAndStrayUndoverlinesFound,
	float maxMmFromRowOrColumnLine = 1.0f // 1 mm
) {
	auto grid = calculateDiceKeyGrid(diceAndStrayUndoverlinesFound, maxMmFromRowOrColumnLine);
	if (!grid.valid) {
		return {};
	}
	std::vector<DieRead> orderedDice(25);
	for (const auto dieFound : diceAndStrayUndoverlinesFound.diceFound) {
		const int dieIndex = grid.getDieIndex(dieFound.center);
		if (dieIndex >= 0) {
			orderedDice[dieIndex] = dieFound;
		}
	}
	for (const auto undoverline : diceAndStrayUndoverlinesFound.strayUndoverlines) {
		const int dieIndex = grid.getDieIndex(undoverline.inferredDieCenter);
		if (dieIndex >= 0) {
			if (undoverline.isOverline) {
				orderedDice[dieIndex].overline = undoverline;
			}
			else {
				orderedDice[dieIndex].underline = undoverline;
			}
			orderedDice[dieIndex].inferredAngleInRadians = angleOfLineInSignedRadians2f(undoverline.line);
			orderedDice[dieIndex].center = undoverline.inferredDieCenter;
		}
	}
	return { true, orderedDice, grid.angleInRadians };
}