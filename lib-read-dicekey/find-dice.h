//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#pragma once

#include <float.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <vector>
#include "read-dice.h"

struct DiceAndStrayUndoverlinesFound {
	std::vector<ElementRead> diceFound;
	std::vector<Undoverline> strayUndoverlines;
	float pixelsPerFaceEdgeWidth;
};

DiceAndStrayUndoverlinesFound findDiceAndStrayUndoverlines(
	const cv::Mat &colorImage,
	const cv::Mat &grayscaleImage
);
