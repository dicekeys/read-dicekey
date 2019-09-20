#include "die-face.h"
//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#include <limits>

namespace DieFaceErrors {
  const DieFaceError WorstPossible = {
    DieFaceErrors::Magnitude::Max,
    DieFaceErrors::Location::All
  };

  const DieFaceError None = { 0, 0 };
}


DieFace::DieFace() {
  letter = 0;
  digit = 0;
  orientationAs0to3ClockwiseTurnsFromUpright = 0;
  error = DieFaceErrors::WorstPossible;
}

DieFace::DieFace(
  char _letter,
  char _digit,
  unsigned char _orientationAs0to3ClockwiseTurnsFromUpright,
  DieFaceError _error
) {
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
std::string DieFace::toJson() const {
  return "{ letter: '" + ( letter ? std::string(1, letter) : "" ) +
    "', digit: '" + ( digit ? std::string(1, digit) : "" ) +
    "', orientationAs0to3ClockwiseTurnsFromUpright: '" + std::string(1, '0' + orientationAs0to3ClockwiseTurnsFromUpright) +
    "', error: { magnitude: " + std::to_string(error.magnitude) + ", location: " + std::to_string(error.location) + " } }";
}

char DieFace::orientationChar(bool useDigitsForOrientation) const {
  if (orientationAs0to3ClockwiseTurnsFromUpright < 0 || orientationAs0to3ClockwiseTurnsFromUpright > 3) {
    throw "Invalid orientation";
  }

  if (useDigitsForOrientation) {
    // Return a digit representing the number of of clockwise
    // 90-degree turns from upright
    return '0' + orientationAs0to3ClockwiseTurnsFromUpright;
  } else {
    // Return the letter represting whether the top of the letter is to the
    // top (t) right (r, 90 degree turn), bottom (b, 180), or left (l, 270)
    return "trbl"[orientationAs0to3ClockwiseTurnsFromUpright];
  }
}

/*
  * Return the die face as a three-character triple of
  * letter, digit, and orientation ('0' - '3') in number of turns from clockwise.
  */
std::string DieFace::toTriple(bool useDigitsForOrientation) const {
  return ( letter ? std::string(1, letter) : "" ) +
    ( digit ? std::string(1, digit) : "" ) +
    std::string(1, orientationChar(useDigitsForOrientation) ) +
    "', error: { magnitude: " + std::to_string(error.magnitude) + ", location: " + std::to_string(error.location) + " } }";
}

bool DieFace::isDefined() const {
  return letter != '\0' && digit != '\0' && error.magnitude < DieFaceErrors::Magnitude::Max;
}

bool DieFace::equals(const DieFace &other) const {
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