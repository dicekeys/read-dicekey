//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#pragma once

#include <vector>
#include "dice-specification.h"
#include "die-face.h"

const int NumberOfFaces = 25;

/**
 * This class represents a DiceKey that has been read by scanning one or more images.
 */
class DiceKey {
  public:
  
  bool initialized;
  DieFace faces[NumberOfFaces];

  DiceKey();

  DiceKey(const std::string humanReadableFormat);

  DiceKey(std::vector<DieFace> _faces);

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
  std::string toJson() const;

  /**
   * Convert a DiceKey to a human-readable format with 3 characters representing
   * each die face as a letter, digit, and orientation.
   */
  std::string toHumanReadableForm(bool useDigitsForOrientation = false) const;

  /**
   * Test to determine if every die face in the DiceKey has a different (unique)
   * letter from all others.  If a letter were to appear twice, indicating the
   * same die appeared twice, we would conclude the key to be invalid.
   */
  bool areLettersUnique() const;

  /**
   * Find the maximum error at any individual die face
   * (or return max error if two die faces have the same letter, indicating
   *  on of them must have a fatal read error because all die should be unique)
   **/
  unsigned char maxError() const;
  
  /**
   * Calculate the sum of the errors over all dice.
   */
  unsigned int totalError() const;

  /**
   * Return a copy of this DiceKey that has been rotated by the specified
   * number of clockwise turns.
   */
  const DiceKey rotate(int clockwiseTurns) const;

  /**
   * Return a copy of this DiceKey rotated into the canonical orientation
   * where the corner with the earliest character in the alphabet is at the
   * top left of the 5x5 square.
   */
  const DiceKey rotateToCanonicalOrientation() const;

  bool isPotentialMatch(const DiceKey &other) const;

  /**
   * Merge an earlier-scanned DiceKey into this one if possible, so that we can
   * remove any errors that appear in the previous or current scan but appear to
   * be scanned without error in one of the two scans.
   **/
  const DiceKey mergePrevious(const DiceKey &previous) const;
};
