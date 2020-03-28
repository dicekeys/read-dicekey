#pragma once

#include "sodium-buffer.hpp"
#include "key-derivation-options.hpp"


class CryptographicVerificationFailure: public std::invalid_argument
{
	public:
	CryptographicVerificationFailure(const char* what) :
		std::invalid_argument(what) {};
};

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
    const std::string& seed,
    const std::string &keyDerivationOptionsJson,
    const KeyDerivationOptionsJson::KeyType keyType,
    const std::string &clientsApplicationId = "",
    size_t keyLengthInBytes = 0
  );

  const KeyDerivationOptions getKeyDerivationOptions() const;

  static void generateKey(
    void* keyGeneratedOutput,
    size_t keyDerivationOutputLengthInBytes,
    const std::string& seed,
    const KeyDerivationOptions &keyDerivationOptions
  );

  // We call this function to generate and write the key into memory so that the
  // class instance can treat the key as a constant.
  static const SodiumBuffer validateAndGenerateKey(
    const std::string& seed,
    const std::string &keyDerivationOptionsJson,
    const KeyDerivationOptionsJson::KeyType keyTypeExpected,
    const std::string &clientsApplicationId,
    size_t keyLengthInBytes
  );
};