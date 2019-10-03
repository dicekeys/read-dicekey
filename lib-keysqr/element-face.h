//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#pragma once

#include <string>

struct FaceError {
	unsigned char magnitude;
	unsigned char location;
};

namespace FaceErrors {
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

  extern const FaceError WorstPossible;
  extern const FaceError None;
}

class ElementFace {
  public:
    char letter;
    char digit;
    unsigned char orientationAs0to3ClockwiseTurnsFromUpright;
    FaceError error;

    ElementFace();
    ElementFace(
      char _letter,
      char _digit,
      unsigned char _orientationAs0to3ClockwiseTurnsFromUpright,
      FaceError _error = {0, 0}
    );

    /**
     * Return the face as a JSON object with the following interface:
     *   inteface ElementFace {
     *     letter: Letter | "",
     *     digit: Digit | "",
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
     * Return the face as a three-character triple of
     * letter, digit, and orientation ('0' - '3') in number of turns from clockwise.
     */
    std::string toTriple(bool useDigitsForOrientation = false) const;

    bool isDefined() const;

    bool equals(const ElementFace &other) const;
};
