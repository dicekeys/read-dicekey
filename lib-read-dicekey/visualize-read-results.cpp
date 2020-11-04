//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#include <float.h>
#include <math.h>
#include "utilities/vfunctional.h"
#include "graphics/cv.h"
#include "graphics/geometry.h"
#include "graphics/draw-rotated-rect.h"
#include "assemble-dicekey.hpp"
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

/**
 * @brief Create an image overlay on top of an existing image
 * be it the image analyzed or a tranparent overlay.
 * 
 * @param overlayImage Must be a CV_8UC4 RGBA image
 * @param faces An array of the 25 faces
 * @param angleInRadiansNonCanonicalForm 
 * @param pixelsPerFaceEdgeWidth 
 * @return cv::Mat 
 */
cv::Mat visualizeReadResults(
	cv::Mat &overlayImage,
	const std::vector<FaceRead> &faces,
	float angleInRadiansNonCanonicalForm,
	float pixelsPerFaceEdgeWidth
) {
  // Derive the length of each side of the face in pixels by dividing the
  // legnth off and 
  const float faceSizeInPixels = FaceDimensionsFractional::size * pixelsPerFaceEdgeWidth;
  const int thinLineThickness = 1 + int(faceSizeInPixels / 70);
  const int thickLineThickness = 2 * thinLineThickness;

  for (const FaceRead &face: faces) {
    const auto error = face.error();

    // Draw a rectangle around the face if an error has been found
    if (error.magnitude > 0) {
      drawRotatedRect(
        overlayImage,
        cv::RotatedRect(face.center(), cv::Size2d(faceSizeInPixels, faceSizeInPixels), radiansToDegrees(angleInRadiansNonCanonicalForm)),
        errorMagnitudeToColor(error.magnitude).scalarRGBA,
        error.magnitude == 0 ? thinLineThickness : thickLineThickness
      );
    }
    // Draw a rectangle around the underline
    if (face.underline.found) {
      bool underlineError = (error.location & FaceErrors::Location::Underline);
      drawRotatedRect(overlayImage, face.underline.fromRotatedRect,
        errorMagnitudeToColor( underlineError ? error.magnitude : 0 ).scalarRGBA,
        underlineError ? thickLineThickness : thinLineThickness );
    }
    // Draw a rectangle around the overline
    if (face.overline.found) {
      bool overlineError = (error.location & FaceErrors::Location::Overline);
      drawRotatedRect(overlayImage, face.overline.fromRotatedRect,
        errorMagnitudeToColor( overlineError ? error.magnitude : 0 ).scalarRGBA,
        overlineError ? thickLineThickness : thinLineThickness );
    }
    // Draw the characters read
    writeFaceCharacters(overlayImage, face.center(), face.inferredAngleInRadians(), pixelsPerFaceEdgeWidth, face.letter(), face.digit(),
      errorMagnitudeToColor( (error.location & FaceErrors::Location::OcrLetter) ? error.magnitude : 0 ),
      errorMagnitudeToColor( (error.location & FaceErrors::Location::OcrDigit) ? error.magnitude : 0 )
    );
  }
  // for (Undoverline undoverline: facesRead.strayUndoverlines) {
  //   drawRotatedRect(overlayImage, undoverline.fromRotatedRect, colorBigErrorRed.scalarRGBA, 1);
  // }
  // for (FaceRead face: facesRead.strayFaces) {
  //     drawRotatedRect(
  //       overlayImage,
  //       cv::RotatedRect(face.center(), cv::Size2d(faceSizeInPixels, faceSizeInPixels), radiansToDegrees(facesRead.angleInRadiansNonCanonicalForm)),
  //       colorBigErrorRed.scalarRGBA, 1
  //     );
  // }


	return overlayImage;
}
