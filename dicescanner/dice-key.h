#pragma once

#include <string>
#include <vector>
#include <limits>
#include "die-specification.h"
#include "decode-die.h"
#include "find-undoverlines.h"
#include "simple-ocr.h"
#include "bit-operations.h"
#include "die-face.h"


const int NumberOfFaces = 25;

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

/**
 * This class represents a DiceKey that has been read by scanning one or more images.
 */
class DiceKey {
  public:
  
  bool initialized;
  DieFace faces[NumberOfFaces];

  DiceKey() {
    initialized = false;
  }

  DiceKey(std::vector<DieFace> _faces) {
    if (_faces.size() != NumberOfFaces) {
      throw std::string("A DiceKey must have " + std::to_string(NumberOfFaces) + " faces");
    }
    initialized = true;
    for (int i=0; i < NumberOfFaces; i++) {
      faces[i] = _faces[i];
    }
  }

  /**
   * Return the DiceKey as a JSON array of 25 faces, or an empty array
   * if the DiceKey was not initialized
   * 
   * Each face follows the JSON format for DieFaces:
   *   inteface DieFace {
   *     letter: (string & DieLetter) | "",
   *     digit: (string & DieDigit) | "",
   *     orientationAs0to3ClockwiseTurnsFromUpright: '0' | '1' | '2' | '3',
   *     error {
   *       magnitude: number,
   *       location: number
   *     }
   *   }
   * 
   */
  std::string toJson() const {
    if (!initialized) {
      return "[]";
    }
    std::string json = "[\n";
    for (int i = 0; i < NumberOfFaces; i++) {
      json += "\t" + faces[i].toJson() + "\n";
    }
    json += "]";
    return json;
  };

