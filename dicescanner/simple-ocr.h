#pragma once

#include <limits>

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include "inconsolata-700.h"


struct OcrResultEntry {
  char character;
  int errorScore;
};

typedef std::vector<OcrResultEntry> OcrResult;

const OcrResult findClosestMatchingCharacter(
  const OcrAlphabet &alphabet,
  const cv::Mat &bwImageOfCharacter
) {
  const int imageHeight = bwImageOfCharacter.rows;
  const int imageWidth = bwImageOfCharacter.cols;
  const int numberOfCharactersInAlphabet = (int) alphabet.characters.size();
  std::vector<OcrResultEntry> result(numberOfCharactersInAlphabet);

  for (int i=0; i < numberOfCharactersInAlphabet; i++) {
    result[i].character = alphabet.characters[i].character;
    result[i].errorScore = 0;
  }

  for (int charIndex = 0; charIndex < numberOfCharactersInAlphabet; charIndex++) {
    for (int imageY = 0; imageY < imageHeight; imageY++) {
      for (int imageX = 0; imageX < imageWidth; imageX++) {
				const uchar pixel = bwImageOfCharacter.at<uchar>(cv::Point2i(imageX, imageY));
        const bool isImagePixelBlack = pixel < 128;
        const int modelX = int( ((imageX + 0.5f) * alphabet.charWidthInPixels) / float(imageWidth));
        const int modelY = int( ((imageY + 0.5f) * alphabet.charHeightInPixels) / float(imageHeight));
				assert(modelX < alphabet.charWidthInPixels);
				assert(modelY < alphabet.charHeightInPixels);
        const int modelIndex = (modelY * alphabet.charWidthInPixels) + modelX;
				assert(modelIndex < alphabet.characters[charIndex].ifPixelIsBlack.size());
        for (int charIndex = 0; charIndex < numberOfCharactersInAlphabet; charIndex++) {
					const int penalty = isImagePixelBlack ?
						alphabet.characters[charIndex].ifPixelIsBlack[modelIndex] :
						alphabet.characters[charIndex].ifPixelIsWhite[modelIndex];
					assert(penalty <= 5);
					result[charIndex].errorScore += penalty;
        }
      }
    }
  }

  std::sort(result.begin(), result.end(), [](OcrResultEntry a, OcrResultEntry b) {return a.errorScore < b.errorScore;} );

  return result;
}

const OcrResult readLetter(const cv::Mat &letterImage) {
  return findClosestMatchingCharacter(Inconsolata700::letters, letterImage);
}

const OcrResult readDigit(const cv::Mat &digitImage) {
  return findClosestMatchingCharacter(Inconsolata700::digits, digitImage);
}

