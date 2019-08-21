#pragma once

#include "inconsolata-700.h"

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

// struct OcrResult {
//   int index = -1;
//   float relativeConfidence;
// };

int findBestFitIndex(
  const std::vector<const std::vector<const std::vector<float>>> &model,
  const cv::Mat bwImageOfCharacter
) {
  const int modelCharacters = model.size();
  const int modelRows = model[0].size();
  const int modelColumns = model[0][0].size();

  const int imageRows = bwImageOfCharacter.rows;
  const int imageCols = bwImageOfCharacter.cols;
  
  int bestIndex = -1;
  float bestError = std::_Max_possible_v<float>;
  float secondBestError;
  
  for (int charIndex = 0; charIndex < modelCharacters; charIndex++) {

    float totalError = 0;
    for (int row = 0; row < modelRows; row++) {
      for (int column = 0; column < modelColumns; column++) {
        const int top = floor( (column * imageCols) / modelColumns);
        const int bottom = floor( ( (column+1) * imageCols) / modelColumns);
        const int left = floor( (row * imageRows) / modelRows);
        const int right = floor( ( (row+1) * imageRows) / modelRows);
        int blackSamples = 0;
        int totalSamples = 0;
        for (int x = left; x < right; x++) {
          for (int y = top; y < bottom; y++) {
            if (bwImageOfCharacter.at<uchar>(cv::Point(x, y)) < 128) {
              blackSamples++;
            }
            totalSamples++;
          }
        }
        const float fractionBlack = (float(blackSamples) / float(totalSamples));
        const float error = abs(fractionBlack - model[charIndex][row][column]);
        totalError += error * error;
        if (charIndex == 0 || totalError < bestError) {
          secondBestError = bestError;
          bestError = totalError;
          bestIndex = charIndex;          
        } else if (totalError < secondBestError) {
          secondBestError = totalError;
        }
      }
    }
  }

  return bestIndex;
}