#pragma once

static float distance2f(const cv::Point2f & a, const cv::Point2f & b) {
	float dx = a.x - b.x;
	float dy = a.y - b.y;
	return sqrt(dx * dx + dy * dy);
}


cv::Point2f pointBetween2f(cv::Point a, cv::Point b)
{
	return cv::Point2f(
		(a.x + b.x) / 2.0f,
		(a.y + b.y) / 2.0f
	);
}

bool isPointBetween2f(float x, float y, cv::Point bound1, cv::Point bound2)
{
	return
		(x >= MIN(bound1.x, bound2.x)) &&
		(x <= MAX(bound1.x, bound2.x)) &&
		(y >= MIN(bound1.y, bound2.y)) &&
		(y <= MAX(bound1.y, bound2.y));
}

bool isPointBetween2f(cv::Point p, cv::Point bound1, cv::Point bound2)
{
	return isPointBetween2f(p.x, p.y, bound1, bound2);
}