#pragma once

#include "../keysqr.h"
#include "key-derivation-options.hpp"
#include "sodium-buffer.hpp"

class KeySqrDerivedKey {
  protected:

  const KeyDerivationOptions keyDerivationOptions;
  SodiumBuffer derivedKey;

  public:
  KeySqrDerivedKey(
    const KeySqr<Face> &keySqr,
    const KeyDerivationOptions &keyDerivationOptions,
    const std::string &clientsApplicationId = "",
    const KeyDerivationOptionsJson::Purpose mandatedPurpose = KeyDerivationOptionsJson::Purpose::_INVALID_PURPOSE_,
    size_t keyLengthInBytes = 0
  );

  KeySqrDerivedKey(
    const KeySqr<Face> &keySqr,
    const std::string &keyDerivationOptionsJson,
    const std::string &clientsApplicationId = "",
    const KeyDerivationOptionsJson::Purpose mandatedPurpose = KeyDerivationOptionsJson::Purpose::_INVALID_PURPOSE_,
    size_t keyLengthInBytes = 0
  );


  ~KeySqrDerivedKey();


  static void generateKey(
    void* keyGeneratedOutput,
    size_t keyDerivationOutputLengthInBytes,
    const KeySqr<Face> &keySqr,
    const KeyDerivationOptions &keyDerivationOptions
  );

  
  private:

  // We call this function to generate and write the key into memory so that the
  // class instance can treat the key as a constant.
  static const SodiumBuffer validateAndGenerateKey(
    const KeySqr<Face> &keySqr,
    const KeyDerivationOptions &keyDerivationOptions,
    const std::string &clientsApplicationId,
    const KeyDerivationOptionsJson::Purpose mandatedPurpose,
    size_t keyLengthInBytes
  );
};