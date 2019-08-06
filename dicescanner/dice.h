#pragma once

#include <string>
#include <vector>
#include "die-specification.h"
#include "decode-die.h"

struct DieFace {
    char letter = 0;
    char digit = 0;
    unsigned char orientationAs0to3ClockwiseTurnsFromUpright = 0;
};

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


std::vector<DieFace> rotateDiceKey(std::vector<DieFace> diceKey, int clockwiseTurns)
{
  if (diceKey.size() < 25) {
	  throw std::string("A DiceKey must have 25 dice, but only found " + std::to_string(diceKey.size()));
  }
  if (clockwiseTurns < 0) {
    clockwiseTurns = 4 - (-clockwiseTurns % 4);
  }
  const unsigned clockwiseTurns0to3 = clockwiseTurns % 4;
  std::vector<unsigned char> &indexToMoveDieFaceFrom = rotationIndexes[clockwiseTurns0to3];
  std::vector<DieFace> rotatedDiceKey;
  for (size_t i = 0; i < indexToMoveDieFaceFrom.size(); i++) {
    DieFace dieFace = diceKey[indexToMoveDieFaceFrom[i]];
    dieFace.orientationAs0to3ClockwiseTurnsFromUpright = (dieFace.orientationAs0to3ClockwiseTurnsFromUpright + clockwiseTurns) % 4;
    rotatedDiceKey.push_back(dieFace);
  }
  return rotatedDiceKey;
}

unsigned char clockwiseRotationsToCanonicalForm(std::vector<DieFace> diceKey)
{
  if (diceKey.size() < 25) {
	  throw std::string("A DiceKey must have 25 dice, but only found " + std::to_string(diceKey.size()));
  }
  unsigned char clockwiseRotationsRequired = 0;
  for (unsigned char candidateRotationRequirement = 1; candidateRotationRequirement < 4; candidateRotationRequirement++) {
    // If the candidate rotation would result in the square having a top-left letter
    // that is earlier in sort order (lower unicode character) than the current rotation,
    // replace the current rotation with the candidate rotation.
    if (diceKey[rotationIndexes[candidateRotationRequirement][0]].letter < diceKey[rotationIndexes[clockwiseRotationsRequired][0]].letter) {
      clockwiseRotationsRequired = candidateRotationRequirement;
    }    
  }
  return clockwiseRotationsRequired;
}

std::vector<DieFace> rotateToCanonicalDiceKey(std::vector<DieFace> diceKey)
{
  return rotateDiceKey(diceKey, clockwiseRotationsToCanonicalForm(diceKey));
}
