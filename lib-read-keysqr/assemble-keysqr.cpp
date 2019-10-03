//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include <float.h>
#include <math.h>
#include "utilities/vfunctional.h"
#include "utilities/statistics.h"
#include "graphics/geometry.h"
#include "graphics/draw-rotated-rect.h"
#include "keysqr-element-face-specification.h"
#include "keysqr.h"
#include "find-undoverlines.h"
#include "find-faces.h"
#include "assemble-keysqr.h"


class KeySqrGridModel {
public:
	float valid = false;
	float distanceBetweenRows = 0;
	float distanceBetweenColumns = 0;
	float angleInRadians = NAN;
	cv::Point2f centerPoint = {0, 0};
	cv::Point2f topLeftPointRotatedClockwise = {0, 0};

	KeySqrGridModel() {
	}

	KeySqrGridModel(
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

	const cv::Point2f getExpectedCenterOfFace(int column, int row) {
		const cv::Point2f faceCenterRotatedClockwise = cv::Point2f(
			topLeftPointRotatedClockwise.x + column * distanceBetweenColumns,
			topLeftPointRotatedClockwise.y + row * distanceBetweenRows
		);
		const cv::Point2f faceCenter = rotatePointCounterclockwise(faceCenterRotatedClockwise, centerPoint, angleInRadians);
		return faceCenter;
	}
	const cv::Point2f getExpectedCenterOfFace(int faceIndex) {
		return getExpectedCenterOfFace( faceIndex % 5, faceIndex / 5);
	}

	/*
		Returns the index [0, 24] for a face center point, or -1 if the point provided
	 	Is not sufficiently close to a center of the grid in the KeySqr model
	*/
	const int inferFaceIndexFromCenterPoint(const cv::Point2f candidateFaceCenter, const float maxFractionFromCenter = 0.25f) {
		const cv::Point2f rotatedPoint = rotatePointClockwise(candidateFaceCenter, centerPoint, angleInRadians);
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
			// The approximate column/row is within grid, but not close enough to a face center
			// (e.g. column 1.4 does not approximate column 1 well enough)
			return -1;
		}
		const int column = (int) round(approxColumn);
		const int row = (int) round(approxRow);
		const int faceIndex = (row * 5) + column;
		return faceIndex;
	}
};

