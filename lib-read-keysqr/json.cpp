#include <iostream>
#include "json.h"

std::string pointToJson(const cv::Point2f point) {
	std::ostringstream jsonStream;
	jsonStream << "{" <<
		"\"" << JsonKeys::Point::x + "\": " << std::to_string(point.x) << ", " <<
		"\"" << JsonKeys::Point::y + "\": " << std::to_string(point.y) <<
		"}";
	return jsonStream.str();
};

std::string lineToJson(const Line line) {
	std::ostringstream jsonStream;
	jsonStream << "{" <<
		"\"" << JsonKeys::Line::start + "\": " << pointToJson(line.start) << ", " <<
		"\"" << JsonKeys::Line::end + "\": " << pointToJson(line.end) <<
	"}";
	return jsonStream.str();
}
