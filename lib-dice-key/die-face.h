//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#pragma once

#include <string>

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

  extern const DieFaceError WorstPossible;
  extern const DieFaceError None;
}

class DieFace {
  public:
    char letter;
    char digit;
    unsigned char orientationAs0to3ClockwiseTurnsFromUpright;
    DieFaceError error;

    DieFace();
    DieFace(
      char _letter,
      char _digit,
      unsigned char _orientationAs0to3ClockwiseTurnsFromUpright,
      DieFaceError _error = {0, 0}
    );

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
    std::string toJson() const;

    char orientationChar(bool useDigitsForOrientation = false) const;

    /*
     * Return the die face as a three-character triple of
     * letter, digit, and orientation ('0' - '3') in number of turns from clockwise.
     */
    std::string toTriple(bool useDigitsForOrientation = false) const;

    bool isDefined() const;

    bool equals(const DieFace &other) const;
};
