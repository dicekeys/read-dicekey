//  © 2019 Stuart Edward Schechter (Github: @uppajung)
#include <string>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include "utilities/vfunctional.h"
#include "keysqr.h"
#include "read-dice.h"
#include "validate-dice-read.h"

/*
Compare the dice read during a test to a 75 character specifiation string that contains
three-character triples of letter, digit ('0'-'6'), and orientation (as number of turns from upright,
'0'-'3').
*/
void validateDiceRead(
	const std::vector<ElementRead> diceRead,
	std::string diceAsString
) {
	const KeySqr diceKeyNonCanonical = diceReadToDiceKey(diceRead, true);
	const KeySqr diceKey = diceKeyNonCanonical.rotateToCanonicalOrientation();
	for (size_t dieIndex = 0; dieIndex < diceRead.size(); dieIndex++) {
		const auto dieFace = diceKey.faces[dieIndex];
		const std::string dieAsString = diceAsString.substr(dieIndex * 3, 3);
		const auto letter = dieAsString[0];
		const char digit = dieAsString[1];
		const char orientationChar = dieAsString[2];
		const int orientationAs0to3ClockwiseTurnsFromUpright =
			(orientationChar == '0' || orientationChar == 't') ? 0 :
			(orientationChar == '1' || orientationChar == 'r') ? 1 :
			(orientationChar == '2' || orientationChar == 'b') ? 2 :
			(orientationChar == '3' || orientationChar == 'l') ? 3 : -1;
		if (dieFace.letter == '\0' && dieFace.digit == '\0') {
			throw std::string("Die ") + std::to_string(dieIndex) + " letter and digit could not be read";
		} else if (dieFace.letter == '\0') {
			throw std::string("Die ") + std::to_string(dieIndex) + " letter could not be read";
		} else if (dieFace.digit == '\0') {
			throw std::string("Die ") + std::to_string(dieIndex) + " digit could not be read";
		} else if (dieFace.orientationAs0to3ClockwiseTurnsFromUpright != orientationAs0to3ClockwiseTurnsFromUpright) {
			throw std::string("Die ") + std::to_string(dieIndex) + " (" + dashIfNull(letter) + dashIfNull(digit) +
  			") has orientation " + std::to_string(dieFace.orientationAs0to3ClockwiseTurnsFromUpright) +
				" but should be " + std::to_string(orientationAs0to3ClockwiseTurnsFromUpright);
		} else if (dieFace.letter != letter) {
			throw std::string("Die ") + std::to_string(dieIndex) + " has letter " + dashIfNull(dieFace.letter) +
				" but should be " + dashIfNull(letter);
		} else if (dieFace.digit != digit) {
			throw std::string("Die ") + std::to_string(dieIndex) + " has digit " + dashIfNull(dieFace.digit) +
				" but should be " + dashIfNull(digit);
		}
	}
}