  /**
   * Test to determine if every die face in the DiceKey has a different (unique)
   * letter from all others.  If a letter were to appear twice, indicating the
   * same die appeared twice, we would conclude the key to be invalid.
   */
  bool areLettersUnique() const {
    if (!initialized) return false;
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

  /**
   * Find the maximum error at any individual die face
   * (or return max error if two die faces have the same letter, indicating
   *  on of them must have a fatal read error because all die should be unique)
   **/
  unsigned char maxError() const
  {
    if (!areLettersUnique()) {
      return DieFaceErrors::Magnitude::Max;
    }
    unsigned char maxErrorFound = 0;
    for (int i=0; i < NumberOfFaces; i++) {
      if (faces[i].error.magnitude > maxErrorFound) {
        maxErrorFound = faces[i].error.magnitude;
      }
    }
    return maxErrorFound;
  }
  
  /**
   * Calculate the sum of the errors over all dice.
   */
  unsigned int totalError() const
  {
    if (!initialized || !areLettersUnique()) {
      return NumberOfFaces * (unsigned int)DieFaceErrors::Magnitude::Max;
    }
    unsigned int sumOfDieFaceErrors = 0;
    for (int i=0; i < NumberOfFaces; i++) {
      sumOfDieFaceErrors += (unsigned int) faces[i].error.magnitude;
    }
    return sumOfDieFaceErrors;
  }

  /**
   * Return a copy of this DiceKey that has been rotated by the specified
   * number of clockwise turns.
   */
  const DiceKey rotate(int clockwiseTurns) const
  {
    if (!initialized) return *this;
    if (clockwiseTurns < 0) {
      clockwiseTurns = 4 - ((-clockwiseTurns) % 4);
    }
    const unsigned clockwiseTurns0to3 = clockwiseTurns % 4;
    const std::vector<unsigned char> &indexToMoveDieFaceFrom = rotationIndexes[clockwiseTurns0to3];
    std::vector<DieFace> rotatedFaces;
    for (size_t i = 0; i < NumberOfFaces; i++) {
      DieFace dieFace = faces[indexToMoveDieFaceFrom[i]];
      dieFace.orientationAs0to3ClockwiseTurnsFromUpright = (dieFace.orientationAs0to3ClockwiseTurnsFromUpright + clockwiseTurns) % 4;
      rotatedFaces.push_back(dieFace);
    }
    return DiceKey(rotatedFaces);
  }

  /**
   * Calculate the number of clockwise turns required to put this DiceKey in
   * canonical orientation.
   * 
   * Canonical orientation assigns the top left corner to be the one with
   * the die with the smallest letter encoding (the first in the alphabet
   * for our ASCII capital letters).
   */
  unsigned char clockwiseRotationsToCanonicalOrientation() const
  {
    if (!initialized) return 0;
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

  /**
   * Return a copy of this DiceKey rotated into the canonical orientation
   * where the corner with the earliest character in the alphabet is at the
   * top left of the 5x5 square.
   */
  const DiceKey rotateToCanonicalOrientation() const
  {
    return rotate(clockwiseRotationsToCanonicalOrientation());
  }

  bool isPotentialMatch(const DiceKey &other) const {
    if (!initialized || !other.initialized) return false;
    int numMatchingDice = 0;
    for (int i=0; i < NumberOfFaces; i++) {
      if (faces[i].equals(other.faces[i])) {
        numMatchingDice++;
      } else {
        // faces don't match
        if (other.faces[i].error.magnitude == 0 && faces[i].error.magnitude == 0) {
          // The faces are different, but neither is supposed to be in error.
          // This means the entire grid must be different.  Either we're now
          // looking at another set of dice, the orientation rotated, or there
          // was an undetected die face read error.
          return false;
        }
      }
    }
    return numMatchingDice > 9;
  };

  /**
   * Merge an earlier-scanned DiceKey into this one if possible, so that we can
   * remove any errors that appear in the previous or current scan but appear to
   * be scanned without error in one of the two scans.
   **/
  const DiceKey mergePrevious(const DiceKey &previous) const {
    if (!initialized) return previous;
    if (isPotentialMatch(previous)) {
      std::vector<DieFace> newFaces;
      // There are enough matching dice in the previously-scanned DiceKey,
      // and no clear conflicts, so we can merge faces from the previous
      // DiceKey in cases where the face was scanned with fewer errors in the
      // past.
      for (int i=0; i < NumberOfFaces; i++) {
        // The face in this, the more-recent scan
        const DieFace &face = faces[i];
        // The face from the previous scan
        const DieFace &previousFace = previous.faces[i];

        // If we're merging, we know that either the faces match, or one
        // of them was not defined.
        if (!previousFace.isDefined() && !face.isDefined()) {
          // Neither is defined, but we can use the one with the smaller error
          newFaces.push_back(
            (face.error.magnitude <= previousFace.error.magnitude) ?
              face : previousFace
          );
        } else if (previousFace.isDefined() && !face.isDefined()) {
          // The previous scan succeeded in reading this face with majority agreement amnng
          // the three encodings, but this scan failed, so use the previously-scanned face.
          newFaces.push_back(previousFace);
        } else if (!previousFace.isDefined() && face.isDefined()) {
          // This scan succeeded in reading this face with majority agreement amnng
          // the three encodings, where the preivous scan failed, so use the face read
          // by this scan.
          newFaces.push_back(face);
        } else {
          // Both faces are defined, and must agree (otherwise isPotentialMatch would have returned false).
          //
          // There are three encodings of a die face: underline, overline, and the letter & digit between them.
          // We report errors in one of the encodings when the other two encodings agree.
          //
          // When two scans of a die face agree, but have errors in different locations, we may
          // (and below will) conclude that these were scanning errors, that the face is correct,
          // and so remove the individual scanning errors that were present in only one of the two scans.
          // 
          // For example, if the previous scan misread the underline and the current one misread
          // the overline (with OCR correctly identifying the letter/digit in both cases), we
          // can conclude that we've now seen a correct underline, overline, and OCR result,
          // and discard the individual errors as scanning errors.
          //
          // We identify the locations of errors as the intersection of the locations of errors
          // in the two scans.
          const unsigned char errorLocationIntersection = previousFace.error.location & face.error.location;
          // If there are no longer locations where errors were identified
          // (there are no longer any errors), we can set the magnitude of all (0) errors to 0.
          // Otherwise, we take the better (lower) error magnitude from the two scans.
          unsigned char errorMagnitude = (errorLocationIntersection == 0) ? 0 :
            MIN(previousFace.error.magnitude, face.error.magnitude);
          newFaces.push_back(DieFace(
            face.letter, face.digit, face.orientationAs0to3ClockwiseTurnsFromUpright,
            { errorMagnitude, errorLocationIntersection } )
          );
        }
      }
      return DiceKey(newFaces);
    } else {
      // isPotentialMatch failed, so the previous scan was incompatable with the die faces
      // from the current scan.  Perhaps the previous scan was at a different frame of
      // reference and the grid was rotated.
      for (int clockwiseTurns = 1; clockwiseTurns < 4; clockwiseTurns++) {
        // Since this rotation wasn't a potential match, try the 3 other potential
        // rotations and recurse a single time only if one is a match.
        // (where it will enter the above clause)
        DiceKey rotatedKey = previous.rotate(clockwiseTurns);
        if (isPotentialMatch(rotatedKey)) {
          return mergePrevious(rotatedKey);
        }
      }
    }
    // The previous key did not match closely enough to merge, even
    // when rotated to any of the four possible rotations.  Just return
    // a copy of the current key that isn't marged.
    return *this;
  }
};
