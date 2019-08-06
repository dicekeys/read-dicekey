#pragma once

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

const double multToGetRadiansFromDegrees = (2 * M_PI) / 360.0f;
const double multToGetDegreesFromRadians = 360.0f / (2 * M_PI);

static float radiansToDegrees(float r) { return float(r * multToGetDegreesFromRadians); }
static float degreesToRadians(float r) { return float(r * multToGetRadiansFromDegrees); }

struct Line {
	cv::Point2f start;
	cv::Point2f end;
};

static Line reverseLineDirection(Line line) {
	return { line.end, line.start };
}

static float distance2f(const cv::Point2f & a, const cv::Point2f & b) {
	float dx = a.x - b.x;
	float dy = a.y - b.y;
	return sqrt(dx * dx + dy * dy);
}
static float lineLength(Line line) {
	return distance2f(line.start, line.end);
}


static cv::Point2f pointBetween2f(cv::Point a, cv::Point b)
{
	return cv::Point2f(
		(a.x + b.x) / 2.0f,
		(a.y + b.y) / 2.0f
	);
}
static cv::Point2f pointAtCenterOfLine(Line line) {
	return pointBetween2f(line.start, line.end);
}

static bool isPointBetween2f(float x, float y, cv::Point2f bound1, cv::Point2f bound2)
{
	return
		(x >= MIN(bound1.x, bound2.x)) &&
		(x <= MAX(bound1.x, bound2.x)) &&
		(y >= MIN(bound1.y, bound2.y)) &&
		(y <= MAX(bound1.y, bound2.y));
}

static bool isPointBetween2f(cv::Point2f p, cv::Point2f bound1, cv::Point bound2)
{
	return isPointBetween2f(p.x, p.y, bound1, bound2);
}

static float angleOfLineInSignedRadians2f(cv::Point2f start, cv::Point2f end) {
	const float delta_x = end.x - start.x;
	const float delta_y = end.y - start.y;
	//if (delta_x == 0.0f) {
	//	return (delta_y > 0) ? float(M_PI/2) : float(-M_PI/2);
	//}
	return float(atan2(double(delta_y), double(delta_x)));
}

static float angleOfLineInSignedRadians2f(Line line) {
	return angleOfLineInSignedRadians2f(line.start, line.end);
}

static float angleOfLineInSignedDegrees2f(cv::Point2f start, cv::Point2f end) {
	return radiansToDegrees(angleOfLineInSignedRadians2f(start, end));
}
static float angleOfLineInSignedDegrees2f(Line line) {
	return angleOfLineInSignedDegrees2f(line.start, line.end);
}

static float normalizeAngleSignedDegrees(float angleInDegrees)
{
	return reduceToSignedRange( angleInDegrees, 45.0f);
}

const float NinetyDegreesAsRadians = float(90.0f * multToGetRadiansFromDegrees);
const float FortyFiveDegreesAsRadians = float(45.0f * multToGetRadiansFromDegrees);

static float normalizeAngleSignedRadians(float angleInRadians)
{
	return reduceToSignedRange(angleInRadians, FortyFiveDegreesAsRadians);
}

static cv::Point2f rotatePointCounterclockwiseAroundOrigin(const cv::Point2f &point, float angleInRadians) {
	const float s = sin(angleInRadians);
	const float c = cos(angleInRadians);
	return cv::Point2f(
		point.x * c - point.y * s,
		point.x * s + point.y * c
	);
}

static cv::Point2f rotatePointClockwiseAroundOrigin(const cv::Point2f &point, float angleInRadians) {
	return rotatePointCounterclockwiseAroundOrigin(point, -angleInRadians);
}