//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#include <float.h>
#include <chrono>
#include <iostream>
#include <math.h>

#include "keysqr.hpp"

#include "utilities/vfunctional.h"
#include "utilities/statistics.h"
#include "utilities/bit-operations.h"
#include "graphics/cv.h"
#include "graphics/geometry.h"
#include "simple-ocr.h"
#include "decode-face.h"
#include "find-undoverlines.h"
#include "find-faces.h"
#include "assemble-keysqr.h"
#include "read-face-characters.h"
#include "visualize-read-results.h"
#include "json.h"

ReadFaceResult readFaces(
	const cv::Mat &grayscaleImage,
	bool outputOcrErrors
) {
	FaceAndStrayUndoverlinesFound faceAndStrayUndoverlinesFound = findFacesAndStrayUndoverlines(grayscaleImage);
	const auto orderedFacesResult = orderFacesAndInferMissingUndoverlines(grayscaleImage, faceAndStrayUndoverlinesFound);
	std::vector<FaceRead> orderedFaces;
	const float angleOfKeySqrInRadiansNonCanonicalForm = orderedFacesResult.angleInRadiansNonCanonicalForm;

	for (auto &face : orderedFacesResult.orderedFaces) {
		if (!(face.underline.determinedIfUnderlineOrOverline || face.overline.determinedIfUnderlineOrOverline)) {
			orderedFaces.push_back(FaceRead(face, '?', "", ""));
			// Without an overline or underline to orient the face, we can't read it.
		} else {
			// The threshold between black pixels and white pixels is calculated as the average (mean)
			// of the threshold used for the underline and for the overline, but if the underline or overline
			// is absent, we use the threshold from the line that is present.
			const uchar whiteBlackThreshold =
				(face.underline.found && face.overline.found) ?
				uchar(((unsigned int)(face.underline.whiteBlackThreshold) + (unsigned int)(face.overline.whiteBlackThreshold)) / 2) :
				face.underline.found ?
				face.underline.whiteBlackThreshold :
				face.overline.whiteBlackThreshold;
			const FaceSpecification& underlineInferred = *face.underline.faceInferred;
			const FaceSpecification& overlineInferred = *face.overline.faceInferred;
			const CharactersReadFromFaces charsRead = readCharactersOnFace(grayscaleImage, face.center(), face.inferredAngleInRadians(),
				faceAndStrayUndoverlinesFound.pixelsPerFaceEdgeWidth, whiteBlackThreshold,
				outputOcrErrors ? ("" + std::string(1, dashIfNull(underlineInferred.letter)) + std::string(1, dashIfNull(overlineInferred.letter))) : "",
				outputOcrErrors ? ("" + std::string(1, dashIfNull(underlineInferred.digit)) + std::string(1, dashIfNull(overlineInferred.digit))) : ""
			);


			const float orientationInRadians = face.inferredAngleInRadians() - angleOfKeySqrInRadiansNonCanonicalForm;
			const float orientationInClockwiseRotationsFloat = orientationInRadians * float(4.0 / (2.0 * M_PI));
			const uchar orientationInClockwiseRotationsFromUpright = uchar(round(orientationInClockwiseRotationsFloat) + 4) % 4;
			orderedFaces.push_back(FaceRead(
				face,
				orientationInClockwiseRotationsFromUpright,
				std::string(1, charsRead.lettersMostLikelyFirst[0].character) + std::string(1, charsRead.lettersMostLikelyFirst[1].character),
				std::string(1, charsRead.digitsMostLikelyFirst[0].character) + std::string(1, charsRead.digitsMostLikelyFirst[1].character)
			));
		}
	}

	return {
		orderedFacesResult.valid,
		orderedFaces,
		orderedFacesResult.angleInRadiansNonCanonicalForm,
		orderedFacesResult.pixelsPerFaceEdgeWidth //,
//		{}
	};
}
