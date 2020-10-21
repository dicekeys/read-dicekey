//////////////////////////////////////////////////////////
// DiceKey Specification (NOT TO BE MODIFIED DIRECTLY!)
//////////////////////////////////////////////////////////
//
// This c++ header file specifies properties of DiceKey,
// including:
//   -- The set of letters that uniquely identify each element
//      (such as a die or chip)
//   -- the set of digits (1-6) that identify the face of
//      each element
//   -- The encoding of bits into the underlines and overlines
//
// This file is generated automatically by the DiceKey
// specification generator, which is written in TypeScript.
// That program also generates:
//    -- The SVG file that renders the appearance of each face
//    -- The TypeScript version of this specification file
//
// To change the DiceKey specification, you will need to change
// and re-run specification program in TypeScript.
//
// To add additional definitions or help functions, create a new file
// that reads the constants you need from this file.
//
#pragma once

#include <string>
#include <vector>

extern const std::string FaceLetters;
extern const std::string FaceDigits;
extern const std::string FaceRotationLetters;

const int NumberOfDotsInUndoverline = 11;
const int MinNumberOfBlackDotsInUndoverline = 4;
const int MinNumberOfWhiteDotsInUndoverline = 4;


struct FaceSpecification {
  char letter;
  char digit;
  unsigned char underlineCode;
  unsigned char overlineCode;
};

extern const FaceSpecification NullFaceSpecification;

namespace FaceDimensionsFractional {
  const float size = float(1);
  const float margin = float(0);
  const float linearSizeOfFace = float(1);
  const float linearSizeOfFacesPrintArea = float(1);
  const float center = float(0.5);
  const float fontSize = float(0.741935);
  const float undoverlineLength = float(1);
  const float undoverlineThickness = float(0.177419);
  const float overlineTop = float(0);
  const float overlineBottom = float(0.177419);
  const float underlineBottom = float(1);
  const float underlineTop = float(0.822581);
  const float undoverlineMarginAtLineStartAndEnd = float(0.056452);
  const float undoverlineMarginAlongLength = float(0.048387);
  const float undoverlineLeftEdge = float(0);
  const float undoverlineFirstDotLeftEdge = float(0.056452);
  const float undoverlineDotWidth = float(0.080645);
  const float undoverlineDotHeight = float(0.080645);
  const float centerOfUndoverlineToCenterOfFace = float(0.41129);
  const float underlineDotTop = float(0.870968);
  const float overlineDotTop = float(0.048387);
  const float textBaselineY = float(0.725806);
  const float charWidth = float(0.370968);
  const float charHeight = float(0.488194);
  const float spaceBetweenLetterAndDigit = float(0.04375);
  const float textRegionWidth = float(0.785685);
  const float textRegionHeight = float(0.488194);
  extern const std::vector<float> dotCentersAsFractionOfUndoverline;
};

namespace JsonKeys {
	namespace Line {
		const std::string start = "start";
		const std::string end = "end";
	};
	namespace Point {
		const std::string x = "x";
		const std::string y = "y";
	};
	namespace Undoverline {
		const std::string line = "line";
		const std::string code = "code";
	};
	namespace FaceRead {
		const std::string underline = "underline";
		const std::string overline = "overline";
		const std::string orientationAsLowercaseLetterTrbl = "orientationAsLowercaseLetterTrbl";
		const std::string ocrLetterCharsFromMostToLeastLikely = "ocrLetterCharsFromMostToLeastLikely";
		const std::string ocrDigitCharsFromMostToLeastLikely = "ocrDigitCharsFromMostToLeastLikely";
		const std::string center = "center";
	};
};

extern const FaceSpecification letterIndexTimesSixPlusDigitIndexFaceWithUndoverlineCodes[150];
extern const FaceSpecification *underlineCodeToFaceSpecification[256];
extern const FaceSpecification *overlineCodeToFaceSpecification[256];

/**
 * Get a pointer to a FaceSpecification for from the eight-bit letter digit encoding
 * on an underline or overline.
 * This function never returns null.  Rather, if the encoding does not map to one of the 150 valid
 * faces, the result will be a pointer to the NullFaceSpecification.
 **/
const FaceSpecification* decodeUndoverlineByte(bool isOverline, unsigned char letterDigitEncodingByte);
