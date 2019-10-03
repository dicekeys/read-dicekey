#pragma once

//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#include <float.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include "graphics/color.h"
#include "read-faces.h"

// Colors are in BGR format
const Color colorNoErrorGreen(0, 192, 0);
const Color colorSmallErrorOrange = Color(192, 96, 0);
const Color colorBigErrorRed = Color(128, 0, 0);

Color errorMagnitudeToColor(unsigned errorMagnitude);

cv::Mat visualizeReadResults(
  cv::Mat &colorImage,
  const ReadDiceResult &diceRead,
  bool writeInPlace = false
);