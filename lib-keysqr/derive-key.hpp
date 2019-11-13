#pragma once

#include "keysqr.h"
#include "key-derivation-options.hpp"

void generateKey(
  void* keyGeneratedOutput,
  size_t keyDerivationOutputLengthInBytes,
  const KeySqr<Face> &keySqr,
  const KeyDerivationOptions &keyDerivationOptions,
  const KeyDerivationOptionsJson::Purpose mandatePurpose = 
    KeyDerivationOptionsJson::_INVALID_PURPOSE_
);

void generateKey(
  std::vector<unsigned char> &keyGeneratedOutput,
  const KeySqr<Face> &keySqr,
  const KeyDerivationOptions &keyDerivationOptions,
  const KeyDerivationOptionsJson::Purpose mandatePurpose = 
    KeyDerivationOptionsJson::_INVALID_PURPOSE_
);