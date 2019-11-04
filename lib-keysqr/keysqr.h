//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#pragma once

#include <vector>
#include "face.h"
#include "keysqr-generics.h"
#include "element-face.h"

/**
 * This class represents a KeySqr that has been read by scanning one or more images.
 */
class KeySqr {
  public:
  
  bool initialized;
  std::vector<ElementFace> faces;

  KeySqr();

  KeySqr(const std::string humanReadableFormat);

  KeySqr(std::vector<ElementFace> _faces);

  /**
   * Return the KeySqr as a JSON array of 25 faces, or an empty array
   * if the KeySqr was not initialized
   * 
   * Each face follows the JSON format for ElementFaces:
   *   inteface ElementFace {
   *     letter: Letter | "",
   *     digit: Digit | "",
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
   * Convert a KeySqr to a human-readable format with 3 characters representing
   * each face as a letter, digit, and orientation.
   */
  std::string toHumanReadableForm(bool includeFaceOrientations) const;

  /**
   * Test to determine if every face in the KeySqr has a different (unique)
   * letter from all others.  If a letter were to appear twice, indicating the
   * same element (die or chip) appeared twice, we would conclude the key to be invalid.
   */
  bool areLettersUnique() const;

  /**
   * Find the maximum error at any individual face
   * (or return max error if two faces have the same letter, indicating
   *  on of them must have a fatal read error because all elements should be unique)
   **/
  unsigned char maxError() const;
  
  /**
   * Calculate the sum of the errors over all faces.
   */
  unsigned int totalError(bool requireUniqueLetters = true) const;

  /**
   * Return a copy of this KeySqr that has been rotated by the specified
   * number of clockwise turns.
   */
  const KeySqr rotate(int clockwiseTurns) const;

  /**
   * Return a copy of this KeySqr rotated into the canonical orientation
   * where the corner with the earliest character in the alphabet is at the
   * top left of the 5x5 square.
   */
  const KeySqr rotateToCanonicalOrientation() const;

  bool isPotentialMatch(const KeySqr &other) const;

  /**
   * Merge an earlier-scanned KeySqr into this one if possible, so that we can
   * remove any errors that appear in the previous or current scan but appear to
   * be scanned without error in one of the two scans.
   **/
  const KeySqr mergePrevious(const KeySqr &previous) const;
};
