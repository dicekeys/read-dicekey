//  © 2019 Stuart Edward Schechter (Github: @uppajung)
#include "draw-rotated-rect.h"

void drawRotatedRect(
  cv::Mat image,
  const cv::RotatedRect& rrect,
  cv::Scalar color,
  int thickness
) {
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
      cv::polylines(image, ppoints, npt, 1, true, color, thickness, cv::LineTypes::LINE_8);
}
