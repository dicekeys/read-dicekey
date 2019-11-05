//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#pragma once

#include <vector>
#include <cassert>
#include "keysqr-from-string.h"


// The face letter, an english capital letter other than 'Q', or '?' if unknown
char FaceFromStringTriple::letter() const { return _letter; };
// The face digit, '1'-'6', or '?' if unknown
char FaceFromStringTriple::digit() const { return _digit; }
// Return integers 0-3 (NOT chars '0'-'3') or '?' if unkown
char FaceFromStringTriple::orientationAs0to3ClockwiseTurnsFromUpright() const {
  return _orientationAs0to3ClockwiseTurnsFromUpright;
}

FaceFromStringTriple::FaceFromStringTriple(
  std::string letterDigitOrientationTriple
) :
  _letter(letterDigitOrientationTriple[0]),
  _digit(letterDigitOrientationTriple[1]),
  _orientationAs0to3ClockwiseTurnsFromUpright(
    letterDigitOrientationTriple.length() < 3 ?
    '?' :
    orientationAsLowercaseLetterTRBLToClockwiseTurnsFromUpright(
      letterDigitOrientationTriple[2]
    )
  )
{}

FaceFromStringTriple FaceFromStringTriple::rotate(int clockwiseTurnsToRight) const
{
  return FaceFromStringTriple(
    std::string(1, letter()) + 
    std::string(1, digit()) +
    std::string(1, trbl(
      clockwiseTurnsToRange0To3(this->orientationAs0to3ClockwiseTurnsFromUpright() + clockwiseTurnsToRight)))
  );
}

inline const std::vector<FaceFromStringTriple> humanReadableFormToFaces(const std::string humanReadableForm) {
    assert((humanReadableForm.length() % NumberOfFaces) == 0);
    unsigned charsPerFace = humanReadableForm.length() / NumberOfFaces;
    assert (charsPerFace >= 2 && charsPerFace <= 3);
    std::vector<FaceFromStringTriple> faces;
    for (int i = 0; i < NumberOfFaces; i++) {
      faces.push_back(FaceFromStringTriple(humanReadableForm.substr(i * charsPerFace, charsPerFace)));
    }
    return faces;
}

KeySqrFromString::KeySqrFromString(const std::string humanReadableForm) :
  KeySqr(humanReadableFormToFaces(humanReadableForm)) {}
