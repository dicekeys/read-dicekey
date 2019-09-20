//////////////////////////////////////////////////////////
// DiceKeys Specification (NOT TO BE MODIFIED DIRECTLY!)
//////////////////////////////////////////////////////////
//
// This c++ header file specifies properties of DiceKeys,
// including:
//   -- The set of letters that uniquely identify each die
//   -- the set of digits (1-6) that identify the face of
//      each die
//   -- The encoding of bits into the underlines and overlines
//
// This file is generated automatically by the DiceKey
// specification generator, which is written in TypeScript.
// That program also generates:
//    -- The SVG file that renders the appearance of each die face
//    -- The TypeScript version of this specification file
//
// To change the DiceKey specification, you will need to change
// and re-run specification program in TypeSecript.
//
// To add additional definitions or help functions, create a new file
// that reads the contants you need from this file.
//
#pragma once

#include <string>
#include <vector>

extern const std::string DieLetters;
extern const std::string DieDigits;

const int NumberOfDotsInUndoverline = 11;
const int MinNumberOfBlackDotsInUndoverline = 4;
const int MinNumberOfWhiteDotsInUndoverline = 4;


struct DieFaceSpecification {
  char letter;
  char digit;
  unsigned char underlineCode;
  unsigned char overlineCode;
};

extern const DieFaceSpecification NullDieFaceSpecification;

namespace DieDimensionsMm {
  const float size = float(8);
  const float fontSize = float(4.6);
  const float undoverlineLength = float(6.2);
  const float undoverlineThickness = float(1.1);
  const float overlineTop = float(0.9);
  const float undoverlineMarginAtLineStartAndEnd = float(0.35);
  const float undoverlineMarginAlongLength = float(0.3);
  const float textBaselineY = float(5.4);
  const float textRegionWidth = float(4.949999999999999);
  const float textRegionHeight = float(3.0267999999999997);
  const float spaceBetweenLetterAndDigit = float(0.35);
  const float underlineTop = float(6);
  const float center = float(4);
  const float undoverlineLeftEdge = float(0.9);
  const float undoverlineFirstDotLeftEdge = float(1.25);
  const float undoverlineDotWidth = float(0.5);
  const float undoverlineDotHeight = float(0.5);
  const float centerOfUndoverlineToCenterOfDie = float(2.55);
  const float overlineDotTop = float(1.2);
  const float underlineDotTop = float(6.3);
};

namespace DieDimensionsFractional {
  const float size = float(1);
  const float center = float(0.5);
  const float centerOfUndoverlineToCenterOfDie = float(0.31875);
  const float undoverlineLength = float(0.775);
  const float undoverlineThickness = float(0.1375);
  const float overlineTop = float(0.1125);
  const float underlineTop = float(0.75);
  const float undoverlineMarginAtLineStartAndEnd = float(0.04375);
  const float undoverlineMarginAlongLength = float(0.0375);
  const float textBaselineY = float(0.675);
  const float textRegionHeight = float(0.37834999999999996);
  const float textRegionWidth = float(0.6187499999999999);
  const float fontSize = float(0.575);
  const float spaceBetweenLetterAndDigit = float(0.04375);
  const float undoverlineLeftEdge = float(0.1125);
  const float undoverlineFirstDotLeftEdge = float(0.15625);
  const float undoverlineDotWidth = float(0.0625);
  const float undoverlineDotHeight = float(0.0625);
  const float overlineDotTop = float(0.15);
  const float underlineDotTop = float(0.7875);
  extern const std::vector<float> dotCentersAsFractionOfUndoverline;
};


extern const DieFaceSpecification letterIndexTimesSixPlusDigitIndexToDieFaceSpecification[150];
extern const DieFaceSpecification *underlineCodeToDieFaceSpecification[256];
extern const DieFaceSpecification *overlineCodeToDieFaceSpecification[256];

/**
 * Get a pointer to a DieFaceSpecification for from the eight-bit letter digit encoding
 * on an underline or overline.
 * This function never returns null.  Rather, if the encoding does not map to one of the 150 valid
 * die faces, the result will be a pointer to the NullDieFaceSpecification.
 **/
const DieFaceSpecification* decodeUndoverlineByte(bool isOverline, unsigned char letterDigitEncodingByte);
