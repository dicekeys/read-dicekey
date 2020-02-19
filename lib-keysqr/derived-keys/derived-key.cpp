#include <cassert>
#include <exception>
#include <vector>

#include <sodium.h>

#include "sodium-initializer.hpp"
#include "derived-key.hpp"

KeySqrDerivedKey::KeySqrDerivedKey(
  const SodiumBuffer &derivedKey,
  const std::string &keyDerivationOptionsJson,
  const KeyDerivationOptionsJson::KeyType keyType
) :
  derivedKey(derivedKey),
  keyType(keyType),
  keyDerivationOptionsJson(keyDerivationOptionsJson)
{}


KeySqrDerivedKey::KeySqrDerivedKey(
  const KeySqr<Face> &keySqr,
  const std::string &keyDerivationOptionsJson,
  const KeyDerivationOptionsJson::KeyType keyTypeExpected,
  const std::string &clientsApplicationId,
  size_t keyLengthInBytes
) :
  keyDerivationOptionsJson(keyDerivationOptionsJson),
  keyType(keyType),
  derivedKey(
    validateAndGenerateKey(
      keySqr,
      keyDerivationOptionsJson,
      keyTypeExpected,
      clientsApplicationId,
      keyLengthInBytes
    )
  ) {}


void KeySqrDerivedKey::generateKey(
  void* keyGeneratedOutput,
  size_t keyDerivationOutputLengthInBytes,
  const KeySqr<Face> &keySqr,
  const KeyDerivationOptions &keyDerivationOptions
) {
  ensureSodiumInitialized();
  if(keyDerivationOutputLengthInBytes != keyDerivationOptions.keyLengthInBytes) {
    throw std::bad_alloc();
  };
  std::string keySqrInHumanReadableForm =
    keySqr.toHumanReadableForm(keyDerivationOptions.includeOrientationOfFacesInKey);

  size_t slowHashPreimageLength =
    // length of the keysqr in human readable format
    keySqrInHumanReadableForm.length() +
    // 1 character for a null char between the two strings
    1 +
    // length of the json string specifying the key generation options
    keyDerivationOptions.keyDerivationOptionsJson.length();

  unsigned char *slowHashPreimage = (unsigned char*)sodium_malloc(slowHashPreimageLength);
  if (slowHashPreimage == NULL) {
    throw std::bad_alloc();
  }

  memcpy(
    slowHashPreimage,
    keySqrInHumanReadableForm.c_str(),
    keySqrInHumanReadableForm.length()
  );
  keySqrInHumanReadableForm[keySqrInHumanReadableForm.length()] = '0';
  memcpy(
    slowHashPreimage + keySqrInHumanReadableForm.length() + 1,
    keyDerivationOptions.keyDerivationOptionsJson.c_str(),
    keyDerivationOptions.keyDerivationOptionsJson.length()
  );

  const int nonZeroHashResultMeansOutOfMemoryError = keyDerivationOptions.hashFunction->hash(
    keyGeneratedOutput,
    slowHashPreimage,
    slowHashPreimageLength
  );

  // sodium_memzero(keySqrInHumanReadableForm.c_str, keySqrInHumanReadableForm.size());

  sodium_free(slowHashPreimage);

  if (nonZeroHashResultMeansOutOfMemoryError != 0) {
    throw std::bad_alloc();
  }

};


// We call this function to generate and write the key into memory so that the
// class instance can treat the key as a constant.
const SodiumBuffer KeySqrDerivedKey::validateAndGenerateKey(
  const KeySqr<Face> &keySqr,
  const std::string &keyDerivationOptionsJson,
  const KeyDerivationOptionsJson::KeyType keyTypeExpected,
  const std::string &clientsApplicationId,
  size_t keyLengthInBytes
) {
  const KeyDerivationOptions keyDerivationOptions(keyDerivationOptionsJson, keyTypeExpected);
  // Ensure that the purpose in the key derivation options matches
  // the actual purpose
  if (
    keyTypeExpected != KeyDerivationOptionsJson::KeyType::_INVALID_KEYTYPE_ &&
    keyTypeExpected != keyDerivationOptions.keyType  
  ) {
    throw InvalidKeyDerivationOptionValueException( (
      "Key generation options must have field " + std::to_string(keyTypeExpected)
    ).c_str() );
  }
  // Ensure that the application ID matches one of the prefixes
  if (keyDerivationOptions.restrictToClientApplicationsIdPrefixes.size() > 0) {
    bool prefixFound = false;
    for (const std::string prefix : keyDerivationOptions.restrictToClientApplicationsIdPrefixes) {
      if (clientsApplicationId.substr(0, prefix.size()) == prefix) {
        prefixFound = true;
        break;
      }
    }
    if (!prefixFound) {
      throw ClientNotAuthorizedException();
    }
  }

  if (keyLengthInBytes == 0) {
    keyLengthInBytes = keyDerivationOptions.keyLengthInBytes;
  } else if (keyLengthInBytes != keyDerivationOptions.keyLengthInBytes) {
    throw InvalidKeyDerivationOptionValueException( (
      "Key length in bytes for this keyType should be " + std::to_string(keyLengthInBytes) +
       " but keyLengthInBytes field was set to " + std::to_string(keyDerivationOptions.keyLengthInBytes)
      ).c_str()
    );
  }
  SodiumBuffer derivedKey(keyLengthInBytes);

  // Generate the key into the key array allocated above
  KeySqrDerivedKey::generateKey(
    derivedKey.data,
    derivedKey.length,
    keySqr,
    keyDerivationOptions
  );
  return derivedKey;
}

const KeyDerivationOptions KeySqrDerivedKey::getKeyDerivationOptions() const {
  return KeyDerivationOptions(keyDerivationOptionsJson, keyType);
}

