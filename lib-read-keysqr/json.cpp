#include <iostream>
#include "json.h"

std::string pointToJson(const cv::Point2f point) {
	std::ostringstream jsonStream;
	jsonStream << "{" <<
		JsonKeys::Point::x + ": " << point.x << ", " <<
		JsonKeys::Point::y + ": " << point.y <<
		"}";
	return jsonStream.str();
};

std::string lineToJson(const Line line) {
	std::ostringstream jsonStream;
	jsonStream << "[" <<
		pointToJson(line.start) << ", " <<
		pointToJson(line.end) <<
	"]";
	return jsonStream.str();
}
