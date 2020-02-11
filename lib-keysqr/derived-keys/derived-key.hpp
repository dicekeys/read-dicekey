#pragma once

#include "../keysqr.hpp"
#include "sodium-buffer.hpp"
#include "key-derivation-options.hpp"

class KeySqrDerivedKey {
protected:
  const SodiumBuffer derivedKey;

public:
  const std::string keyDerivationOptionsJson;
  const KeyDerivationOptionsJson::KeyType keyType;

  KeySqrDerivedKey(
    const SodiumBuffer &derivedKey,
    const std::string &keyDerivationOptionsJson,
    const KeyDerivationOptionsJson::KeyType keyType
  );

  KeySqrDerivedKey(
    const KeySqr<Face> &keySqr,
    const std::string &keyDerivationOptionsJson,
    const KeyDerivationOptionsJson::KeyType keyType,
    const std::string &clientsApplicationId = "",
    size_t keyLengthInBytes = 0
  );

  const KeyDerivationOptions getKeyDerivationOptions() const;

  static void generateKey(
    void* keyGeneratedOutput,
    size_t keyDerivationOutputLengthInBytes,
    const KeySqr<Face> &keySqr,
    const KeyDerivationOptions &keyDerivationOptions
  );

  // We call this function to generate and write the key into memory so that the
  // class instance can treat the key as a constant.
  static const SodiumBuffer validateAndGenerateKey(
    const KeySqr<Face> &keySqr,
    const std::string &keyDerivationOptionsJson,
    const KeyDerivationOptionsJson::KeyType keyTypeExpected,
    const std::string &clientsApplicationId,
    size_t keyLengthInBytes
  );
};