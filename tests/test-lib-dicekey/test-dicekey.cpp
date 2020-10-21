#include "gtest/gtest.h"
#include "dicekey-from-human-readable-form.hpp"


const std::string orderedDiceKeyHrf =
	"A1tB2rC3bD4lE5tF6bG1tH1tI1tJ1tK1tL1tM1tN1tO1tP1tR1tS1tT1tU1tV1tW1tX1tY1tZ1t";

TEST(DiceKey, ToHumanReadableFormandBack) {
	DiceKeyFromString key = DiceKeyFromString(orderedDiceKeyHrf);
	const std::string hrf = key.toHumanReadableForm(true);
	ASSERT_STREQ(orderedDiceKeyHrf.c_str(), hrf.c_str());
}
