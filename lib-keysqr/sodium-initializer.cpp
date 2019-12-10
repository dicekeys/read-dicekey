#pragma once

#include "sodium.h"
#include "sodium-initializer.hpp"

void ensureSodiumInitialized() {
  static bool hasInitializedSodium = false;
  if (!hasInitializedSodium) {
    if (sodium_init() < 0) {
      throw "Could not initialize sodium";
      /* panic! the library couldn't be initialized, it is not safe to use */
    }
    hasInitializedSodium = true;
  }
}
