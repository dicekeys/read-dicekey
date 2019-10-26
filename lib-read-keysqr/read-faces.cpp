//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#include <float.h>
#include <chrono>
#include <iostream>
#include <math.h>

#include "keysqr.h"

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

char FaceRead::ocrLetterMostLikely() const {
	return this->ocrLetter.size() == 0 ? '\0' : ocrLetter[0].character;
}
char FaceRead::ocrDigitMostLikely() const {
	return ocrDigit.size() == 0 ? '\0' : ocrDigit[0].character;
}

char FaceRead::letter() const {
	return majorityOfThree(
		underline.faceInferred->letter, overline.faceInferred->letter, ocrLetterMostLikely()
	);
}
char FaceRead::digit() const {
	return majorityOfThree(
		underline.faceInferred->digit, overline.faceInferred->digit, ocrDigitMostLikely()
	);
}

std::string point2fToJson(const cv::Point2f point) {
	std::ostringstream jsonStream;
	jsonStream << "{" <<
		JsonKeys::Point::x + ": " << point.x << ", " <<
		JsonKeys::Point::y + ": " << point.y <<
		"}";
	return jsonStream.str();
};

std::string FaceRead::toJson() const {
  // JsonKeys::FaceRead::ocrDigitCharsFromMostToLeastLikely + "=" + ;
  // JsonKeys::FaceRead::ocrLetterCharsFromMostToLeastLikely + "=" + ;


	std::ostringstream jsonStream;
	jsonStream <<
		"{" <<
			JsonKeys::FaceRead::underline << ": " << underline.toJson() << ", " <<
			JsonKeys::FaceRead::overline << ": " << overline.toJson() << ", " <<
			JsonKeys::FaceRead::center << ": " << point2fToJson(center) << ", " <<
//			"angleInRadians: " << inferredAngleInRadians << "," <<
			JsonKeys::FaceRead::clockwise90DegreeRotationsFromUpright << ": " << orientationAs0to3ClockwiseTurnsFromUpright << "," <<
			JsonKeys::FaceRead::ocrLetterCharsFromMostToLeastLikely << ": \"" <<
				ocrLetter[0].character << ocrLetter[1].character << ocrLetter[2].character << "\", " <<
			JsonKeys::FaceRead::ocrDigitCharsFromMostToLeastLikely << ": \"" <<
				ocrDigit[0].character << ocrDigit[1].character << ocrDigit[2].character << "\", " <<
//			"ocrLetter: " << (ocrLetter.size() > 0 ? ocrLetter[0].character : '-') << "," <<
//			"ocrDigit: " << (ocrDigit.size() > 0 ? ocrDigit[0].character : '-') << "" <<
		"}";
	return jsonStream.str();
}

// Return an estimate of the error in reading an element face.
// If the underline, overline, and OCR results match, the error is 0.
// If the only error is a 1-3 bit error in either the underline or overline,
// the result is the number of bits (hamming distance) in the underline or overline
// that doesn't match the OCR result.
// If the underline and overline match but matched with the OCR's second choice of
// letter or digit, we return 2.
FaceError FaceRead::error() const {
		if (ocrLetter.size() == 0 || ocrDigit.size() == 0) {
			return FaceErrors::WorstPossible;
		}
		unsigned char errorLocation = 0;
		unsigned int errorMagnitude = 0;
		const char ocrLetter0 = ocrLetter[0].character;
		const char ocrDigit0 = ocrDigit[0].character;
		const FaceSpecification* pUnderlineFaceInferred = underline.faceInferred;
		const FaceSpecification* pOverlineFaceInferred = overline.faceInferred;

		// Test hypothesis of no error
		if (pUnderlineFaceInferred == pOverlineFaceInferred) {
			// The underline and overline map to the same face
			const FaceSpecification& undoverlineFaceInferred = *(underline.faceInferred);

			// Check for OCR errors for the letter read
			if (undoverlineFaceInferred.letter != ocrLetter[0].character) {
				errorLocation |= FaceErrors::Location::OcrLetter;
				errorMagnitude += undoverlineFaceInferred.letter == ocrLetter[1].character ?
					FaceErrors::Magnitude::OcrCharacterWasSecondChoice :
					FaceErrors::Magnitude::OcrCharacterInvalid;
			}
			if (undoverlineFaceInferred.digit != ocrDigit[0].character) {
				errorLocation |= FaceErrors::Location::OcrDigit;
				errorMagnitude += undoverlineFaceInferred.digit == ocrDigit[1].character ?
					FaceErrors::Magnitude::OcrCharacterWasSecondChoice :
					FaceErrors::Magnitude::OcrCharacterInvalid;
			}
			return {(unsigned char) MIN(std::numeric_limits<unsigned char>::max(), errorMagnitude), errorLocation};
		}
		const FaceSpecification& underlineFaceInferred = *pUnderlineFaceInferred;
		const FaceSpecification& overlineFaceInferred = *pOverlineFaceInferred;
		if (underlineFaceInferred.letter == ocrLetter0 && underlineFaceInferred.digit == ocrDigit0) {
			// The underline matches the OCR result, so the error is in the overline
			return {
					overline.found ?
						// The magnitude of the error is the hamming distance error in overline
						(unsigned char)hammingDistance(underlineFaceInferred.overlineCode, overline.letterDigitEncoding) :
						// Since the overline was not found, the magntidue is specified via a constant
						FaceErrors::Magnitude::UnderlineOrOverlineMissing,
					FaceErrors::Location::Overline
				};
		}
		if (overlineFaceInferred.letter == ocrLetter0 && overlineFaceInferred.digit == ocrDigit0) {
			// Since overline matches the OCR result, so the error is in the underline
			return {
				underline.found ?
					// The magnitude of the error is the hamming distance error in underline
					(unsigned char)hammingDistance(overlineFaceInferred.underlineCode, underline.letterDigitEncoding) :				
					// Since the underline was not found, the magntidue is specified via a constant
					FaceErrors::Magnitude::UnderlineOrOverlineMissing,
					FaceErrors::Location::Underline
			};
		}
		// No good matching.  Return max error
		return {std::numeric_limits<unsigned char>::max(), std::numeric_limits<unsigned char>::max()};
	};


