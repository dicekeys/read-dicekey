//////////////////////////////////////////////////////////
// KeySqr Specification (NOT TO BE MODIFIED DIRECTLY!)
//////////////////////////////////////////////////////////
//
// This c++ header file specifies properties of KeySqr,
// including:
//   -- The set of letters that uniquely identify each element
//      (such as a die or chip)
//   -- the set of digits (1-6) that identify the face of
//      each element
//   -- The encoding of bits into the underlines and overlines
//
// This file is generated automatically by the KeySqr
// specification generator, which is written in TypeScript.
// That program also generates:
//    -- The SVG file that renders the appearance of each face
//    -- The TypeScript version of this specification file
//
// To change the KeySqr specification, you will need to change
// and re-run specification program in TypeSecript.
//
// To add additional definitions or help functions, create a new file
// that reads the contants you need from this file.
//
#pragma once

#include <string>
#include <vector>

extern const std::string ElementLetters;
extern const std::string ElementDigits;

const int NumberOfDotsInUndoverline = 11;
const int MinNumberOfBlackDotsInUndoverline = 4;
const int MinNumberOfWhiteDotsInUndoverline = 4;


struct ElementFaceSpecification {
  char letter;
  char digit;
  unsigned char underlineCode;
  unsigned char overlineCode;
};

extern const ElementFaceSpecification NullElementFaceSpecification;

namespace ElemmentDimensionsMm {
};

namespace ElemmentDimensionsFractional {
  const float size = float(1);
  const float fontSize = float(0.575);
  const float undoverlineLength = float(0.775);
  const float undoverlineThickness = float(0.1375);
  const float overlineTop = float(0.1125);
  const float undoverlineMarginAtLineStartAndEnd = float(0.04375);
  const float undoverlineMarginAlongLength = float(0.0375);
  const float textBaselineY = float(0.675);
  const float textRegionWidth = float(0.925);
  const float textRegionHeight = float(0.37835);
  const float spaceBetweenLetterAndDigit = float(0.35);
  const float underlineTop = float(0.75);
  const float center = float(0.5);
  const float undoverlineLeftEdge = float(0.1125);
  const float undoverlineFirstDotLeftEdge = float(0.15625);
  const float undoverlineDotWidth = float(0.0625);
  const float undoverlineDotHeight = float(0.0625);
  const float centerOfUndoverlineToCenterOfFace = float(0.31875);
  const float overlineDotTop = float(0.15);
  const float underlineDotTop = float(0.7875);
  extern const std::vector<float> dotCentersAsFractionOfUndoverline;
};


extern const ElementFaceSpecification letterIndexTimesSixPlusDigitIndexToElementFaceSpecification[150];
extern const ElementFaceSpecification *underlineCodeToElementFaceSpecification[256];
extern const ElementFaceSpecification *overlineCodeToElementFaceSpecification[256];

/**
 * Get a pointer to a ElementFaceSpecification for from the eight-bit letter digit encoding
 * on an underline or overline.
 * This function never returns null.  Rather, if the encoding does not map to one of the 150 valid
 * faces, the result will be a pointer to the NullElementFaceSpecification.
 **/
const ElementFaceSpecification* decodeUndoverlineByte(bool isOverline, unsigned char letterDigitEncodingByte);
