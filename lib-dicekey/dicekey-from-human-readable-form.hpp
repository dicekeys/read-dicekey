//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#pragma once

#include <vector>
#include <cassert>
#include "face.hpp"
#include "dicekey.hpp"

/**
 * This class represents a DiceKey that has been read by scanning one or more images.
 */
class DiceKeyFromString: public DiceKey<Face> {
  public:
    DiceKeyFromString(const std::string humanReadableForm);
};
