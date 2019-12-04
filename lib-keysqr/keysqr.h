//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#pragma once

#include <cassert>
#include <type_traits>
#include <vector>
#include "face.h"

const int NumberOfFaces = 25;

const std::vector<unsigned char> getRotationIndexesFor5x5Square(int clockwiseTurns);
std::string rotateHumanReadableForm(const std::string humanReadableForm, int clockwiseTurns);
const int rotationsToCanonicalForm(std::string humanReadableForm);

 /*
 * This class represents a KeySqr of 25 faces
 */
template<typename F, typename std::enable_if<std::is_base_of<Rotateable<F>, F>::value>::type* = nullptr>
class KeySqr {
  public:
    // const
    std::vector<F> faces;

    // KeySqr();

    KeySqr() {}

    KeySqr(const F &copyFrom) {
      faces = copyFrom.faces;
    }

    KeySqr(const std::vector<F> _faces) : faces(_faces) {
      if (_faces.size() != NumberOfFaces) {
	    	throw std::string("A KeySqr must contain " + std::to_string(NumberOfFaces) + " faces but only has " + std::to_string(_faces.size()));
	    }
      // faces = _faces;
    }

    bool isInitialized() const {
      return faces.size() == NumberOfFaces;
    }

    bool isDefined() const {
      if (!isInitialized()) {
        return false;
      }
      for (const F& face : faces) {
        if (!face.isDefined()) {
          return false;
        }
      }
      return true;
    }

    /**
     * Return the KeySqr as a JSON array of 25 faces, or an empty array
     * if the KeySqr was not initialized
     *
     * Each face follows the JSON format for Fs:
     *   inteface F {
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
    std::string toJson() const {
      if (!isInitialized()) {
        return "null";
      }
      std::string json = "[";
      for (int i = 0; i < NumberOfFaces; i++) {
        if (i!=0) {
          json += ",";
        }
        json += faces[i].toJson();
      }
      json += "]";
      return json;
    }

    /**
     * Convert a KeySqr to a human-readable format with 3 characters representing
     * each face as a letter, digit, and orientation.
     * 
     * FUTURE -- build string in place in memory that can be zerod
     */
    std::string toHumanReadableForm(bool includeFaceOrientations) const {
      assert(faces.size() == NumberOfFaces);
      std::string humanReadableForm = "";
      for (const F &face : faces) {
        humanReadableForm += face.toHumanReadableForm(includeFaceOrientations);
      }
      return humanReadableForm;
//      return facesToHumanReadableForm<F>(faces, includeFaceOrientations);
    }

    /**
     * Test to determine if every face in the KeySqr has a different (unique)
     * letter from all others.  If a letter were to appear twice, indicating the
     * same element (die or chip) appeared twice, we would conclude the key to be invalid.
     */
    bool areLettersUnique() const {
      if (!isInitialized()) {
        return false;
      }
      std::vector<char> letters;
      for (const F &face : faces) {
        letters.push_back(face.letter);
      }
      std::sort(letters.begin(), letters.end(), [](char a, char b) { return a < b; });
      for (int i = 0; i < NumberOfFaces; i++) {
        if (letters[i] != FaceLetters[i]) {
          return false;
        }
      }
      return true;
    }


  /**
   * Return a copy of this KeySqr that has been rotated by the specified
   * number of clockwise turns.
   */
  const KeySqr<F> rotate(int clockwiseTurns) const {
//    return KeySqr(rotateKeySqrFaces<F>(faces, clockwiseTurns));
    assert(faces.size() == NumberOfFaces);
    const unsigned clockwiseTurns0to3 = clockwiseTurnsToRange0To3(clockwiseTurns);
    const std::vector<unsigned char> &indexToMoveFaceFrom = getRotationIndexesFor5x5Square(clockwiseTurns0to3);
    std::vector<F> rotatedFaces;
    for (size_t i = 0; i < NumberOfFaces; i++) {
      rotatedFaces.push_back(faces[indexToMoveFaceFrom[i]].rotate(clockwiseTurns0to3));
    }
    return KeySqr<F>(rotatedFaces);
  }

  /**
   * Return a copy of this KeySqr rotated into the canonical orientation
   * where the corner with the earliest character in the alphabet is at the
   * top left of the 5x5 square.
   */
  const KeySqr rotateToCanonicalOrientation() const {
    return rotate(
      rotationsToCanonicalForm(toHumanReadableForm(true))
    );
//    return KeySqr(toCanonicalOrientation(faces));
  }

  bool isPotentialMatch(const KeySqr &other) const {
    int numMatchingFaces = 0;
    for (int i=0; i < NumberOfFaces; i++) {
      if (faces[i].equals(other.faces[i])) {
        numMatchingFaces++;
      } else {
        // faces don't match
        if (other.faces[i].errorSize() == 0 && faces[i].errorSize() == 0) {
          // The faces are different, but neither is supposed to be in error.
          // This means the entire grid must be different.  Either we're now
          // looking at another set of faces, the orientation rotated, or there
          // was an undetected face read error.
          return false;
        }
      }
    }
    return numMatchingFaces > 9;
  }

  /**
   * Merge an earlier-scanned KeySqr into this one if possible, so that we can
   * remove any errors that appear in the previous or current scan but appear to
   * be scanned without error in one of the two scans.
   **/
  const KeySqr<F> mergePrevious(const KeySqr<F> &previous) const {
    if (isPotentialMatch(previous)) {
      std::vector<F> newFaces;
      // There are enough matching faces in the previously-scanned KeySqr,
      // and no clear conflicts, so we can merge faces from the previous
      // KeySqr in cases where the face was scanned with fewer errors in the
      // past.
      for (int i=0; i < NumberOfFaces; i++) {
        // The face in this, the more-recent scan
        const F &face = faces[i];
        // The face from the previous scan
        const F &previousFace = previous.faces[i];

        newFaces.push_back((
            !previousFace.isDefined() ||
            (face.isDefined() && face.errorSize() <= previousFace.errorSize())
          ) ?
            face :
            previousFace
        );
      }
      return KeySqr<F>(newFaces);
    } else {
      // isPotentialMatch failed, so the previous scan was incompatible with the faces
      // from the current scan.  Perhaps the previous scan was at a different frame of
      // reference and the grid was rotated.
      for (int clockwiseTurns = 1; clockwiseTurns < 4; clockwiseTurns++) {
        // Since this rotation wasn't a potential match, try the 3 other potential
        // rotations and recurse a single time only if one is a match.
        // (where it will enter the above clause)
        KeySqr<F> rotatedKey = previous.rotate(clockwiseTurns);
        if (isPotentialMatch(rotatedKey)) {
          return mergePrevious(rotatedKey);
        }
      }
      // No match with previous, so just return new faces
      return KeySqr<F>(faces);
    }
  }

  /**
   * Calculate the sum of the errors over all faces.
   */
  unsigned int totalError() const
  {
    if (!isInitialized()) {
      return std::numeric_limits<unsigned int>::max();
    }
    unsigned int sumOfFaceErrors = 0;
    for (const F &face: faces) {
      sumOfFaceErrors += (unsigned int) face.errorSize();
    }
    return sumOfFaceErrors;
  }

  /**
   * Find the maximum error at any individual face
   * (or return max error if two faces have the same letter, indicating
   *  on of them must have a fatal read error because all elements should be unique)
   **/
  unsigned int maxError() const
  {
    if (!isInitialized()) {
      return std::numeric_limits<unsigned int>::max();
    }
    unsigned int max = 0;
    for (const F &face: faces) {
      if (face.errorSize() > max) {
        max = face.errorSize();
      }
    }
    return max;
  }

};
