//  © 2019 Stuart Edward Schechter (Github: @uppajung)
#include <string>
#include <iostream>
#include "utilities/vfunctional.h"
#include "graphics/cv.h"
#include "keysqr.h"
#include "read-faces.h"
#include "read-keysqr.h"
#include "validate-faces-read.h"

/*
Compare the faces read during a test to a 75 character specifiation string that contains
three-character triples of letter, digit ('0'-'6'), and orientation (as number of turns from upright,
'0'-'3').
*/
void validateFacesRead(
	const std::vector<FaceRead> facesRead,
	std::string keySqrInHumanReadableForm
) {
	const KeySqr<FaceRead> keySqrNonCanonical = KeySqr<FaceRead>(facesRead);
	const KeySqr<FaceRead> keySqr = keySqrNonCanonical.rotateToCanonicalOrientation();
	for (size_t faceIndex = 0; faceIndex < facesRead.size(); faceIndex++) {
		const auto face = keySqr.faces[faceIndex];
		const std::string faceAsString = keySqrInHumanReadableForm.substr(faceIndex * 3, 3);
		const auto letter = faceAsString[0];
		const char digit = faceAsString[1];
		const int orientationAs0to3ClockwiseTurnsFromUpright = orientationAsLowercaseLetterTRBLToClockwiseTurnsFromUpright(faceAsString[2]);
		if (face.letter() == '?' && face.digit() == '?') {
			throw std::string("Face ") + std::to_string(faceIndex) + " letter and digit could not be read";
		} else if (face.letter() == '?') {
			throw std::string("Face ") + std::to_string(faceIndex) + " letter could not be read";
		} else if (face.digit() == '?') {
			throw std::string("Face ") + std::to_string(faceIndex) + " digit could not be read";
		} else if (face.orientationAs0to3ClockwiseTurnsFromUpright() != orientationAs0to3ClockwiseTurnsFromUpright) {
			throw std::string("Face ") + std::to_string(faceIndex) + " (" + dashIfNull(letter) + dashIfNull(digit) +
  			") has orientation " + std::to_string(face.orientationAs0to3ClockwiseTurnsFromUpright()) +
				" but should be " + std::to_string(orientationAs0to3ClockwiseTurnsFromUpright);
		} else if (face.letter() != letter) {
			throw std::string("Face ") + std::to_string(faceIndex) + " has letter " + dashIfNull(face.letter()) +
				" but should be " + dashIfNull(letter);
		} else if (face.digit() != digit) {
			throw std::string("Face ") + std::to_string(faceIndex) + " has digit " + dashIfNull(face.digit()) +
				" but should be " + dashIfNull(digit);
		}
	}
}