ReadFaceResult readFaces(
	const cv::Mat &grayscaleImage,
	bool outputOcrErrors
) {
	FacesAndStrayUndoverlinesFound facesAndStrayUndoverlinesFound = findFacesAndStrayUndoverlines(grayscaleImage);
	auto orderedFacesResult = orderFacesAndInferMissingUndoverlines(grayscaleImage, facesAndStrayUndoverlinesFound);
	if (!orderedFacesResult.valid) {
		return { false, {}, 0, 0, facesAndStrayUndoverlinesFound.facesFound, facesAndStrayUndoverlinesFound.strayUndoverlines };
	}
	std::vector<FaceRead> ordeeredFaces = orderedFacesResult.orderedFaces;
	const float angleOfKeySqrInRadiansNonCanonicalForm = orderedFacesResult.angleInRadiansNonCanonicalForm;

	for (auto &face : ordeeredFaces) {
		if (!(face.underline.determinedIfUnderlineOrOverline || face.overline.determinedIfUnderlineOrOverline)) {
			continue;
			// Without an overline or underline to orient the face, we can't read it.
		}
		// The threshold between black pixels and white pixels is calculated as the average (mean)
		// of the threshold used for the underline and for the overline, but if the underline or overline
		// is absent, we use the threshold from the line that is present.
		const uchar whiteBlackThreshold =
			(face.underline.found && face.overline.found) ?
				uchar((uint(face.underline.whiteBlackThreshold) + uint(face.overline.whiteBlackThreshold)) / 2) :
			face.underline.found ?
				face.underline.whiteBlackThreshold :
				face.overline.whiteBlackThreshold;
		const FaceSpecification &underlineInferred = *face.underline.faceInferred;
		const FaceSpecification &overlineInferred = *face.overline.faceInferred;
		const CharactersReadFromFaces charsRead = readCharactersOnFace(grayscaleImage, face.center, face.inferredAngleInRadians,
			facesAndStrayUndoverlinesFound.pixelsPerFaceEdgeWidth, whiteBlackThreshold,
			outputOcrErrors ? ("" + std::string(1, dashIfNull(underlineInferred.letter)) + std::string(1, dashIfNull(overlineInferred.letter))) : "",
			outputOcrErrors ? ("" + std::string(1, dashIfNull(underlineInferred.digit)) + std::string(1, dashIfNull(overlineInferred.digit))) : ""
		);
			
		
		const float orientationInRadians = face.inferredAngleInRadians - angleOfKeySqrInRadiansNonCanonicalForm;
		const float orientationInClockwiseRotationsFloat = orientationInRadians * float(4.0 / (2.0 * M_PI));
		const uchar orientationInClockwiseRotationsFromUpright = uchar(round(orientationInClockwiseRotationsFloat) + 4) % 4;
		face.orientationAs0to3ClockwiseTurnsFromUpright = orientationInClockwiseRotationsFromUpright;
		face.ocrLetter = charsRead.lettersMostLikelyFirst;
		face.ocrDigit = charsRead.digitsMostLikelyFirst;
	}

	return { true, ordeeredFaces, orderedFacesResult.angleInRadiansNonCanonicalForm, orderedFacesResult.pixelsPerFaceEdgeWidth, {} };
}
