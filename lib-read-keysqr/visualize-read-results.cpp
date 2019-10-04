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
#include "keysqr.h"
#include "read-faces.h"
#include "visualize-read-results.h"
#include "write-face-characters.h"

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
	cv::Mat &colorImage_BGR_CV_8UC3,
	const ReadFaceResult &facesRead,
	bool writeInPlace
) {
  cv::Mat resultImage = (writeInPlace ? colorImage_BGR_CV_8UC3 : colorImage_BGR_CV_8UC3.clone());
  // Derive the length of each side of the face in pixels by dividing the
  // legnth off and 
  const float faceSizeInPixels = ElementDimensionsFractional::size * facesRead.pixelsPerFaceEdgeWidth;
  const int thinLineThickness = 1 + int(faceSizeInPixels / 70);
  const int thickLineThickness = 2 * thinLineThickness;

  if (facesRead.success) {
    for (const FaceRead &face: facesRead.faces) {
      const auto error = face.error();

      // Draw a rectangle around the face if an error has been found
      if (error.magnitude > 0) {
        drawRotatedRect(
          resultImage,
          cv::RotatedRect(face.center, cv::Size2d(faceSizeInPixels, faceSizeInPixels), radiansToDegrees(facesRead.angleInRadiansNonCononicalForm)),
          errorMagnitudeToColor(error.magnitude).scalar,
          error.magnitude == 0 ? thinLineThickness : thickLineThickness
        );
      }
      // Draw a rectangle arond the underline
      if (face.underline.found) {
        bool underlineError = (error.location & FaceErrors::Location::Underline);
        drawRotatedRect(resultImage, face.underline.fromRotatedRect,
          errorMagnitudeToColor( underlineError ? error.magnitude : 0 ).scalar,
          underlineError ? thickLineThickness : thinLineThickness );
      }
      // Draw a rectangle arond the overline
      if (face.overline.found) {
        bool overlineError = (error.location & FaceErrors::Location::Overline);
        drawRotatedRect(resultImage, face.overline.fromRotatedRect,
          errorMagnitudeToColor( overlineError ? error.magnitude : 0 ).scalar,
          overlineError ? thickLineThickness : thinLineThickness );
      }
      // Draw the characters read
      writeFaceCharacters(resultImage, face.center, face.inferredAngleInRadians, facesRead.pixelsPerFaceEdgeWidth, face.letter(), face.digit(),
        errorMagnitudeToColor( (error.location & FaceErrors::Location::OcrLetter) ? error.magnitude : 0 ),
        errorMagnitudeToColor( (error.location & FaceErrors::Location::OcrDigit) ? error.magnitude : 0 )
      );
    }
  }
  for (Undoverline undoverline: facesRead.strayUndoverlines) {
    drawRotatedRect(resultImage, undoverline.fromRotatedRect, colorBigErrorRed.scalar, 1);
  }
  for (FaceRead face: facesRead.strayFaces) {
      drawRotatedRect(
        resultImage,
        cv::RotatedRect(face.center, cv::Size2d(faceSizeInPixels, faceSizeInPixels), radiansToDegrees(facesRead.angleInRadiansNonCononicalForm)),
        colorBigErrorRed.scalar, 1
      );
  }


	return resultImage;
}
