#include "gtest/gtest.h"
#include "../../lib-keysqr/keysqr.h"


TEST(KeySqr, NotInitalized) {
	KeySqr key = KeySqr();
	ASSERT_EQ(key.initialized, false);
}

const std::string orderedKeySqrHrf =
	"A1tB2rC3bD4lE5tF6bG1tH1tI1tJ1tK1tL1tM1tN1tO1tP1tR1tS1tT1tU1tV1tW1tX1tY1tZ1t";

TEST(KeySqr, ToHumanReadableFormandBack) {
	KeySqr key = KeySqr(orderedKeySqrHrf);
	const std::string hrf = key.toHumanReadableForm();
	ASSERT_STREQ(orderedKeySqrHrf.c_str(), hrf.c_str());
}

TEST(KeySqr, DoesMergeRemoveErrors) {
	KeySqr first(orderedKeySqrHrf);
	first.faces[0].error.magnitude = 3;
	first.faces[0].error.magnitude = FaceErrors::Location::Underline;
	KeySqr second(orderedKeySqrHrf);
	second.faces[0].error.magnitude = 2;
	second.faces[0].error.magnitude = FaceErrors::Location::Overline;
	ASSERT_GT(second.totalError(), (unsigned int) 0);
	second = second.mergePrevious(first);
	ASSERT_EQ(second.totalError(), 0);
}
