//  © 2019 Stuart Edward Schechter (Github: @uppajung)
#include <vector>
#include <cassert>
#include "face.hpp"

// The face letter, an english capital letter other than 'Q', or '?' if unknown
char Face::letter() const { return _letter; };
// The face digit, '1'-'6', or '?' if unknown
char Face::digit() const { return _digit; }
// Return integers 0-3 (NOT chars '0'-'3') or '?' if unkown
char Face::orientationAs0to3ClockwiseTurnsFromUpright() const {
  return _orientationAs0to3ClockwiseTurnsFromUpright;
}

Face::Face(
  const IFace &copyFrom
) : 
  _letter(copyFrom.letter()),
  _digit(copyFrom.digit()),
  _orientationAs0to3ClockwiseTurnsFromUpright(copyFrom.orientationAs0to3ClockwiseTurnsFromUpright())
{}

Face::Face(
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

Face Face::rotate(int clockwiseTurnsToRight) const
{
  return Face(
    std::string(1, letter()) + 
    std::string(1, digit()) +
    std::string(1, trbl(
      clockwiseTurnsToRange0To3(this->orientationAs0to3ClockwiseTurnsFromUpright() + clockwiseTurnsToRight)))
  );
}