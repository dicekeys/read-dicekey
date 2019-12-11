#pragma once

#include "../keysqr.hpp"
#include "key-derivation-options.hpp"
#include "sodium-buffer.hpp"

class KeySqrDerivedKey {
protected:
  const SodiumBuffer derivedKey;

public:
  const std::string keyDerivationOptionsJson;

  // KeySqrDerivedKey(
  //   const KeySqr<Face> &keySqr,
  //   const KeyDerivationOptions &keyDerivationOptions,
  //   const std::string &clientsApplicationId = "",
  //   const KeyDerivationOptionsJson::Purpose mandatedPurpose = KeyDerivationOptionsJson::Purpose::_INVALID_PURPOSE_,
  //   size_t keyLengthInBytes = 0
  // );

  KeySqrDerivedKey(
    const SodiumBuffer &derivedKey,
    const std::string &keyDerivationOptionsJson
  );

  KeySqrDerivedKey(
    const KeySqr<Face> &keySqr,
    const std::string &keyDerivationOptionsJson,
    const std::string &clientsApplicationId = "",
    const KeyDerivationOptionsJson::Purpose mandatedPurpose = KeyDerivationOptionsJson::Purpose::_INVALID_PURPOSE_,
    size_t keyLengthInBytes = 0
  );

  ~KeySqrDerivedKey();

  const KeyDerivationOptions getKeyDerivationOptions() const;

  static void generateKey(
    void* keyGeneratedOutput,
    size_t keyDerivationOutputLengthInBytes,
    const KeySqr<Face> &keySqr,
    const KeyDerivationOptions &keyDerivationOptions
  );

  
  protected:

  // We call this function to generate and write the key into memory so that the
  // class instance can treat the key as a constant.
  static const SodiumBuffer validateAndGenerateKey(
    const KeySqr<Face> &keySqr,
    const std::string &keyDerivationOptionsJson,
    const std::string &clientsApplicationId,
    const KeyDerivationOptionsJson::Purpose mandatedPurpose,
    size_t keyLengthInBytes
  );
};