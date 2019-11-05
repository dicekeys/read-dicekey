//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#pragma once

#include <vector>
#include <cassert>
#include "face.h"
#include "keysqr.h"

class FaceFromStringTriple: public Face, public Rotateable<FaceFromStringTriple> {
  private:
    const char _letter;
    const char _digit;
    char _orientationAs0to3ClockwiseTurnsFromUpright;
  public:
      // The face letter, an english capital letter other than 'Q', or '?' if unknown
    char letter() const;
    // The face digit, '1'-'6', or '?' if unknown
    char digit() const;
    // Return integers 0-3 (NOT chars '0'-'3') or '?' if unkown
    char orientationAs0to3ClockwiseTurnsFromUpright() const;

    FaceFromStringTriple rotate(int clockwiseTurnsToRight) const;

    FaceFromStringTriple(
      std::string letterDigitOrientationTriple
    );
};


/**
 * This class represents a KeySqr that has been read by scanning one or more images.
 */
class KeySqrFromString: public KeySqr<FaceFromStringTriple> {
  public:
    KeySqrFromString(const std::string humanReadableForm);
};
