#pragma once

//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#include "undoverline.h"
#include "dice-key.h"
#include "find-dice.h"
//#include "read-die-characters.h"

struct ReadDiceResult {
//	public:
	bool success;
	std::vector<DieRead> dice;
	float angleInRadiansNonCononicalForm;
	float pixelsPerMm;
	std::vector<DieRead> strayDice;
	std::vector<Undoverline> strayUndoverlines;
};