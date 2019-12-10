#include <cassert>
#include <vector>

#include <sodium.h>

#include "../sodium-initializer.hpp"
#include "derived-key.hpp"

KeySqrDerivedKey::KeySqrDerivedKey(
  const KeySqr<Face> &keySqr,
  const KeyDerivationOptions &keyDerivationOptions,
  const std::string &clientsApplicationId,
  const KeyDerivationOptionsJson::Purpose mandatedPurpose,
  size_t keyLengthInBytes
) :
  keyDerivationOptions(keyDerivationOptions),
  derivedKey(
    validateAndGenerateKey(
      keySqr,
      keyDerivationOptions,
      clientsApplicationId,
      mandatedPurpose,
      keyLengthInBytes > 0 ? keyLengthInBytes : keyDerivationOptions.keyLengthInBytes
    )
  ) {}

KeySqrDerivedKey::KeySqrDerivedKey(
  const KeySqr<Face> &keySqr,
  const std::string &keyDerivationOptionsJson,
  const std::string &clientsApplicationId,
  const KeyDerivationOptionsJson::Purpose mandatedPurpose,
  size_t keyLengthInBytes
) : KeySqrDerivedKey(
  keySqr,
  KeyDerivationOptions(keyDerivationOptionsJson),
  clientsApplicationId, mandatedPurpose, keyLengthInBytes
) {}



void KeySqrDerivedKey::generateKey(
  void* keyGeneratedOutput,
  size_t keyDerivationOutputLengthInBytes,
  const KeySqr<Face> &keySqr,
  const KeyDerivationOptions &keyDerivationOptions
) {
  ensureSodiumInitialized();
  if(keyDerivationOutputLengthInBytes != keyDerivationOptions.keyLengthInBytes) {
    throw "Invalid length of key to generate";
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
    throw "Insufficient memory";
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
    throw "Insufficient memory";
  }

};


// We call this function to generate and write the key into memory so that the
// class instance can treat the key as a constant.
const SodiumBuffer KeySqrDerivedKey::validateAndGenerateKey(
  const KeySqr<Face> &keySqr,
  const KeyDerivationOptions &keyDerivationOptions,
  const std::string &clientsApplicationId,
  const KeyDerivationOptionsJson::Purpose mandatedPurpose,
  size_t keyLengthInBytes
) {
  // Ensure that the purpose in the key derivation options matches
  // the actual purpose
  if (
    mandatedPurpose != KeyDerivationOptionsJson::_INVALID_PURPOSE_ &&
    mandatedPurpose != keyDerivationOptions.purpose  
  ) {
    throw ("Key generation options must have purpose " + std::to_string(mandatedPurpose));
  }
  // Ensure that the application ID matches one of the prefixes
  if (keyDerivationOptions.restictToClientApplicationsIdPrefixes.size() > 0) {
    bool prefixFound = false;
    for (const std::string prefix : keyDerivationOptions.restictToClientApplicationsIdPrefixes) {
      if (clientsApplicationId.substr(0, prefix.size()) == prefix) {
        prefixFound = true;
        break;
      }
    }
    if (!prefixFound) {
      throw ("The client application is not allowed to use this key");
    }
  }

  SodiumBuffer derivedKey(crypto_secretbox_KEYBYTES);

  // Generate the key into the key array allocated above
  KeySqrDerivedKey::generateKey(
    derivedKey.data,
    derivedKey.length,
    keySqr,
    keyDerivationOptions
  );
  return derivedKey;
}
