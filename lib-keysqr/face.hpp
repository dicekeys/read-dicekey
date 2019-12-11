//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#pragma once

#include "externally-generated/keysqr-face-specification.h"
#include <string>

inline unsigned clockwiseTurnsToRange0To3(int clockwiseTurns) {
  return clockwiseTurns % 4;
}

// Convert clockwise turns to right to 't', 'r', 'b', 'l'
// (top, right, bottom, left)
// The preferred format of River City.
inline char trbl(int clockwiseTurnsToRight) {
  return clockwiseTurnsToRight == '?' ? '?' :
    FaceRotationLetters[clockwiseTurnsToRange0To3(clockwiseTurnsToRight)];
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
  return trbl(orientationAs0to3ClockwiseTurnsFromUprightAfterRotation);
}

class IFace {
  public:
    // The face letter, an english capital letter other than 'Q', or '?' if unknown
    virtual char letter() const = 0;
    // The face digit, '1'-'6', or '?' if unknown
    virtual char digit() const = 0;
    // Return integers 0-3 (NOT chars '0'-'3') or '?' if unkown
    virtual char orientationAs0to3ClockwiseTurnsFromUpright() const = 0;
    // Return the face in JSON format
    virtual std::string toJson() const {
      return "{ letter: '" + ( std::string(1, letter()) ) +
        "', digit: '" + ( std::string(1, digit()) ) +
        "', orientationAsLowercaseLetterTRBL: '" + std::string(1, orientationAsLowercaseLetterTRBL()) +
        "'}";
    }

    virtual unsigned int errorSize() const { return 0; }
    virtual int errorLocation() const { return 0; }

    // Returns
    //   't' (top, 0 clockwise turns from upright)
    //   'r' (top, 1 clockwise turn from upright)
    //   'b' (top, 2 clockwise turns from upright)
    //   'l' (top, 3 clockwise turns from upright)
    //   '?' unknown
    // Oh yes we've got trbl. Right here in river city.
    inline char orientationAsLowercaseLetterTRBL() const {
      return trbl(orientationAs0to3ClockwiseTurnsFromUpright());
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

    inline bool equals(const IFace &other) const {
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

template<class F>
class Rotateable {
public:
  virtual F rotate(int clockwiseTurnsToRight) const = 0;
};


class Face : public IFace, public Rotateable<Face> {
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

	Face rotate(int clockwiseTurnsToRight) const;

	Face(
		std::string letterDigitOrientationTriple
	);

	Face(
		const IFace& f
	);
};
