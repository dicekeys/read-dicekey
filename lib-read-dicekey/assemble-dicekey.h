//  © 2019 Stuart Edward Schechter (Github: @uppajung)
#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include "find-undoverlines.h"
#include "find-dice.h"
#include "read-dice.h"

struct DiceOrderdWithMissingDiceInferredFromUnderlines {
	bool valid = false;
	std::vector<ElementRead> orderedDice = {};
	// The angle of what was read on the page, without any conversion to have
	// the top left be the corner with the earliest letter in the alphabet
	float angleInRadiansNonCononicalForm = NAN;
	float pixelsPerFaceEdgeWidth;
};

DiceOrderdWithMissingDiceInferredFromUnderlines orderDiceAndInferMissingUndoverlines(
	const cv::Mat &colorImage,
	const cv::Mat &grayscaleImage,
	const DiceAndStrayUndoverlinesFound& diceAndStrayUndoverlinesFound,
	float maxMmFromRowOrColumnLine = 1.0f // 1 mm
);