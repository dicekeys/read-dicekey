#include "gtest/gtest.h"
#include "keysqr-from-string.h"


TEST(face, Inits) {
	FaceFromStringTriple face("A6r");
	ASSERT_EQ(face.letter(), 'A');
	ASSERT_EQ(face.digit(), '6');
	ASSERT_EQ(face.orientationAs0to3ClockwiseTurnsFromUpright(), 1);
}

TEST(face, toHumanReadableForm) {
	FaceFromStringTriple face("A6r");
	ASSERT_EQ(face.toHumanReadableForm(true), "A6r");
}
