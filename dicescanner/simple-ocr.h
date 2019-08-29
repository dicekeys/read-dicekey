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

cv::Mat ocrErrorHeatMap(
  const OcrAlphabet &alphabet,
  const char characterRead,
  const cv::Mat &bwImageOfCharacter
) {  
  const int charRows = bwImageOfCharacter.rows;
  const int charCols = bwImageOfCharacter.cols;

  const int rows = charRows * 3 + 2;
  const int cols = charCols;

  const int topRowOfErrorMode = 0;
  const int topRowOfCharacterRead = charRows + 1;
  const int topRowOfErrorsCalculatedForCharacterRead = (charRows + 1) * 2;

  cv::Mat errorImage(rows, cols, CV_8UC3);
  // Black out the rectangle so that lines between images are black.
  rectangle(errorImage, cv::Rect(0, 0, cols, rows), cv::Scalar(0, 0, 0), cv::FILLED);

  const OcrChar* charRecord = vreduce<OcrChar, const OcrChar*>( alphabet.characters,
    [characterRead](const OcrChar* r, const OcrChar* c) -> const OcrChar* {return c->character == characterRead ? c : r; },
    (const OcrChar*)(NULL));

  for (int y=0; y < charRows; y++) {
    for (int x=0; x < charCols; x++) {
        const uchar pixel = bwImageOfCharacter.at<uchar>(cv::Point2i(x, y));
        const bool isImagePixelBlack = pixel < 128;
        const int modelX = int( ((x + 0.5f) * alphabet.ocrCharWidthInPixels) / float(charCols));
        const int modelY = int( ((y + 0.5f) * alphabet.ocrCharHeightInPixels) / float(charRows));
				assert(modelX < alphabet.ocrCharWidthInPixels);
				assert(modelY < alphabet.ocrCharHeightInPixels);
        const int modelIndex = (modelY * alphabet.ocrCharWidthInPixels) + modelX;
        const unsigned penaltyIfWhite = charRecord->ifPixelIsWhite[modelIndex];
        const unsigned penaltyIfBlack = charRecord->ifPixelIsBlack[modelIndex];
        // BGR => B=0, G=1, R=2

        // Write in the the image that shows the potential errors at each pixel
        if (penaltyIfWhite > 0) {
					errorImage.at<cv::Vec3b>(topRowOfErrorMode + y, x)[0] = ((255 * (5 - penaltyIfWhite)) / 5);
					errorImage.at<cv::Vec3b>(topRowOfErrorMode + y, x)[1] = ((255 * (5 - penaltyIfWhite)) / 5);
          errorImage.at<cv::Vec3b>(topRowOfErrorMode + y, x)[2] = 127 + ((128 * (5 - penaltyIfWhite)) / 5);
        } else if (penaltyIfBlack > 0) {
					errorImage.at<cv::Vec3b>(topRowOfErrorMode + y, x)[0] = 127 + ((128 * (5 - penaltyIfBlack)) / 5);
					errorImage.at<cv::Vec3b>(topRowOfErrorMode + y, x)[1] = ((255 * (5 - penaltyIfBlack)) / 5);
					errorImage.at<cv::Vec3b>(topRowOfErrorMode + y, x)[2] = ((255 * (5 - penaltyIfBlack)) / 5);
				} else {
					errorImage.at<cv::Vec3b>(topRowOfErrorMode + y, x)[0] =
					errorImage.at<cv::Vec3b>(topRowOfErrorMode + y, x)[1] =
					errorImage.at<cv::Vec3b>(topRowOfErrorMode + y, x)[2] = 255;
				}
        // Copy the b/w image of the character read into this color version
        errorImage.at<cv::Vec3b>(topRowOfCharacterRead + y, x)[0] =
          errorImage.at<cv::Vec3b>(topRowOfCharacterRead + y, x)[1] =
          errorImage.at<cv::Vec3b>(topRowOfCharacterRead + y, x)[2] = isImagePixelBlack ? 0 : 255;
        // Copy in the actual penalty or white if none
        if ( (isImagePixelBlack && penaltyIfBlack > 0) || ((!isImagePixelBlack) && penaltyIfWhite > 0) ) {
          errorImage.at<cv::Vec3b>(topRowOfErrorsCalculatedForCharacterRead + y, x) = errorImage.at<cv::Vec3b>(topRowOfErrorMode + y, x);
        } else {
          errorImage.at<cv::Vec3b>(topRowOfErrorsCalculatedForCharacterRead + y, x)[0] = 
            errorImage.at<cv::Vec3b>(topRowOfErrorsCalculatedForCharacterRead + y, x)[1] =
            errorImage.at<cv::Vec3b>(topRowOfErrorsCalculatedForCharacterRead + y, x)[2] = 255;
        }
    }
  }

  return errorImage;
}

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
        const int modelX = int( ((imageX + 0.5f) * alphabet.ocrCharWidthInPixels) / float(imageWidth));
        const int modelY = int( ((imageY + 0.5f) * alphabet.ocrCharHeightInPixels) / float(imageHeight));
				assert(modelX < alphabet.ocrCharWidthInPixels);
				assert(modelY < alphabet.ocrCharHeightInPixels);
        const int modelIndex = (modelY * alphabet.ocrCharWidthInPixels) + modelX;
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

