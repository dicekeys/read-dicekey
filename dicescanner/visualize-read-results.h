#pragma once

#include <float.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include <math.h>
#include "vfunctional.h"
#include "statistics.h"
#include "geometry.h"
#include "die-specification.h"
#include "dice.h"
#include "simple-ocr.h"
#include "decode-die.h"
#include "find-undoverlines.h"
#include "value-clusters.h"
#include "bit-operations.h"
#include "find-dice.h"
#include "assemble-dice-key.h"
#include "read-die-characters.h"
#include "read-dice.h"

static cv::Mat visualizeReadResults(cv::Mat &colorImage, ReadDiceResult diceRead, bool writeInPlace = false)
{
  cv::Mat resultImage = (writeInPlace ? colorImage : colorImage.clone());
  for (DieRead die: diceRead.dice) {
    const int errors = die.error();
    // Derive the length of each side of the die in pixels by dividing the
    // legnth off and 
    const float dieSizeInPixels = DieDimensionsMm::size * diceRead.pixelsPerMm;
    cv::RotatedRect dieEdges(die.center, cv::Size2d(dieSizeInPixels, dieSizeInPixels), radiansToDegrees(diceRead.angleInRadiansNonCononicalForm));
    cv::Point2f pointsf[4];
    cv::Point points[4];
    dieEdges.points(pointsf);
    for (int i=0; i<4; i++) {
      points[i] = pointsf[i];
    }
    const cv:: Point* ppoints[1] = {
		  points
	  };
    int npt[] = { 4 };
    // Color in BGR format
    cv::Scalar color = errors == 0 ?
        cv::Scalar(0, 192, 0) :
      errors <= 3 ?
        cv::Scalar(0, 96, 192) :
        cv::Scalar(0, 00, 128);
    const int thickness = errors == 0 ? 1 : 2;
    polylines(resultImage, ppoints, npt, 1, true, color, thickness, cv::LineTypes::LINE_8);
		writeDieCharacters(resultImage, die.center, die.inferredAngleInRadians, diceRead.pixelsPerMm, die.letter(), die.digit(), errors > 0);
  }

	return resultImage;
}