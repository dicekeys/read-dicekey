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
  const std::string& seed,
  const std::string &keyDerivationOptionsJson,
  const KeyDerivationOptionsJson::KeyType keyTypeExpected,
  const std::string &clientsApplicationId,
  size_t keyLengthInBytes
) :
  keyDerivationOptionsJson(keyDerivationOptionsJson),
  keyType(keyTypeExpected),
  derivedKey(
    validateAndGenerateKey(
      seed,
      keyDerivationOptionsJson,
      keyTypeExpected,
      clientsApplicationId,
      keyLengthInBytes
    )
  ) {}

// std::string removeOrientationCharacters(
//   std::string seedWithOrientationCharacters
// ) {
//     std::string seedWithoutOrientationCharacters = "";
//     int elements = seedWithOrientationCharacters.length / 3;
//     for (int e = 0; e < elements; e++) {
//       seedWithoutOrientationCharacters +=
//         seedWithOrientationCharacters[e*3] +
//         seedWithOrientationCharacters[(e*3)+1];
//     }
//     return seedWithoutOrientationCharacters;
// }

void KeySqrDerivedKey::generateKey(
  void* keyGeneratedOutput,
  size_t keyDerivationOutputLengthInBytes,
  const std::string& seed,
  const KeyDerivationOptions &keyDerivationOptions
) {
  ensureSodiumInitialized();
  if(keyDerivationOutputLengthInBytes != keyDerivationOptions.keyLengthInBytes) {
    throw std::bad_alloc();
  };
  // std::string seed =
  //   (keyDerivationOptions.includeOrientationOfFacesInKey || _seed.length < 75) ?
  //     _seed : removeOrientationCharacters(_seed);

  size_t slowHashPreimageLength =
    // length of the keysqr in human readable format
    seed.length() +
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
    seed.c_str(),
    seed.length()
  );
  slowHashPreimage[seed.length()] = '0';
  memcpy(
    slowHashPreimage + seed.length() + 1,
    keyDerivationOptions.keyDerivationOptionsJson.c_str(),
    keyDerivationOptions.keyDerivationOptionsJson.length()
  );

  const int nonZeroHashResultMeansOutOfMemoryError = keyDerivationOptions.hashFunction->hash(
    keyGeneratedOutput,
    slowHashPreimage,
    slowHashPreimageLength
  );

  // sodium_memzero(seed.c_str, seed.size());

  sodium_free(slowHashPreimage);

  if (nonZeroHashResultMeansOutOfMemoryError != 0) {
    throw std::bad_alloc();
  }

};


// We call this function to generate and write the key into memory so that the
// class instance can treat the key as a constant.
const SodiumBuffer KeySqrDerivedKey::validateAndGenerateKey(
  const std::string& seed,
  const std::string &keyDerivationOptionsJson,
  const KeyDerivationOptionsJson::KeyType keyTypeExpected,
  const std::string &clientsApplicationId,
  size_t keyLengthInBytes
) {
  std::string clientsApplicationIdWithTerminalDot = clientsApplicationId + std::string(
          (clientsApplicationId.length() == 0 || clientsApplicationId[clientsApplicationId.length()-1] == '.') ?
          "" : ".");
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
      const std::string prefixWithTerminalDot = prefix + std::string(
              (prefix.length() == 0 || prefix[prefix.length()-1] == '.') ?
              "" : ".");
      if (clientsApplicationIdWithTerminalDot.substr(0, prefixWithTerminalDot.size()) == prefixWithTerminalDot) {
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
    seed,
    keyDerivationOptions
  );
  return derivedKey;
}

const KeyDerivationOptions KeySqrDerivedKey::getKeyDerivationOptions() const {
  return KeyDerivationOptions(keyDerivationOptionsJson, keyType);
}

