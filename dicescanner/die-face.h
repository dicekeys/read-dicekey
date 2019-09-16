//  © 2019 Stuart Edward Schechter (Github: @uppajung)

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

    DieFace() {
      letter = 0;
      digit = 0;
      orientationAs0to3ClockwiseTurnsFromUpright = 0;
      error = DieFaceErrors::WorstPossible;
    }

    DieFace(char _letter, char _digit, unsigned char _orientationAs0to3ClockwiseTurnsFromUpright,
            DieFaceError _error) {
      letter = _letter;
      digit = _digit;
      orientationAs0to3ClockwiseTurnsFromUpright = _orientationAs0to3ClockwiseTurnsFromUpright;
      error = _error;
    }

    /**
     * Return the die face as a JSON object with the following interface:
     *   inteface DieFace {
     *     letter: (string & DieLetter) | "",
     *     digit: (string & DieDigit) | "",
     *     orientationAs0to3ClockwiseTurnsFromUpright: '0' | '1' | '2' | '3',
     *     error {
     *       magnitude: number,
     *       location: number
     *     }
     *   }
     **/
    std::string toJson() const {
      return "{ letter: '" + ( letter ? std::string(1, letter) : "" ) +
        "', digit: '" + ( digit ? std::string(1, digit) : "" ) +
        "', orientationAs0to3ClockwiseTurnsFromUpright: '" + std::string(1, '0' + orientationAs0to3ClockwiseTurnsFromUpright) +
        "', error: { magnitude: " + std::to_string(error.magnitude) + ", location: " + std::to_string(error.location) + " } }";
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
