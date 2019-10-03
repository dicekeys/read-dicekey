//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#include <float.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include <math.h>
#include "utilities/vfunctional.h"
#include "graphics/geometry.h"
#include "graphics/draw-rotated-rect.h"
#include "dicekey.h"
#include "read-dice.h"
#include "visualize-read-results.h"
#include "write-die-characters.h"

// Colors are in BGR format
//const Color colorNoErrorGreen(0, 192, 0);
//const Color colorSmallErrorOrange = Color(192, 96, 0);
//const Color colorBigErrorRed = Color(128, 0, 0);

Color errorMagnitudeToColor(unsigned errorMagnitude) {
  return errorMagnitude == 0 ?
      colorNoErrorGreen :
    errorMagnitude <= 3 ?
      colorSmallErrorOrange :
      colorBigErrorRed;
}

cv::Mat visualizeReadResults(
	cv::Mat &colorImage,
	const ReadDiceResult &diceRead,
	bool writeInPlace
) {
  cv::Mat resultImage = (writeInPlace ? colorImage : colorImage.clone());
  // Derive the length of each side of the die in pixels by dividing the
  // legnth off and 
  const float dieSizeInPixels = ElementDimensionsMm::size * diceRead.pixelsPerMm;
  const int thinLineThickness = 1 + int(dieSizeInPixels / 70);
  const int thickLineThickness = 2 * thinLineThickness;

  if (diceRead.success) {
    for (const DieRead &die: diceRead.dice) {
      const auto error = die.error();

      // Draw a rectangle around the die if an error has been found
      if (error.magnitude > 0) {
        drawRotatedRect(
          resultImage,
          cv::RotatedRect(die.center, cv::Size2d(dieSizeInPixels, dieSizeInPixels), radiansToDegrees(diceRead.angleInRadiansNonCononicalForm)),
          errorMagnitudeToColor(error.magnitude).scalar,
          error.magnitude == 0 ? thinLineThickness : thickLineThickness
        );
      }
      // Draw a rectangle arond the underline
      if (die.underline.found) {
        bool underlineError = (error.location & DieFaceErrors::Location::Underline);
        drawRotatedRect(resultImage, die.underline.fromRotatedRect,
          errorMagnitudeToColor( underlineError ? error.magnitude : 0 ).scalar,
          underlineError ? thickLineThickness : thinLineThickness );
      }
      // Draw a rectangle arond the overline
      if (die.overline.found) {
        bool overlineError = (error.location & DieFaceErrors::Location::Overline);
        drawRotatedRect(resultImage, die.overline.fromRotatedRect,
          errorMagnitudeToColor( overlineError ? error.magnitude : 0 ).scalar,
          overlineError ? thickLineThickness : thinLineThickness );
      }
      // Draw the characters read
      writeDieCharacters(resultImage, die.center, die.inferredAngleInRadians, diceRead.pixelsPerMm, die.letter(), die.digit(),
        errorMagnitudeToColor( (error.location & DieFaceErrors::Location::OcrLetter) ? error.magnitude : 0 ),
        errorMagnitudeToColor( (error.location & DieFaceErrors::Location::OcrDigit) ? error.magnitude : 0 )
      );
    }
  }
  for (Undoverline undoverline: diceRead.strayUndoverlines) {
    drawRotatedRect(resultImage, undoverline.fromRotatedRect, colorBigErrorRed.scalar, 1);
  }
  for (DieRead die: diceRead.strayDice) {
      drawRotatedRect(
        resultImage,
        cv::RotatedRect(die.center, cv::Size2d(dieSizeInPixels, dieSizeInPixels), radiansToDegrees(diceRead.angleInRadiansNonCononicalForm)),
        colorBigErrorRed.scalar, 1
      );
  }


	return resultImage;
}
