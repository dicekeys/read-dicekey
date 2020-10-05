//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#pragma once

#include "cv.h"
#include "color.h"

void drawRotatedRect(
  cv::Mat image,
  const cv::RotatedRect& rrect,
  cv::Scalar color = cv::Scalar(0,0,0),
  int thickness = 1
);