KeySqrGridModel calculateKeySqrGrid(
	const cv::Mat &colorImage,
	const FacesAndStrayUndoverlinesFound &facesAndStrayUndoverlinesFound,
	float maxFractionOffaceWithromRowOrColumnLine = 0.1f // 1 mm
) {
	const std::vector<FaceRead> &facesFound = facesAndStrayUndoverlinesFound.facesFound;
	const std::vector<Undoverline> &strayUndoverlines = facesAndStrayUndoverlinesFound.strayUndoverlines;
	const	float maxPixelsFromRowOrColumnLine = maxFractionOffaceWithromRowOrColumnLine * facesAndStrayUndoverlinesFound.pixelsPerFaceEdgeWidth;

	for (int i = 0; i < facesFound.size(); i++) {
		// We can build a model of the grid based on this face if we can
		// find four others in the same row and four others in the same column.
		const FaceRead &candidateIntersectionFace = facesFound[i];
		GridProximity gridModel(candidateIntersectionFace.center, candidateIntersectionFace.inferredAngleInRadians);
		
		std::vector<cv::Point2f> candidatePoints, sameColumn, sameRow;
		for (int j = 0; j < facesFound.size(); j++) {
			if (j!=i) candidatePoints.push_back(facesFound[j].center);
		}
		for (const Undoverline &undoverline: strayUndoverlines) {
			candidatePoints.push_back(undoverline.inferredCenterOfFace);
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
		sameRow.push_back(candidateIntersectionFace.center);
		sameColumn.push_back(candidateIntersectionFace.center);

		// Sort the elements in rows by x and in columns by y
		std::sort(sameRow.begin(), sameRow.end(), [](cv::Point2f a, cv::Point2f b) { return a.x < b.x; });
		std::sort(sameColumn.begin(), sameColumn.end(), [](cv::Point2f a, cv::Point2f b) { return a.y < b.y; });

		// Uncomment when debugging grid, to see row and column the grid was constructed from
		// cv::line(colorImage, sameRow[0], sameRow[4], cv::Scalar(255, 255), 2);
		// cv::line(colorImage, sameColumn[0], sameColumn[4], cv::Scalar(255, 255), 2);
		
		// Now check that our row has near-constant distances
		std::vector<float> xValuesOfRowsWithinColumn = vmap<cv::Point2f, float>(sameColumn, [](const cv::Point2f *p) { return p->x; });
		std::vector<float> yValuesOfRowsWithinColumn = vmap<cv::Point2f, float>(sameColumn, [](const cv::Point2f *p) { return p->y; });
		std::vector<float> xValuesOfColumnsWithinRow = vmap<cv::Point2f, float>(sameRow, [](const cv::Point2f *p) { return p->x; });
		std::vector<float> yValuesOfColumnsWithinRow = vmap<cv::Point2f, float>(sameRow, [](const cv::Point2f *p) { return p->y; });
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
		
		// Figure out the row and column of the intersection face.
		// Since the arrayys of row Y values and column X values are sorted, we can do a linear serach.
		int rowOfIntersectionFace = 0, columnOfIntersectionFace = 0;
		while (rowOfIntersectionFace < yValuesOfRowsWithinColumn.size() && candidateIntersectionFace.center.y > yValuesOfRowsWithinColumn[rowOfIntersectionFace]) {
			rowOfIntersectionFace++;
		}
		while (columnOfIntersectionFace < xValuesOfColumnsWithinRow.size() && candidateIntersectionFace.center.x > xValuesOfColumnsWithinRow[columnOfIntersectionFace]) {
			columnOfIntersectionFace++;
		}
		
		// Find the center face's center point by moving to the third row
		// (row 2 with 0-based indexing) and third column (col 2).
		float centerX = candidateIntersectionFace.center.x
			+ (( 2 - rowOfIntersectionFace) * meanXDistanceBetweenRows)
			+ (( 2 - columnOfIntersectionFace) * meanXDistanceBetweenColumns);
		float centerY = candidateIntersectionFace.center.y
			+ (( 2 - rowOfIntersectionFace) * meanYDistanceBetweenRows)
			+ (( 2 - columnOfIntersectionFace) * meanYDistanceBetweenColumns);

		// Get the angle of the faces box as the angle of the row to which
		// the intersection face belonged.
		const float angleInRadians = angleOfLineInSignedRadians2f({0, 0}, {meanXDistanceBetweenColumns, meanYDistanceBetweenColumns});
		// Average the distance between the faces in rows and columsn to get the mean distanc
		const float distanceBetweenRows = distance2f(meanXDistanceBetweenColumns, meanYDistanceBetweenColumns);
		const float distanceBetweenColumns = distance2f(meanXDistanceBetweenRows, meanYDistanceBetweenRows);
		const cv::Point2f center(centerX, centerY);
		// Uncomment to debug grid boundaries
		// drawRotatedRect(colorImage,
		// 	cv::RotatedRect(center, cv::Size2f(distanceBetweenColumns * 5, distanceBetweenRows * 5), radiansToDegrees(angleInRadians)),
		// 	cv::Scalar(255, 0, 255, 4)
		// );

		return KeySqrGridModel(
			distanceBetweenRows,
			distanceBetweenColumns,
			angleInRadians,
			center
		);
	}
	// Made it to end without finding a grid.  Return invalid.
	return KeySqrGridModel();
}

FacesOrderdWithMissingFacesInferredFromUnderlines orderFacesAndInferMissingUndoverlines(
	const cv::Mat &colorImage,
	const cv::Mat &grayscaleImage,
	const FacesAndStrayUndoverlinesFound& facesAndStrayUndoverlinesFound,
	float maxMmFromRowOrColumnLine // = 1.0f // 1 mm
) {
	// First, take the faces and undoverlines we've found and try to build
	// a model a model that describes the locations within a 5x5 grid 
	auto grid = calculateKeySqrGrid(colorImage, facesAndStrayUndoverlinesFound, maxMmFromRowOrColumnLine);
	if (!grid.valid) {
		return {};
	}
	std::vector<FaceRead> orderedFaces(25);
	// Copy all the faces we found into the grid model
	for (const auto &faceFound : facesAndStrayUndoverlinesFound.facesFound) {
		const int faceIndex = grid.inferFaceIndexFromCenterPoint(faceFound.center);
		if (faceIndex >= 0) {
			// We were able to find this face in the grid model, so copy it in
			orderedFaces[faceIndex] = faceFound;
		}
	}
	// Copy all the stray undoverlines we found into the grid model
	for (const auto &undoverline : facesAndStrayUndoverlinesFound.strayUndoverlines) {
		const int faceIndex = grid.inferFaceIndexFromCenterPoint(undoverline.inferredCenterOfFace);
		if (faceIndex >= 0) {
			// We found this undoverline's face in the grid model, so copy it in
			if (undoverline.isOverline) {
				orderedFaces[faceIndex].overline = undoverline;
				if (!orderedFaces[faceIndex].underline.found) {
					orderedFaces[faceIndex].underline = readUndoverline(colorImage, grayscaleImage, undoverline.inferredOpposingUndoverlineRotatedRect);
				}
			}
			else {
				orderedFaces[faceIndex].underline = undoverline;
				if (!orderedFaces[faceIndex].overline.found) {
					orderedFaces[faceIndex].overline = readUndoverline(colorImage, grayscaleImage, undoverline.inferredOpposingUndoverlineRotatedRect);
				}
			}
			orderedFaces[faceIndex].inferredAngleInRadians = angleOfLineInSignedRadians2f(undoverline.line);
			orderedFaces[faceIndex].center = undoverline.inferredCenterOfFace;
		}
	}
	// Provide a center point for faces we know should be there, but for which we could find an overline
	// or underline.  We can use that center point for drawing a box around what we couldn't read.
	for (int i=0; i < 25; i++) {
		// Uncomment when debugging grid
		// circle(colorImage, grid.getExpectedCenterOfFace(i), 6, cv::Scalar(255, 0, 255), 3);
		if (!orderedFaces[i].underline.found && !orderedFaces[i].overline.found) {
			orderedFaces[i].center = grid.getExpectedCenterOfFace(i);
		}
	}

	return { true, orderedFaces, grid.angleInRadians, facesAndStrayUndoverlinesFound.pixelsPerFaceEdgeWidth };
}
