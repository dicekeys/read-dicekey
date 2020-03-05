#include <cassert>
#include <exception>
#include "sodium.h"
#pragma warning( disable : 26812 )

#include "../keysqr.hpp"
#include "hash-functions.hpp"

#include "../../includes/json.hpp"
// Must come after json.hpp
#include "../externally-generated/key-derivation-parameters.hpp"

#include "key-derivation-options.hpp"

KeyDerivationOptions::~KeyDerivationOptions() {
  if (hashFunction) {
    delete hashFunction;
  }
}

KeyDerivationOptions::KeyDerivationOptions(
  const std::string &keyDerivationOptionsJson,
  const KeyDerivationOptionsJson::KeyType keyTypeExpected
):
  keyDerivationOptionsJson(keyDerivationOptionsJson)
{
  // Use the nlohmann::json library to read the JSON-encoded
  // key generation options.
  // We make heavy use of the library's enum conversion, as documented at:
  //   https://github.com/nlohmann/json#specializing-enum-conversion
  nlohmann::json keyDerivationOptionsObject;
  try {
    if (keyDerivationOptionsJson.size() > 0) {
      keyDerivationOptionsObject = nlohmann::json::parse(keyDerivationOptionsJson);
    } else {
      // A zero length string should create a completely default set of key derivation options
      keyDerivationOptionsObject = nlohmann::json::parse("{}");
    }
  } catch (nlohmann::json::exception e) {
    throw InvalidJsonKeyDerivationOptionsException(e.what());
  } catch (...) {
    throw InvalidJsonKeyDerivationOptionsException();
  }

  //
  // keyType
  //
  keyType = keyDerivationOptionsObject.value<KeyDerivationOptionsJson::KeyType>(
      KeyDerivationOptionsJson::FieldNames::keyType,
      keyTypeExpected
    );

  if (keyTypeExpected != KeyDerivationOptionsJson::KeyType::_INVALID_KEYTYPE_ &&
      keyType != keyTypeExpected) {
    // We were expecting keyType == keyTypeExpected since keyTypeExpected wasn't invalid,
    // but the JSON specified a different key type
    throw InvalidKeyDerivationOptionValueException("Unexpected keyType in KeyDerivationOptions");
  }

  if (keyType == KeyDerivationOptionsJson::KeyType::_INVALID_KEYTYPE_) {
    // No valid keyType was specified
    throw InvalidKeyDerivationOptionValueException("Invalid keyType in KeyDerivationOptions");
  }
  keyDerivationOptionsExplicit[KeyDerivationOptionsJson::FieldNames::keyType] = keyType;

  //
  // algorithm
  //
  algorithm = keyDerivationOptionsObject.value<KeyDerivationOptionsJson::Algorithm>(
    KeyDerivationOptionsJson::FieldNames::algorithm,
    // Default value depends on the purpose
    (keyType == KeyDerivationOptionsJson::KeyType::Symmetric) ?
        // For symmetric crypto, default to XSalsa20Poly1305
        KeyDerivationOptionsJson::Algorithm::XSalsa20Poly1305 :
    (keyType == KeyDerivationOptionsJson::KeyType::Public) ?
      // For public key crypto, default to X25519
      KeyDerivationOptionsJson::Algorithm::X25519 :
      // Otherwise, the leave the key setting to invalid (we don't care about a specific key type)
      KeyDerivationOptionsJson::Algorithm::_INVALID_ALGORITHM_
  );

  // Validate that the key type is allowed for this keyType
  if (keyType == KeyDerivationOptionsJson::KeyType::Symmetric &&
      algorithm != KeyDerivationOptionsJson::Algorithm::XSalsa20Poly1305
  ) {
    throw InvalidKeyDerivationOptionValueException(
      "Invalid algorithm type for symmetric key cryptography"
    );
  }

  if ( keyType == KeyDerivationOptionsJson::KeyType::Public &&
      algorithm != KeyDerivationOptionsJson::Algorithm::X25519
  ) {
    throw InvalidKeyDerivationOptionValueException(
      "Invalid algorithm type for public key cryptography"
    );
  }

  if (keyType != KeyDerivationOptionsJson::KeyType::_INVALID_KEYTYPE_) {
    keyDerivationOptionsExplicit[KeyDerivationOptionsJson::FieldNames::keyType] = keyType;
  }

  if (algorithm != KeyDerivationOptionsJson::Algorithm::_INVALID_ALGORITHM_) {
    keyDerivationOptionsExplicit[KeyDerivationOptionsJson::FieldNames::algorithm] = algorithm;
  }

  //
  // keyLengthInBytes
  //
  keyLengthInBytes =
    keyDerivationOptionsObject.value<unsigned int>(
      KeyDerivationOptionsJson::FieldNames::keyLengthInBytes,
      algorithm == KeyDerivationOptionsJson::Algorithm::X25519 ?
        crypto_box_SEEDBYTES :
      algorithm == KeyDerivationOptionsJson::Algorithm::XSalsa20Poly1305 ?
        // When a 256-bit (32 byte) key is needed, default to 32 bytes
        crypto_stream_xsalsa20_KEYBYTES :
        // When the key type is not defined, default to 32 bytes. 
        32
    );

  if (
    algorithm == KeyDerivationOptionsJson::Algorithm::X25519
    && keyLengthInBytes != crypto_box_SEEDBYTES
  ) {
    throw InvalidKeyDerivationOptionValueException( (
        "X25519 public key cryptography must use keyLengthInBytes of " +
        std::to_string(crypto_box_SEEDBYTES)
      ).c_str() );
  }
  if (
    algorithm == KeyDerivationOptionsJson::Algorithm::XSalsa20Poly1305 &&
    keyLengthInBytes != crypto_stream_xsalsa20_KEYBYTES
  ) {
    throw InvalidKeyDerivationOptionValueException( (
        "XSalsa20Poly1305 symmetric cryptography must use keyLengthInBytes of " +
        std::to_string(crypto_stream_xsalsa20_KEYBYTES)
      ).c_str() );
  }

	if (keyType == KeyDerivationOptionsJson::KeyType::Seed) {
		keyDerivationOptionsExplicit[KeyDerivationOptionsJson::FieldNames::keyLengthInBytes] = keyLengthInBytes;
	}

  restrictToClientApplicationsIdPrefixes =
    keyDerivationOptionsObject.value<const std::vector<std::string>>(
      KeyDerivationOptionsJson::FieldNames::restrictToClientApplicationsIdPrefixes,
      // Default to empty list containing the empty string, which is a prefix of all strings
      {""}
    );

	if (keyDerivationOptionsObject.contains(KeyDerivationOptionsJson::FieldNames::restrictToClientApplicationsIdPrefixes)) {
		keyDerivationOptionsExplicit[KeyDerivationOptionsJson::FieldNames::restrictToClientApplicationsIdPrefixes] = restrictToClientApplicationsIdPrefixes;
	}


  //
  // hashFunction
  //
  if (!keyDerivationOptionsObject.contains(KeyDerivationOptionsJson::FieldNames::hashFunction)) {
    hashFunction = new HashFunctionSHA256();    
    keyDerivationOptionsExplicit[KeyDerivationOptionsJson::FieldNames::hashFunction] = KeyDerivationOptionsJson::HashFunction::SHA256;
  } else {
    const auto jhashFunction = keyDerivationOptionsObject.at(KeyDerivationOptionsJson::FieldNames::hashFunction);
    keyDerivationOptionsExplicit[KeyDerivationOptionsJson::FieldNames::hashFunction] = jhashFunction;
    if (jhashFunction == KeyDerivationOptionsJson::HashFunction::SHA256) {
      hashFunction = new HashFunctionSHA256();
    } else if (jhashFunction == KeyDerivationOptionsJson::HashFunction::BLAKE2b) {
      hashFunction = new HashFunctionBlake2b();
    } else if (jhashFunction.is_object()) {
      const HashAlgorithmJson::Algorithm algorithm =
        jhashFunction.value<HashAlgorithmJson::Algorithm>(
          HashAlgorithmJson::FieldNames::algorithm,
          HashAlgorithmJson::Algorithm::_INVALID_ALGORITHM_
        );
      const unsigned long long opslimit =
        jhashFunction.value(
          HashAlgorithmJson::FieldNames::opsLimit,
          Argoin2idDefaults::opslimit
        );
      const size_t memlimit =
        jhashFunction.value(
          HashAlgorithmJson::FieldNames::memLimit,
          Argoin2idDefaults::memlimit
        );
      if (algorithm == HashAlgorithmJson::Algorithm::Argon2id) {
        hashFunction = new HashFunctionArgon2id(keyLengthInBytes, opslimit, memlimit);
      } else if (algorithm == HashAlgorithmJson::Algorithm::Scrypt) {
        hashFunction = new HashFunctionScrypt(keyLengthInBytes, opslimit, memlimit);
      } else {
        throw std::invalid_argument("Invalid hashFunction");
      }
      keyDerivationOptionsExplicit[KeyDerivationOptionsJson::FieldNames::hashFunction] = {
        {HashAlgorithmJson::FieldNames::algorithm, algorithm},
        {HashAlgorithmJson::FieldNames::memLimit, memlimit},
        {HashAlgorithmJson::FieldNames::opsLimit, opslimit}
      };
    } else {
      throw std::invalid_argument("Invalid hashFunction");
    }
  }

  //
  // includeOrientationOfFacesInKey
  //
  includeOrientationOfFacesInKey = keyDerivationOptionsObject.value<bool>(
    KeyDerivationOptionsJson::FieldNames::includeOrientationOfFacesInKey,
    true
  );
  keyDerivationOptionsExplicit[KeyDerivationOptionsJson::FieldNames::includeOrientationOfFacesInKey] =
    includeOrientationOfFacesInKey;
  //
  // additionalSalt
  //
  // There's no need to read in the additionalSalt string, as it's already part
  // of the KeyDerivationOptions json string from which keys are generated.
  // const string additionalSalt = keyDerivationOptionsObject.value<std::string>(
  // 		KeyDerivationOptionsJson::FieldNames::additionalSalt, "");
  if (keyDerivationOptionsObject.contains(KeyDerivationOptionsJson::FieldNames::additionalSalt)) {
    keyDerivationOptionsExplicit[KeyDerivationOptionsJson::FieldNames::additionalSalt] =
      keyDerivationOptionsObject[KeyDerivationOptionsJson::FieldNames::additionalSalt];
  }

};


const void KeyDerivationOptions::validate(
  const std::string applicationId
) const {
  if (restrictToClientApplicationsIdPrefixes.size() > 0) {
    bool prefixFound = false;
    for (const std::string prefix : restrictToClientApplicationsIdPrefixes) {
      if (applicationId.substr(0, prefix.length()) == prefix) {
        prefixFound = true;
        break;
      }
    }
    if (!prefixFound) {
      throw ClientNotAuthorizedException("The client application is not allowed to use this key");
    }
    bool noneMatched = true;
  }
}



const std::string KeyDerivationOptions::jsonKeyDerivationOptionsWithAllOptionalParametersSpecified(
  int indent,
  const char indent_char
) const {
  return keyDerivationOptionsExplicit.dump(indent, indent_char);
}
