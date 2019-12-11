//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#pragma once

#include <vector>
#include <cassert>
#include "face.hpp"
#include "keysqr.hpp"

/**
 * This class represents a KeySqr that has been read by scanning one or more images.
 */
class KeySqrFromString: public KeySqr<Face> {
  public:
    KeySqrFromString(const std::string humanReadableForm);
};
