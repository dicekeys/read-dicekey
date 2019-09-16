//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

void drawRotatedRect(cv::Mat image, const cv::RotatedRect& rrect, cv::Scalar color = cv::Scalar(0,0,0), int thickness = 1) {
      cv::Point2f pointsf[4];
      cv::Point points[4];
      rrect.points(pointsf);
      for (int i=0; i<4; i++) {
        points[i] = pointsf[i];
      }
      const cv:: Point* ppoints[1] = {
        points
      };
      int npt[] = { 4 };
      polylines(image, ppoints, npt, 1, true, color, thickness, cv::LineTypes::LINE_8);
}