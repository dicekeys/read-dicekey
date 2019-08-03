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
static void getDiceFromImage(cv::Mat image, std::string fileIdentifier, std::string intermediateImagePath = "/dev/null", int boxDebugLevel = 0)
{
	cv::Mat gray;
	cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);

	// Step one: find the underlines
	auto candidateUndoverlines = findCandidateUndoverlines(gray);
	auto dice = readDice(image, gray, candidateUndoverlines);
	//for (auto undoverline: candidateUndoverlines) {
	//	readUndoverline(image, gray, undoverline);
	//}

	//
	// Step one: identify the dice box by identifying the 25 squares formed by the 25 dice
	//
	//auto candidateDiceSquares = findCandidateDiceSquares(gray);

	//
	// Step two: find and remove the slope (angle) so that the dice are horizonally aligned
	// Future: should simply unskew image to have same effect but also remove kew
	//
	// auto boxSlope = median(vmap<RectangleDetected, float>(candidateDiceSquares, [](RectangleDetected r) { return slope(r.topLeft(), r.topRight()); }));
	// auto rotatedImage = rotateImageAndRectanglesFound(gray, candidateDiceSquares, boxSlope);

	// if (boxDebugLevel > 0) {
	// // Write the rotated dice for debugging purposes.
	// 	cv::Mat rotatedColor;
	// 	cv::cvtColor(rotatedImage, rotatedColor, cv::COLOR_GRAY2BGR);
	// 	if (!intermediateImagePath.find("/dev/null") == 0 && boxDebugLevel >= 1) {
	// 		writeSquares(rotatedColor, candidateDiceSquares, intermediateImagePath + "-dice-squares" + ".png");
	// 	}
	// }

	// //
	// // Find the dice within the flattened image and calculate the number of pixesl per millimeter
	// // in the process
	// //
	// float approxPixelsPerMm;
	// auto diceGrayscaleImages = findDice(rotatedImage, candidateDiceSquares, approxPixelsPerMm);

	// //
	// // Read each of the 25 dice indivudally by finding the underline to determine the rotation
	// // and then reading the text above the underline.
	// std::vector<DieRead> diceRead;
	// for (uint i = 0; i < diceGrayscaleImages.size(); i++) {
	// 	// DieRead dieRead;
	// 	int dieDebugLevel =
	// 		// FIXME -- remove this hack
	// 		((fileIdentifier.size() > 0) && (fileIdentifier[fileIdentifier.size() -1] == '1') && (i == 16)) ? 2 :
	// 		boxDebugLevel;

	// 	// orientAndReadDie(intermediateImagePath + + "-" + std::to_string(i) + "-", diceGrayscaleImages[i], dieRead, approxPixelsPerMm, dieDebugLevel);
	// 	// diceRead.push_back(dieRead);
	// 	//
	// 	if (dieDebugLevel > 0) {
	// 		std::string identifier = "-" + std::to_string(i);
	// 		if (dieRead.letterConfidence > 0 && dieRead.digitConfidence > 0) {
	// 			identifier += " ";
	// 			identifier += dieRead.letter;
	// 			identifier += dieRead.digit;
	// 			identifier += " " + std::to_string(dieRead.orientationInDegrees) + "";
	// 			std::cout << "File " << fileIdentifier << " Die " << i << " at angle " << dieRead.orientationInDegrees <<
	// 				" read as " << std::string(1, dieRead.letter) << std::string(1, dieRead.digit) <<
	// 				" with confidence " << dieRead.letterConfidence << ", " << dieRead.digitConfidence <<
	// 				"\n";
	// 		} else {
	// 			identifier += "READ-ERROR";
	// 			std::cout << "File " << fileIdentifier << " Die " << i << " at angle " << dieRead.orientationInDegrees << " could not be read.\n";
	// 		}
	// 		if (!intermediateImagePath.rfind("/dev/null", 0) == 0 && dieDebugLevel >= 1) {
	// 			cv::imwrite(intermediateImagePath + identifier + "-die.png", diceGrayscaleImages[i]);
	// 		}
	// 	}
	// }
	
	// if (diceGrayscaleImages.size() < 25) {
	// 	if (boxDebugLevel > 0) {
	// 		std::cout << "Not enough dice in found in " << intermediateImagePath << std::endl;
	// 	}
	// }

	// return diceRead;
}

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