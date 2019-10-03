#include "gtest/gtest.h"
#include "../../lib-dicekey/element-face.h"


TEST(DieFace, Inits) {
	ElementFace face('A', '6', 1, { 1, ElementFaceErrors::Location::Underline });
	ASSERT_EQ(face.letter, 'A');
	ASSERT_EQ(face.digit, '6');
	ASSERT_EQ(face.orientationAs0to3ClockwiseTurnsFromUpright, 1);
	ASSERT_EQ(face.error.magnitude, 1);
	ASSERT_EQ(face.error.location, ElementFaceErrors::Location::Underline);
}

TEST(DieFace, toTriple) {
	ElementFace face('A', '6', 1, { 1, ElementFaceErrors::Location::Underline });
	ASSERT_EQ(face.toTriple(), "A6r");
	ASSERT_EQ(face.toTriple(true), "A61");
}
