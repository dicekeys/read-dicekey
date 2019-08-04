#pragma once
#include <float.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include <iostream>
#include "vfunctional.h"
#include "rectangle.h"
#include "find-undoverlines.h"
#include "value-clusters.h"
#include "rotate.h"
#include "read-dice.h"
#include "slope.h"
#define _USE_MATH_DEFINES
#include <math.h>


// the function draws all the squares in the image
static void writeSquares(cv::Mat& image, const std::vector<RectangleDetected>& rects, std::string name)
{
	auto clone = image.clone();
	for (auto rect : rects)
	{
		std::vector<cv::Point> points = vmap<cv::Point2f, cv::Point>(rect.points, [](cv::Point2f point) -> cv::Point {
			return cv::Point(point);
		});
		const cv::Point* p = points.data();
		int n = (int)points.size();
		polylines(clone, &p, &n, 1, true, cv::Scalar(0, 255, 0), 3, cv::LINE_AA);
	}

	// cv::imshow(wndname, image);
	cv::imwrite(name, clone);
}

//static std::vector<DieRead>
static std::vector<DieRead> getDiceFromImage(cv::Mat image, std::string fileIdentifier, std::string intermediateImagePath = "/dev/null", int boxDebugLevel = 0)
{
	cv::Mat gray;
	cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);

	// Step one: find the underlines
	auto candidateUndoverlines = findCandidateUndoverlines(gray);
	auto dice = readDice(image, gray, candidateUndoverlines);
	return dice;
	

// static std::string
static void getDiceJsonFromBoxImage(cv::Mat boxImage, std::string fileIdentifier = "", std::string intermediateImagePath = "/dev/null", int boxDebugLevel = 0) {
	static const std::vector<int> imageEncodingParametersForLosslessPNG = { cv::IMWRITE_PNG_COMPRESSION, 0 };
	//auto dieRead = 
	getDiceFromImage(boxImage, fileIdentifier, intermediateImagePath, boxDebugLevel);
	// // FIXME, walk through each die converting to 
	// std::vector<uchar> encodedImage;
	// // FIXME - should do this for each die and then base64 encode.
	// cv::imencode("png", boxImage, encodedImage, imageEncodingParametersForLosslessPNG);
	// return std::string("");
}