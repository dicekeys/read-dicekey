#include "gtest/gtest.h"
#include "keysqr-from-string.h"


const std::string orderedKeySqrHrf =
	"A1tB2rC3bD4lE5tF6bG1tH1tI1tJ1tK1tL1tM1tN1tO1tP1tR1tS1tT1tU1tV1tW1tX1tY1tZ1t";

TEST(KeySqr, ToHumanReadableFormandBack) {
	KeySqrFromString key = KeySqrFromString(orderedKeySqrHrf);
	const std::string hrf = key.toHumanReadableForm(true);
	ASSERT_STREQ(orderedKeySqrHrf.c_str(), hrf.c_str());
}
