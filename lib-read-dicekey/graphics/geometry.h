//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#pragma once

#include <float.h>
#include "../utilities/statistics.h"
#include "cv.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

const double multToGetRadiansFromDegrees = (2 * M_PI) / 360.0f;
const double multToGetDegreesFromRadians = 360.0f / (2 * M_PI);

inline float radiansToDegrees(float r) { return float(r * multToGetDegreesFromRadians); }
inline float degreesToRadians(float r) { return float(r * multToGetRadiansFromDegrees); }

/*
Represent a directed line from a start point to an end point.
*/
struct Line {
	cv::Point2f start;
	cv::Point2f end;
};

/*
Reverse the direction of a line by swapping the start end end.
*/
inline Line reverseLineDirection(Line line) {
	return { line.end, line.start };
}

/*
Calculate the distance of a line from the origin of to point x,y.
*/
inline float distance2f(const float x, const float y) {
	return sqrt(x * x + y * y);
}


/*
Calculate the distance between two points.
*/
inline float distance2f(const cv::Point2f & a, const cv::Point2f & b) {
	float dx = a.x - b.x;
	float dy = a.y - b.y;
	return distance2f(dx, dy);
}

/*
Calculate the length of a line.
*/
inline float lineLength(Line line) {
	return distance2f(line.start, line.end);
}

/*
Calculate the midpoint between two points.
*/
inline cv::Point2f midpoint2f(cv::Point2f a, cv::Point2f b)
{
	return cv::Point2f(
		(a.x + b.x) / 2.0f,
		(a.y + b.y) / 2.0f
	);
}

/*
Calculate the midpoint of a line.
*/
inline cv::Point2f midpointOfLine(Line line) {
	return midpoint2f(line.start, line.end);
}

/*
Test to see if a point on a line, specified as an x and y coordinate, is between two bounds.
*/
inline bool isPointBetween2f(float x, float y, cv::Point2f bound1, cv::Point2f bound2)
{
	return
		(x >= std::min(bound1.x, bound2.x)) &&
		(x <= std::max(bound1.x, bound2.x)) &&
		(y >= std::min(bound1.y, bound2.y)) &&
		(y <= std::max(bound1.y, bound2.y));
}

/*
Test to see if a point on a line is between two bounds
(e.g., the start and end of the line).
*/
inline bool isPointBetween2f(cv::Point2f p, cv::Point2f bound1, cv::Point bound2)
{
	return isPointBetween2f(p.x, p.y, bound1, bound2);
}

/*
Calculate the angle (in radians) of a line from a start point to an end point.
*/
inline float angleOfLineInSignedRadians2f(cv::Point2f start, cv::Point2f end) {
	const float delta_x = end.x - start.x;
	const float delta_y = end.y - start.y;
	return float(atan2(double(delta_y), double(delta_x)));
}

/*
Calculate the angle (in radians) of a line from the start point to the end point.
*/
inline float angleOfLineInSignedRadians2f(Line line) {
	return angleOfLineInSignedRadians2f(line.start, line.end);
}

/*
Calculate the angle (in degrees) of a line from the start point to the end point.
*/
inline float angleOfLineInSignedDegrees2f(cv::Point2f start, cv::Point2f end) {
	return radiansToDegrees(angleOfLineInSignedRadians2f(start, end));
}

/*
Calculate the angle (in degrees) of a line from the start point to the end point.
*/
inline float angleOfLineInSignedDegrees2f(Line line) {
	return angleOfLineInSignedDegrees2f(line.start, line.end);
}

/*
Convert an angle in degrees to the distance from a right angle
(the distance to the nearest 90 degree mark).
*/
inline float degreesFromRightAngle(float angleInDegrees)
{
	return reduceToSignedRange( angleInDegrees, 45.0f);
}

const float NinetyDegreesAsRadians = float(90.0f * multToGetRadiansFromDegrees);
const float FortyFiveDegreesAsRadians = float(45.0f * multToGetRadiansFromDegrees);

/*
Convert an angle in radians to the distance from a right angle
(the distance to the nearest 90 degree mark, which is Pi/2 radians).
*/
inline float radiansFromRightAngle(float angleInRadians)
{
	return reduceToSignedRange(angleInRadians, FortyFiveDegreesAsRadians);
}

