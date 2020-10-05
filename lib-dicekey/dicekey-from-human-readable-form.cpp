//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#include <vector>
#include <cassert>
#include "dicekey-from-human-readable-form.hpp"


const std::vector<Face> humanReadableFormToFaces(const std::string humanReadableForm) {
    assert((humanReadableForm.length() % NumberOfFaces) == 0);
    unsigned charsPerFace = humanReadableForm.length() / NumberOfFaces;
    assert (charsPerFace >= 2 && charsPerFace <= 3);
    std::vector<Face> faces;
    for (int i = 0; i < NumberOfFaces; i++) {
      faces.push_back(Face(humanReadableForm.substr(i * charsPerFace, charsPerFace)));
    }
    return faces;
}

DiceKeyFromString::DiceKeyFromString(const std::string humanReadableForm) :
  DiceKey(humanReadableFormToFaces(humanReadableForm)) {}
