#pragma once

#include "keysqr.h"
#include "key-generation-options.hpp"

void generateKey(
  std::vector<unsigned char> keyGeneratedOutput,
  const KeySqr<Face> &keySqr,
  const KeyGenerationOptions &keyGenerationOptions
);