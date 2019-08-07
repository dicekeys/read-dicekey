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