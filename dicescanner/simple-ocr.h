#pragma once

#include <limits>

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include "inconsolata-700.h"


// struct OcrResult {
//   int index = -1;
//   float relativeConfidence;
// };

int findClosestMatchingCharacter(
  const OcrAlphabet &alphabet,
  const cv::Mat &bwImageOfCharacter
) {
  const int numberOfCharactersInAlphabet = (int) alphabet.pixelPenalties.size();
  const int imageHeight = bwImageOfCharacter.rows;
  const int imageWidth = bwImageOfCharacter.cols;
  
  int bestIndex = -1;
	int bestError = std::numeric_limits<int>::max();
  int secondBestError = bestError;
  
  std::vector<int> scoreAtIndex(numberOfCharactersInAlphabet);
  for (int charIndex = 0; charIndex < numberOfCharactersInAlphabet; charIndex++) {
    for (int imageY = 0; imageY < imageHeight; imageY++) {
      for (int imageX = 0; imageX < imageWidth; imageX++) {
				const uchar pixel = bwImageOfCharacter.at<uchar>(cv::Point2i(imageX, imageY));
        const bool isImagePixelBlack = pixel < 128;
        const int modelX = (imageX * alphabet.charWidthInPixels) / imageWidth;
        const int modelY = (imageY * alphabet.charHeightInPixels) / imageHeight;
				assert(modelX < alphabet.charWidthInPixels);
				assert(modelY < alphabet.charHeightInPixels);
        const int modelIndex = (modelY * alphabet.charWidthInPixels) + modelX;
				assert(modelIndex < alphabet.pixelPenalties[charIndex].ifPixelIsBlack.size());
        for (int charIndex = 0; charIndex < numberOfCharactersInAlphabet; charIndex++) {
					const int penalty = isImagePixelBlack ?
						alphabet.pixelPenalties[charIndex].ifPixelIsBlack[modelIndex] :
						alphabet.pixelPenalties[charIndex].ifPixelIsWhite[modelIndex];
					assert(penalty <= 5);
					scoreAtIndex[charIndex] += penalty;
        }
      }
    }
  }


  for (int charIndex = 0; charIndex < numberOfCharactersInAlphabet; charIndex++) {
    int totalError = scoreAtIndex[charIndex];
    // const OcrChar &penalties = alphabet.pixelPenalties[charIndex];
    // // Calculate error for this character
    // for (int imageY = 0; imageY < imageHeight; imageY++) {
    //   for (int imageX = 0; imageX < imageWidth; imageX++) {
    //     const bool isImagePixelBlack = bwImageOfCharacter.at<uchar>(cv::Point(imageX, imageY)) < 128;
    //     const int modelX = (imageX * alphabet.charWidthInPixels) / imageWidth;
    //     const int modelY = (imageY * alphabet.charHeightInPixels) / imageHeight;
    //     const int modelIndex = modelY * imageWidth + modelX;
    //     const int errorAtPixel = isImagePixelBlack ?
    //       ( 1 * penalties.ifPixelIsBlack[modelIndex]) : (1 * penalties.ifPixelIsWhite[modelIndex]);
    //     totalError += errorAtPixel;
    //   }
    // }
    // If error is smaller than for any prior character, make this
    // the new winner
    if (charIndex == 0 || totalError < bestError) {
      secondBestError = bestError;
      bestError = totalError;
      bestIndex = charIndex;          
    } else if (totalError < secondBestError) {
      // If error is the second best, track that so we know how much
      // better best result is over second best.
      secondBestError = totalError;
    }
  }

  return bestIndex;
}

int readLetter(const cv::Mat &letterImage) {
  return findClosestMatchingCharacter(Inconsolata700::letters, letterImage);
}

int readDigit(const cv::Mat &digitImage) {
  return findClosestMatchingCharacter(Inconsolata700::digits, digitImage);
}

