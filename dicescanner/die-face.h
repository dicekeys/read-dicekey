#pragma once


struct DieFaceError {
	unsigned char magnitude;
	unsigned char location;
};

namespace DieFaceErrors {
  namespace Location {
    const unsigned char Underline = 1;
    const unsigned char Overline = 2;
    const unsigned char OcrLetter = 4;
    const unsigned char OcrDigit = 8;
    const unsigned char All = Underline + Overline + OcrLetter + OcrDigit;
  }

  namespace Magnitude {
    const unsigned char OcrCharacterWasSecondChoice = 2;
    const unsigned char UnderlineOrOverlineMissing = 2;
    const unsigned char OcrCharacterInvalid = 8;
    const unsigned char Max = std::numeric_limits<unsigned char>::max();
  }

  const DieFaceError WorstPossible = {
    DieFaceErrors::Magnitude::Max,
    DieFaceErrors::Location::All
  };

  const DieFaceError None = { 0, 0 };
}

class DieFace {
  public:
    char letter;
    char digit;
    unsigned char orientationAs0to3ClockwiseTurnsFromUpright;
    DieFaceError error;

    const DieFace() {
      letter = 0;
      digit = 0;
      orientationAs0to3ClockwiseTurnsFromUpright = 0;
      error = DieFaceErrors::WorstPossible;
    }

    const DieFace(char _letter, char _digit, unsigned char _orientationAs0to3ClockwiseTurnsFromUpright,
            DieFaceError _error) {
      letter = _letter;
      digit = _digit;
      orientationAs0to3ClockwiseTurnsFromUpright = _orientationAs0to3ClockwiseTurnsFromUpright;
      error = _error;
    }

    bool isDefined() const {
      return letter != '\0' && digit != '\0' && error.magnitude < DieFaceErrors::Magnitude::Max;
    }

    bool equals(const DieFace &other) const {
      return (
        // Undefined faces cannot be equal
        isDefined() &&
        other.isDefined() &&
        // Faces are equal if their letter, digit, and orientation match,
        // even if there were more errors when one was read than the other.
        letter == other.letter &&
        digit == other.digit &&
        orientationAs0to3ClockwiseTurnsFromUpright == other.orientationAs0to3ClockwiseTurnsFromUpright
      );
    }
};
