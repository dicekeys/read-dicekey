#include "keysqr.hpp"

const std::vector<std::vector<unsigned char>> rotationIndexes = {
  {
    0,  1,  2,  3,  4,
    5,  6,  7,  8,  9,
    10, 11, 12, 13, 14,
    15, 16, 17, 18, 19,
    20, 21, 22, 23, 24
  },
  {
    20, 15, 10,  5,  0,
    21, 16, 11,  6,  1,
    22, 17, 12,  7,  2,
    23, 18, 13,  8,  3,
    24, 19, 14,  9,  4
  },
  {
    24, 23, 22, 21, 20,
    19, 18, 17, 16, 15,
    14, 13, 12, 11, 10,
      9,  8,  7,  6,  5,
      4,  3,  2,  1,  0
  },
  {
    4,  9, 14, 19, 24,
    3,  8, 13, 18, 23,
    2,  7, 12, 17, 22,
    1,  6, 11, 16, 21,
    0,  5, 10, 15, 20,
  }
};

const std::vector<unsigned char> getRotationIndexesFor5x5Square(int clockwiseTurns) {
  return rotationIndexes[clockwiseTurnsToRange0To3(clockwiseTurns)];
}

std::string rotateHumanReadableForm(const std::string humanReadableForm, int clockwiseTurns) {
  assert((humanReadableForm.length() % NumberOfFaces) == 0);
  unsigned charsPerFace = (unsigned) (humanReadableForm.length() / (size_t) NumberOfFaces);
  assert (charsPerFace >= 2 && charsPerFace <= 3);
  const std::vector<unsigned char> &indexToMoveFaceFrom = getRotationIndexesFor5x5Square(clockwiseTurns);
  std::string rotatedHumanReadableForm = "";
  for (size_t i = 0; i < NumberOfFaces; i++) {
    std::string faceString = humanReadableForm.substr( ((unsigned)indexToMoveFaceFrom[i]) * charsPerFace, charsPerFace);
    if (faceString.length() == 3) {
      faceString[2] = rotateOrientationAsLowercaseLetterTRBL(faceString[2], clockwiseTurns);
    }
    rotatedHumanReadableForm += faceString;
  }
  return rotatedHumanReadableForm;
};


const int rotationsToCanonicalForm(std::string humanReadableForm) {
  std::string humanReadableFormFirstInSortOrder = humanReadableForm;
  int clockwiseTurnsToHumanReadableFormFirstInSortOrder = 0;
  for (int clockwise90DegreeTurns = 1; clockwise90DegreeTurns < 4; clockwise90DegreeTurns++) {
    std::string rotatedHumanReadableForm = rotateHumanReadableForm(humanReadableForm, clockwise90DegreeTurns);
    if (rotatedHumanReadableForm < humanReadableFormFirstInSortOrder) {
      humanReadableFormFirstInSortOrder = rotatedHumanReadableForm;
      clockwiseTurnsToHumanReadableFormFirstInSortOrder = clockwise90DegreeTurns;
    }
  }
  return clockwiseTurnsToHumanReadableFormFirstInSortOrder;
}
