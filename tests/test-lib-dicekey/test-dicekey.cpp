#include "gtest/gtest.h"
#include "../../lib-dicekey/dicekey.h"


TEST(DiceKey, NotInitalized) {
	DiceKey key = DiceKey();
	ASSERT_EQ(key.initialized, false);
}

const std::string orderedDiceKeyHrf =
	"A1tB2rC3bD4lE5tF6bG1tH1tI1tJ1tK1tL1tM1tN1tO1tP1tR1tS1tT1tU1tV1tW1tX1tY1tZ1t";

TEST(DiceKey, ToHumanReadableFormandBack) {
	DiceKey key = DiceKey(orderedDiceKeyHrf);
	const std::string hrf = key.toHumanReadableForm();
	ASSERT_STREQ(orderedDiceKeyHrf.c_str(), hrf.c_str());
}

TEST(DiceKey, DoesMergeRemoveErrors) {
	DiceKey first(orderedDiceKeyHrf);
	first.faces[0].error.magnitude = 3;
	first.faces[0].error.magnitude = DieFaceErrors::Location::Underline;
	DiceKey second(orderedDiceKeyHrf);
	second.faces[0].error.magnitude = 2;
	second.faces[0].error.magnitude = DieFaceErrors::Location::Overline;
	ASSERT_GT(second.totalError(), (unsigned int) 0);
	second = second.mergePrevious(first);
	ASSERT_EQ(second.totalError(), 0);
}
