#pragma once

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

const double multToGetRadiansFromDegrees = (2 * M_PI) / 360.0f;
const double multToGetDegreesFromRadians = 360.0f / (2 * M_PI);

static float radiansToDegrees(float r) { return float(r * multToGetDegreesFromRadians); }
static float degreesToRadians(float r) { return float(r * multToGetRadiansFromDegrees); }

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
static Line reverseLineDirection(Line line) {
	return { line.end, line.start };
}

/*
Calculate the distance between two points.
*/
static float distance2f(const cv::Point2f & a, const cv::Point2f & b) {
	float dx = a.x - b.x;
	float dy = a.y - b.y;
	return sqrt(dx * dx + dy * dy);
}

/*
Calculate the length of a line.
*/
static float lineLength(Line line) {
	return distance2f(line.start, line.end);
}

/*
Calculate the midpoint between two points.
*/
static cv::Point2f midpoint2f(cv::Point2f a, cv::Point2f b)
{
	return cv::Point2f(
		(a.x + b.x) / 2.0f,
		(a.y + b.y) / 2.0f
	);
}

/*
Calculate the midpoint of a line.
*/
static cv::Point2f midpointOfLine(Line line) {
	return midpoint2f(line.start, line.end);
}

/*
Test to see if a point on a line, specified as an x and y coordinate, is between two bounds.
*/
static bool isPointBetween2f(float x, float y, cv::Point2f bound1, cv::Point2f bound2)
{
	return
		(x >= MIN(bound1.x, bound2.x)) &&
		(x <= MAX(bound1.x, bound2.x)) &&
		(y >= MIN(bound1.y, bound2.y)) &&
		(y <= MAX(bound1.y, bound2.y));
}

/*
Test to see if a point on a line is between two bounds
(e.g., the start and end of the line).
*/
static bool isPointBetween2f(cv::Point2f p, cv::Point2f bound1, cv::Point bound2)
{
	return isPointBetween2f(p.x, p.y, bound1, bound2);
}

/*
Calculate the slope of a line from a start point to an end point.
*/
static float angleOfLineInSignedRadians2f(cv::Point2f start, cv::Point2f end) {
	const float delta_x = end.x - start.x;
	const float delta_y = end.y - start.y;
	if (delta_x == 0) {
		if (delta_y < 0) {
			return -std::numeric_limits<float>::infinity();
		} else {
			return std::numeric_limits<float>::infinity();
		}
	}
	return delta_y / delta_x;
}

/*
Calculate the angle (in radians) of a line from a start point to an end point.
*/
static float angleOfLineInSignedRadians2f(cv::Point2f start, cv::Point2f end) {
	const float delta_x = end.x - start.x;
	const float delta_y = end.y - start.y;
	return float(atan2(double(delta_y), double(delta_x)));
}

/*
Calculate the angle (in radians) of a line from the start point to the end point.
*/
static float angleOfLineInSignedRadians2f(Line line) {
	return angleOfLineInSignedRadians2f(line.start, line.end);
}

/*
Calculate the angle (in degrees) of a line from the start point to the end point.
*/
static float angleOfLineInSignedDegrees2f(cv::Point2f start, cv::Point2f end) {
	return radiansToDegrees(angleOfLineInSignedRadians2f(start, end));
}

/*
Calculate the angle (in degrees) of a line from the start point to the end point.
*/
static float angleOfLineInSignedDegrees2f(Line line) {
	return angleOfLineInSignedDegrees2f(line.start, line.end);
}

/*
Convert an angle in degrees to the distance from a right angle
(the distance to the nearest 90 degree mark).
*/
static float degreesFromRightAngle(float angleInDegrees)
{
	return reduceToSignedRange( angleInDegrees, 45.0f);
}

const float NinetyDegreesAsRadians = float(90.0f * multToGetRadiansFromDegrees);
const float FortyFiveDegreesAsRadians = float(45.0f * multToGetRadiansFromDegrees);

/*
Convert an angle in radians to the distance from a right angle
(the distance to the nearest 90 degree mark, which is Pi/2 radians).
*/
static float radiansFromRightAngle(float angleInRadians)
{
	return reduceToSignedRange(angleInRadians, FortyFiveDegreesAsRadians);
}

/*
Rotate a point around the origin (counterclockwise for positive angles and clockwise for negative angles).
*/
static cv::Point2f rotatePointCounterclockwiseAroundOrigin(const cv::Point2f &point, float angleInRadians) {
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
static cv::Point2f rotatePointClockwiseAroundOrigin(const cv::Point2f &point, float angleInRadians) {
	return rotatePointCounterclockwiseAroundOrigin(point, -angleInRadians);
}




class GridProximity {
	// See https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line
	// Section: Line defined by two points
	private:
	// ax + by + c = 0     =>   y = (-a/b)x + c/b
	float distance, row_dx, row_dy, column_dx, column_dy, row_thirdAndFourthTerm, column_thirdAndFourthTerm;

	public:
	GridProximity(const cv::Point2f centerOfElement, const Line lineParallelToRowOrColumn) {
		const float dx = lineParallelToRowOrColumn.end.x - lineParallelToRowOrColumn.start.x;
		const float dy = lineParallelToRowOrColumn.end.y - lineParallelToRowOrColumn.start.y;

		// The row and column models will be represented by lines of the same distance
		distance = sqrt(dx * dx + dy * dy);

		// Rows should have |dx| > |dy| and columns should have |dy| > |dx|
		if (abs(dy) > abs(dx)) {
			row_dx = column_dy = dy;
			column_dx = row_dy = dx;
		} else {
			row_dx = column_dy = dx;
			column_dx = row_dy = dy;
		}
		
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
		return abs( (row_dy * p.x) - (row_dx * p.y) + row_thirdAndFourthTerm) / distance;
	}

	float pixelDistanceFromColumn(cv::Point2f p) {
		// See the formula for "line defined by two points" in
		// https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line
		// Noting that the denominator is just the distance between the two points
		// of the line model.
		return abs( (column_dy * p.x) - (column_dx * p.y) + column_thirdAndFourthTerm) / distance;
	}
};