/*
Rotate a point around the origin (counterclockwise for positive angles and clockwise for negative angles).
*/
inline cv::Point2f rotatePointCounterclockwiseAroundOrigin(const cv::Point2f &point, float angleInRadians) {
	const float s = sin(angleInRadians);
	const float c = cos(angleInRadians);
	return cv::Point2f(
		point.x * c - point.y * s,
		point.x * s + point.y * c
	);
}

/*
Rotate a point around the origin (clockwise for positive angles and counterclockwise for negative angles).
*/
inline cv::Point2f rotatePointClockwiseAroundOrigin(const cv::Point2f &point, float angleInRadians) {
	return rotatePointCounterclockwiseAroundOrigin(point, -angleInRadians);
}

inline cv::Point2f rotatePointCounterclockwise(
	const cv::Point2f &pointToRotate,
	const cv::Point2f &centerToRotateAround,
	const float angleInRadians) {
	const cv::Point2f pointRelativeToCenter = cv::Point2f(
		pointToRotate.x - centerToRotateAround.x,
		pointToRotate.y - centerToRotateAround.y
	);
	const cv::Point2f rotatedPointRelativeToCenter =
		rotatePointCounterclockwiseAroundOrigin(pointRelativeToCenter, angleInRadians);
	const cv::Point2f result = cv::Point2f(
		rotatedPointRelativeToCenter.x + centerToRotateAround.x,
		rotatedPointRelativeToCenter.y + centerToRotateAround.y
	);
	return result;
}

inline cv::Point2f rotatePointClockwise(
	const cv::Point2f &pointToRotate,
	const cv::Point2f &centerToRotateAround,
	const float angleInRadians) {
	return rotatePointCounterclockwise(pointToRotate, centerToRotateAround, -angleInRadians);
}



class GridProximity {
	// See https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line
	// Section: Line defined by two points
	private:
	float row_dx, row_dy, column_dx, column_dy, row_thirdAndFourthTerm, column_thirdAndFourthTerm;

	public:
	GridProximity(const cv::Point2f centerOfElement, const float angleInRadians) {
	
		const float rowAngleRadians = radiansFromRightAngle(angleInRadians);
		const float columnAngleRadians = float(rowAngleRadians + M_PI/2);
		row_dx = cos(rowAngleRadians);
		row_dy = sin(rowAngleRadians);
		column_dx = cos(columnAngleRadians);
		column_dy = sin(columnAngleRadians);
		
		// In the line model, x1 and y1 are the coordinates of the starting point.
		// We'll start both the column and row model from the center point
		const float x1 = centerOfElement.x;
		const float y1 = centerOfElement.y;
		// In the line model, x2 and y2 are the coordinates of the destination point.
		// We'll caclculate this for the row and column model by adding their respective
		// delta_x and delta_y values to the starting point.
		const float row_x2 = (x1 + row_dx);
		const float row_y2 = (y1 + row_dy);
		const float column_x2 = (x1 + column_dx);
		const float column_y2 = (y1 + column_dy);
		// The third and fourth terms of the numerator of the equation for the distance
		// of a line defined by (x1, y1) (x2, y2) is:
		//   x2 * y1 - y2 * x1
		// We calculate this for both the row line and the column line
		row_thirdAndFourthTerm = row_x2 * y1 - row_y2 * x1;
		column_thirdAndFourthTerm = column_x2 * y1 -  column_y2 * x1;
	}

	float pixelDistanceFromRow(cv::Point2f p) {
		// See the formula for "line defined by two points" in
		// https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line
		// Noting that the denominator is just the distance between the two points
		// of the line model.
		return abs( (row_dy * p.x) - (row_dx * p.y) + row_thirdAndFourthTerm);// / distance; // (distance=1)
	}

	float pixelDistanceFromColumn(cv::Point2f p) {
		// See the formula for "line defined by two points" in
		// https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line
		// Noting that the denominator is just the distance between the two points
		// of the line model.
		return abs( (column_dy * p.x) - (column_dx * p.y) + column_thirdAndFourthTerm);// / distance; // (distance=1)
	}
};
