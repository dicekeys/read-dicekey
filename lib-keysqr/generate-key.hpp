#pragma once

#include "keysqr.h"
#include "key-generation-options.hpp"

void generateKey(
  void* keyGeneratedOutput,
  size_t keyGenerationOutputLengthInBytes,
  const KeySqr<Face> &keySqr,
  const KeyGenerationOptions &keyGenerationOptions,
  const KeyGenerationOptionsJson::Purpose mandatePurpose = 
    KeyGenerationOptionsJson::_INVALID_PURPOSE_
);

void generateKey(
  std::vector<unsigned char> &keyGeneratedOutput,
  const KeySqr<Face> &keySqr,
  const KeyGenerationOptions &keyGenerationOptions,
  const KeyGenerationOptionsJson::Purpose mandatePurpose = 
    KeyGenerationOptionsJson::_INVALID_PURPOSE_
);