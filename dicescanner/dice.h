#pragma once

#include <string>
#include <vector>
#include <limits>
#include "die-specification.h"
#include "decode-die.h"
#include "find-undoverlines.h"
#include "simple-ocr.h"
#include "bit-operations.h"


const int NumberOfFaces = 25;

std::vector<std::vector<unsigned char>> rotationIndexes = {
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



class DieFace {
  public:
    char letter;
    char digit;
    unsigned char orientationAs0to3ClockwiseTurnsFromUpright;
    unsigned char errorsPresent;

    DieFace() {
      letter = 0;
      digit = 0;
      orientationAs0to3ClockwiseTurnsFromUpright = 0;
      errorsPresent = 0xFF;
    }

    DieFace(char _letter, char _digit, unsigned char _orientationAs0to3ClockwiseTurnsFromUpright, unsigned char _errorsPresent = 0) {
      letter = _letter;
      digit = _digit;
      orientationAs0to3ClockwiseTurnsFromUpright = _orientationAs0to3ClockwiseTurnsFromUpright;
      errorsPresent = _errorsPresent;
    }

    bool equals(DieFace other) {
      return (
        // Undefined faces cannot be equal
        letter != '\0' &&
        digit != '\0' &&
        // Faces are equal if their letter, digit, and orientation match,
        // even if there were more errors when one was read than the other.
        letter == other.letter &&
        digit == other.digit &&
        orientationAs0to3ClockwiseTurnsFromUpright == other.orientationAs0to3ClockwiseTurnsFromUpright
      );
    }
};

class DiceKey {
  public:
  
  DieFace faces[NumberOfFaces];

  DiceKey(std::vector<DieFace> _faces) {
    if (_faces.size() == NumberOfFaces) {
      throw std::string("A DiceKey must have 25 faces");
    }
    for (int i=0; i < NumberOfFaces; i++) {
      faces[i] = _faces[i];
    }
  }

  bool areLettersUnique() {
    std::vector<char> letters;
    for (int i = 0; i < NumberOfFaces; i++) {
      letters.push_back(faces[i].letter);
    }
    std::sort(letters.begin(), letters.end(), [](char a, char b) { return a < b; });
    for (int i = 0; i < NumberOfFaces; i++) {
      if (letters[i] != Inconsolata700::letters.characters[i].character) {
        return false;
      }
    }
    return true;
  };


  unsigned char maxError()
  {
    if (!areLettersUnique()) {
      return 255;
    }
    unsigned char maxErrorFound = 0;
    for (int i=0; i < NumberOfFaces; i++) {
      if (faces[i].errorsPresent > maxErrorFound) {
        maxErrorFound = faces[i].errorsPresent;
      }
    }
    return maxErrorFound;
  }

  DiceKey rotateDiceKey(int clockwiseTurns)
  {
    if (clockwiseTurns < 0) {
      clockwiseTurns = 4 - ((-clockwiseTurns) % 4);
    }
    const unsigned clockwiseTurns0to3 = clockwiseTurns % 4;
    std::vector<unsigned char> &indexToMoveDieFaceFrom = rotationIndexes[clockwiseTurns0to3];
    std::vector<DieFace> rotatedFaces;
    for (size_t i = 0; i < NumberOfFaces; i++) {
      DieFace dieFace = faces[indexToMoveDieFaceFrom[i]];
      dieFace.orientationAs0to3ClockwiseTurnsFromUpright = (dieFace.orientationAs0to3ClockwiseTurnsFromUpright + clockwiseTurns) % 4;
      rotatedFaces.push_back(dieFace);
    }
    return DiceKey(rotatedFaces);
  }

  unsigned char clockwiseRotationsToCanonicalForm()
  {
    unsigned char clockwiseRotationsRequired = 0;
    for (unsigned char candidateRotationRequirement = 1; candidateRotationRequirement < 4; candidateRotationRequirement++) {
      // If the candidate rotation would result in the square having a top-left letter
      // that is earlier in sort order (lower unicode character) than the current rotation,
      // replace the current rotation with the candidate rotation.
      if (faces[rotationIndexes[candidateRotationRequirement][0]].letter < faces[rotationIndexes[clockwiseRotationsRequired][0]].letter) {
        clockwiseRotationsRequired = candidateRotationRequirement;
      }    
    }
    return clockwiseRotationsRequired;
  }

  DiceKey rotateToCanonicalDiceKey()
  {
    return rotateDiceKey(clockwiseRotationsToCanonicalForm());
  }

  int countMatchingDice(const DiceKey &other) {
    int numMatchingDice = 0;
    for (int i=0; i < NumberOfFaces; i++) {
      if (faces[i].equals(other.faces[i])) {
        numMatchingDice++;
      }
    }
  }

  DiceKey rotateToBestMatchOtherDiceKey(const DiceKey &other) {
    
  }

};
