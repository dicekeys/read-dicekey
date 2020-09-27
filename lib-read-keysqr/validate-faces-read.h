//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#pragma once

#include <string>
#include <iostream>
#include "graphics/cv.h"
#include "read-faces.h"


/*
Compare the faces read during a test to a 75 character specifiation string that contains
three-character triples of letter, digit ('0'-'6'), and orientation (as number of turns from upright,
'0'-'3').
*/
void validateFacesRead(
	const std::vector<FaceRead> &facesRead,
	std::string keySqrInHumanReadableForm,
	bool rotateToCanonicalOrientation = true
);

void validateFacesRead(
	const KeySqr<FaceRead>& keySqrNonCanonical,
	std::string keySqrInHumanReadableForm,
	bool rotateToCanonicalOrientation = true
);