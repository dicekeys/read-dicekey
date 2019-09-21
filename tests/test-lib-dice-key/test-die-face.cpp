#include "gtest/gtest.h"
#include "../../lib-dice-key/die-face.h"


TEST(DieFace, Inits) {
	DieFace face('A', '6', 1, { 1, DieFaceErrors::Location::Underline });
	ASSERT_EQ(face.letter, 'A');
	ASSERT_EQ(face.digit, '6');
	ASSERT_EQ(face.orientationAs0to3ClockwiseTurnsFromUpright, 1);
	ASSERT_EQ(face.error.magnitude, 1);
	ASSERT_EQ(face.error.location, DieFaceErrors::Location::Underline);
}

TEST(DieFace, toTriple) {
	DieFace face('A', '6', 1, { 1, DieFaceErrors::Location::Underline });
	ASSERT_EQ(face.toTriple(), "A6r");
	ASSERT_EQ(face.toTriple(true), "A61");
}
