//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#pragma once

#include "externally-generated/keysqr-face-specification.h"
#include <string>

inline unsigned clockwiseTurnsToRange0To3(int clockwiseTurns) {
  if (clockwiseTurns < 0) {
    clockwiseTurns = 4 - ((-clockwiseTurns) % 4);
  }
  const unsigned clockwiseTurns0to3 = clockwiseTurns % 4;
  return clockwiseTurns0to3;
}

inline char orientationAsClockwiseTurnsFromUprightToLowercaseLetterTRBL(char orientationAs0to3ClockwiseTurnsFromUpright) {
  return (
    orientationAs0to3ClockwiseTurnsFromUpright >= 0 &&
    orientationAs0to3ClockwiseTurnsFromUpright < FaceRotationLetters.length()
  ) ?
    FaceRotationLetters[orientationAs0to3ClockwiseTurnsFromUpright] :
    '?';
}

inline char orientationAsLowercaseLetterTRBLToClockwiseTurnsFromUpright(char orientationAsLowercaseLetterTRBL) {
  switch (orientationAsLowercaseLetterTRBL) {
    case 't':
    case 'T':
    case '0':
    case 0:
      return 0;
    case 'r':
    case 'R':
    case '1':
    case 1:
      return 1;
    case 'b':
    case 'B':
    case '2':
    case 2:
      return 2;
    case 'l':
    case 'L':
    case '3':
    case 3:
      return 3;
    default:
      return '?';
  };
}

inline char rotateOrientationAsLowercaseLetterTRBL(char orientationAsLowercaseLetterTRBL, int clockwiseTurns) {
  const char orientationAs0to3ClockwiseTurnsFromUprightBeforeRotation = 
    orientationAsLowercaseLetterTRBLToClockwiseTurnsFromUpright(orientationAsLowercaseLetterTRBL);
  if (orientationAsLowercaseLetterTRBL == '?') {
    return '?';
  }
  const unsigned orientationAs0to3ClockwiseTurnsFromUprightAfterRotation = clockwiseTurnsToRange0To3(
    orientationAs0to3ClockwiseTurnsFromUprightBeforeRotation +
    clockwiseTurns
  );
  return orientationAsClockwiseTurnsFromUprightToLowercaseLetterTRBL(orientationAs0to3ClockwiseTurnsFromUprightAfterRotation);
}


class Face {
  public:
    // The face letter, an english capital letter other than 'Q', or '?' if unknown
    virtual char letter() const = 0;
    // The face digit, '1'-'6', or '?' if unknown
    virtual char digit() const = 0;
    // Return integers 0-3 (NOT chars '0'-'3') or '?' if unkown
    virtual char orientationAs0to3ClockwiseTurnsFromUpright() const = 0;
    // Return the face in JSON format
    virtual std::string toJson() const = 0;

    // Returns
    //   't' (top, 0 clockwise turns from upright)
    //   'r' (top, 1 clockwise turn from upright)
    //   'b' (top, 2 clockwise turns from upright)
    //   'l' (top, 3 clockwise turns from upright)
    //   '?' unknown
    inline char orientationAsLowercaseLetterTRBL() const {
      return orientationAsClockwiseTurnsFromUprightToLowercaseLetterTRBL(orientationAs0to3ClockwiseTurnsFromUpright());
    }

    inline std::string toHumanReadableForm(bool includeOrientation) const {
      return
        std::string(1, letter()) +
    		std::string(1, digit()) +
        (includeOrientation ?
          std::string(1, orientationAsLowercaseLetterTRBL()) : "");
    }

    inline bool isDefined() const {
      return letter() != '?' && digit() != '?' && orientationAs0to3ClockwiseTurnsFromUpright() != '?';
    }

    inline bool equals(const Face &other) const {
      return (
        // Undefined faces cannot be equal
        isDefined() &&
        // Faces are equal if their letter, digit, and orientation match.
        letter() == other.letter() &&
        digit() == other.digit() &&
        orientationAs0to3ClockwiseTurnsFromUpright() == other.orientationAs0to3ClockwiseTurnsFromUpright()
      );
    }
};
