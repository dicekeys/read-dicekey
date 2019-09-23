//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include "color.h"

void drawRotatedRect(
  cv::Mat image,
  const cv::RotatedRect& rrect,
  cv::Scalar color = cv::Scalar(0,0,0),
  int thickness = 1
);