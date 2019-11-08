#pragma once

#include <cassert>
#include "keysqr.h"

// #include "sodium.h"
#include "../includes/sodium.h"
#include "hash-functions.hpp"

#include "json.hpp"
// Must come after json.hpp
#include "externally-generated/key-generation-parameters.hpp"

#include "key-generation-options.hpp"


KeyGenerationOptions::~KeyGenerationOptions() {
  if (slowerHashFunction) {
    delete slowerHashFunction;
  }
  if (fasterHashFunction) {
    delete fasterHashFunction;
  }
}

KeyGenerationOptions::KeyGenerationOptions(const std::string &keyGenerationOptionsJson):
  keyGenerationOptionsJsonString(keyGenerationOptionsJson)
{
  // Use the nlohmann::json library to read the JSON-encoded
  // key generation options.
  // We make heavy use of the library's enum conversion, as documented at:
  //   https://github.com/nlohmann/json#specializing-enum-conversion
  nlohmann::json keyGenerationOptionsObject =
    nlohmann::json(keyGenerationOptionsJsonString);

  //
  // purpose (the purpose of the key to be generated)
  //
  purpose = keyGenerationOptionsObject.value<KeyGenerationOptionsJson::Purpose>(
      KeyGenerationOptionsJson::FieldNames::purpose,
      KeyGenerationOptionsJson::Purpose::_INVALID_
    );

  if (purpose == KeyGenerationOptionsJson::Purpose::_INVALID_) {
    throw "Invalid purpose in KeyGenerationOptions";
  }

  //
  // keyType
  //
  keyType = keyGenerationOptionsObject.value<KeyGenerationOptionsJson::KeyType>(
    KeyGenerationOptionsJson::FieldNames::keyType,
    // Default value depends on the purpose
    (purpose == KeyGenerationOptionsJson::Purpose::ForSymmetricKeySealedMessages) ?
        // For symmetric crypto, default to XSalsa20Poly1305
        KeyGenerationOptionsJson::KeyType::XSalsa20Poly1305 :
    (	
      purpose == KeyGenerationOptionsJson::Purpose::ForPublicKeySealedMesssages ||
      purpose == KeyGenerationOptionsJson::Purpose::ForPublicKeySealedMesssagesWithRestrictionsEnforcedPostDecryption
    ) ?
      // For public key crypto, default to X25519
      KeyGenerationOptionsJson::KeyType::X25519 :
      // Otherwise, the leave the key setting to invalid (we don't care about a specific key type)
      KeyGenerationOptionsJson::KeyType::_INVALID_
  );

  // Validate that the key type is allowed for this purpose
  if (purpose == KeyGenerationOptionsJson::Purpose::ForSymmetricKeySealedMessages &&
      keyType != KeyGenerationOptionsJson::KeyType::XSalsa20Poly1305
  ) {
    throw "Invalid key type for symmetric key cryptography";
  }

  if ( (	
        purpose == KeyGenerationOptionsJson::Purpose::ForPublicKeySealedMesssages ||
        purpose == KeyGenerationOptionsJson::Purpose::ForPublicKeySealedMesssagesWithRestrictionsEnforcedPostDecryption
      ) &&
      keyType != KeyGenerationOptionsJson::KeyType::X25519
  ) {
    throw "Invalid key type for public key cryptography";
  }

  //
  // keyLengthInBytes
  //
  keyLengthInBytes =
    keyGenerationOptionsObject.value<unsigned int>(
      KeyGenerationOptionsJson::FieldNames::keyLengthInBytes,
      (
        keyType == KeyGenerationOptionsJson::KeyType::X25519 ||
        keyType == KeyGenerationOptionsJson::KeyType::XSalsa20Poly1305
      ) ?
        // When a 256-bit (32 byte) key is needed, default to 32 bytes
        32 :
        // When the key type is not defined, default to 32 bytes. 
        32
    );

  if ( (
      keyType == KeyGenerationOptionsJson::KeyType::X25519 ||
      keyType == KeyGenerationOptionsJson::KeyType::XSalsa20Poly1305
    ) && keyLengthInBytes != 32
  ) {
    throw "Invalid keyLengthInBytes for this key type";
  }

  restictToClientApplicationsIdPrefixes =
    keyGenerationOptionsObject.value<const std::vector<std::string>>(
      KeyGenerationOptionsJson::FieldNames::restictToClientApplicationsIdPrefixes,
      // Default to empty list containing the empty string, which is a prefix of all strings
      {""}
    );

  //
  // slowerHashFunction
  //
  if (!keyGenerationOptionsObject.contains(KeyGenerationOptionsJson::FieldNames::slowerHashFunction)) {
    slowerHashFunction = new HashFunctionSHA256();
  } else {
    const auto jslowerHashFunction = keyGenerationOptionsObject.at(KeyGenerationOptionsJson::FieldNames::slowerHashFunction);
    if (jslowerHashFunction == KeyGenerationOptionsJson::HashFunction::SHA256) {
      slowerHashFunction = new HashFunctionSHA256();
    } else if (jslowerHashFunction == KeyGenerationOptionsJson::HashFunction::BLAKE2b) {
      slowerHashFunction = new HashFunctionBlake2b();
    } else if (jslowerHashFunction.is_object()) {
      const HashAlgorithmJson::Algorithm algorithm =
        jslowerHashFunction.value<HashAlgorithmJson::Algorithm>(
          HashAlgorithmJson::FieldNames::algorithm,
          HashAlgorithmJson::Algorithm::_INVALID_
        );
      if (algorithm == HashAlgorithmJson::Algorithm::Argon2id) {
          const unsigned long long opslimit =
            jslowerHashFunction.value(
              HashAlgorithmJson::FieldNames::opsLimit,
              Argoin2idDefaults::opslimit
            );
          const size_t memlimit =
            jslowerHashFunction.value(
              HashAlgorithmJson::FieldNames::memLimit,
              Argoin2idDefaults::memlimit
            );
          slowerHashFunction = new HashFunctionArgon2id(keyLengthInBytes, opslimit, memlimit);
      } else {
        throw "Invalid slowerHashFunction";
      }
    } else {
      throw "Invalid slowerHashFunction";
    }
  }

  //
  // fasterHashFunction
  //
  if (!keyGenerationOptionsObject.contains(KeyGenerationOptionsJson::FieldNames::fasterHashFunction)) {
    fasterHashFunction = new HashFunctionSHA256();
  } else {
    const auto jfasterHashFunction = keyGenerationOptionsObject.at(KeyGenerationOptionsJson::FieldNames::fasterHashFunction);
    if (jfasterHashFunction == KeyGenerationOptionsJson::HashFunction::SHA256) {
      fasterHashFunction = new HashFunctionSHA256();
    } else if (jfasterHashFunction == KeyGenerationOptionsJson::HashFunction::BLAKE2b) {
      fasterHashFunction = new HashFunctionBlake2b();
    } else {
      throw "Invalid fasterHashFunction";
    }
  }

  //
  // includeOrientationOfFacesInKey
  //
  includeOrientationOfFacesInKey = keyGenerationOptionsObject.value<bool>(
    KeyGenerationOptionsJson::FieldNames::includeOrientationOfFacesInKey,
    false
  );

  //
  // additionalSalt
  //
  // There's no need to read in the additionalSalt string, as it's already part
  // of the KeyGenerationOptions json string from which keys are generated.
  // const string additionalSalt = keyGenerationOptionsObject.value<std::string>(
  // 		KeyGenerationOptionsJson::FieldNames::additionalSalt, "");

};
