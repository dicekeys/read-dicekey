//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#include <limits>

#include "utilities/vfunctional.h"
#include "graphics/cv.h"
#include "simple-ocr.h"
#include "font.h"

cv::Mat ocrErrorHeatMap(
  const OcrFont &font,
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
        const size_t modelX = int( ((x + 0.5f) * font.ocrCharWidthInPixels) / float(charCols));
        const size_t modelY = int( ((y + 0.5f) * font.ocrCharHeightInPixels) / float(charRows));
				assert(modelX < font.ocrCharWidthInPixels);
				assert(modelY < font.ocrCharHeightInPixels);
        const unsigned char penaltyEntry = alphabet.penalties[
          ((modelY * font.ocrCharWidthInPixels) + modelX) * alphabet.characters.size()
        ];
        const unsigned penaltyIfWhite = penaltyEntry >> 4;
        const unsigned penaltyIfBlack = penaltyEntry & 0xf;
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
  const OcrFont &font,
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

  const float charWidthOverImageWidth = float(font.ocrCharWidthInPixels) / float(imageWidth);
  const float charHeightOverImageHeight = float(font.ocrCharHeightInPixels) / float(imageHeight);
  const size_t penaltyStepX = numberOfCharactersInAlphabet;
  const size_t penaltyStepY = (penaltyStepX * font.ocrCharWidthInPixels);
  for (int imageY = 0; imageY < imageHeight; imageY++) {
    const int modelY = int( (imageY + 0.5f) * charHeightOverImageHeight ); //  font.ocrCharHeightInPixels) / float(imageHeight));
    const unsigned char* penaltyPtrAtModelY = alphabet.penalties + (modelY * penaltyStepY);
    assert(modelY < font.ocrCharHeightInPixels);
    for (int imageX = 0; imageX < imageWidth; imageX++) {
      const int modelX = int( (imageX + 0.5f) * charWidthOverImageWidth ); // * font.ocrCharWidthInPixels) / float(imageWidth));
      const unsigned char* penaltyPtrAtModelYX = penaltyPtrAtModelY + (modelX * penaltyStepX);
      assert(modelX < font.ocrCharWidthInPixels);
      for (int charIndex = 0; charIndex < numberOfCharactersInAlphabet; charIndex++) {
        const uchar pixel = bwImageOfCharacter.at<uchar>(imageY, imageX); // cv::Point2i(imageX, imageY));
        unsigned char penaltyEntry = *(penaltyPtrAtModelYX + charIndex);
        const bool isImagePixelBlack = pixel < 128;
        // The high nibble has the penalty if the pixel is white,
        // and the low nibble has the penalty if the pixel is black
        unsigned char penalty = isImagePixelBlack ?
          (penaltyEntry & 0xf) : (penaltyEntry >> 4);
        result[charIndex].errorScore += penalty;
      }
    }
  }

  std::sort(result.begin(), result.end(), [](OcrResultEntry a, OcrResultEntry b) {return a.errorScore < b.errorScore;} );

  return result;
}

const OcrResult readLetter(const cv::Mat &letterImage) {
  const OcrFont *font = getFont();
  return findClosestMatchingCharacter(*font, font->letters, letterImage);
}

const OcrResult readDigit(const cv::Mat &digitImage) {
  const OcrFont *font = getFont();
  return findClosestMatchingCharacter(*font, font->digits, digitImage);
}

