#pragma once

#include <cassert>
#include "keysqr.h"
// #include "sodium.h"
#include "../includes/sodium.h"

#include "json.hpp"
// Must come after json.hpp
#include "../lib-keysqr/externally-generated/key-generation-parameters.hpp"
using json = nlohmann::json;

class HashFunction {
	public:
	virtual size_t hash_size_in_bytes() = 0;
	virtual int hash(
		unsigned char *hash_output,
		const unsigned char *message,
		unsigned long long message_length
	) = 0;

	void hash(
		std::vector<unsigned char>hash_output,
		const unsigned char *message,
		unsigned long long message_length
	) {
		assert(hash_output.size() == hash_size_in_bytes());
		hash(hash_output.data(), message, message_length);
	};

	void hash(
		std::vector<unsigned char>hash_output,
		const std::vector<unsigned char>message
	) {
		assert(hash_output.size() == hash_size_in_bytes());
		hash(hash_output.data(), message.data(), message.size());
	};
};

class HashFunctionBlake2b : public HashFunction {
	size_t hash_size_in_bytes() { return crypto_generichash_BYTES; }
	
	int hash(
		unsigned char *hash_output,
		const unsigned char *message,
		unsigned long long message_length
	) {
		return crypto_generichash(
			hash_output, crypto_generichash_BYTES,
			message, message_length,
			NULL, 0);
	}
};

class HashFunctionSHA256 : public HashFunction {
	size_t hash_size_in_bytes() { return crypto_hash_sha256_BYTES; }
	
	int hash(
		unsigned char *hash_output,
		const unsigned char *message,
		unsigned long long message_length
	) {
		return crypto_hash_sha256(hash_output,message, message_length);
	}
};

class HashFunctionArgon2id : public HashFunction {
	private:
		unsigned long long hash_output_size_in_bytes;
		unsigned long long opslimit;
		size_t memlimit;
	public:

	HashFunctionArgon2id(
		unsigned long long _hash_output_size_in_bytes,
		unsigned long long _opslimit,
		size_t _memlimit
	) :
		hash_output_size_in_bytes(_hash_output_size_in_bytes),
		opslimit(_opslimit),
		memlimit(_memlimit)
	{
		assert(
			hash_output_size_in_bytes >= crypto_pwhash_argon2id_BYTES_MIN &&
			hash_output_size_in_bytes <= crypto_pwhash_argon2id_BYTES_MAX
		);
	}

	size_t hash_size_in_bytes() { return hash_output_size_in_bytes; }
	
	int hash(
		unsigned char *hash_output,
		const unsigned char *message,
		unsigned long long message_length
	) {
		// Argon2id requires a 16-byte salt.
		// Since this is only used with messages that are already salted,
		// we use a salt of 16 zero bytes.
		static const unsigned char zero_bytes_for_salt[crypto_pwhash_argon2id_SALTBYTES] =
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		return crypto_pwhash(
			hash_output, hash_size_in_bytes(),
			(const char*)message, message_length,
			zero_bytes_for_salt,
			opslimit,
			memlimit,
			crypto_pwhash_ALG_ARGON2ID13
		);
	};
};

class KeyGenerationOptions {

public:
	KeyGenerationOptionsJson::Purpose purpose;
	KeyGenerationOptionsJson::KeyType keyType;
	std::vector<std::string> restictToClientApplicationsIdPrefixes;
	HashFunction *slowerHashFunction;
	HashFunction *fasterHashFunction;
	bool includeOrientationOfFacesInKey;

	~KeyGenerationOptions() {
		if (slowerHashFunction) {
			delete slowerHashFunction;
		}
		if (fasterHashFunction) {
			delete fasterHashFunction;
		}
	}

  KeyGenerationOptions(const std::string &keyGenerationOptionsJsonJson) {
    json kgoJson = json(keyGenerationOptionsJsonJson);

		//
		// Load purpose of key to be generated
		//
    purpose = kgoJson.value<KeyGenerationOptionsJson::Purpose>(
				KeyGenerationOptionsJson::FieldNames::purpose,
				KeyGenerationOptionsJson::Purpose::_INVALID_
			);

		if (purpose == KeyGenerationOptionsJson::Purpose::_INVALID_) {
			throw "Invalid purpose in KeyGenerationOptions";
		}

		//
		// Load keyType and validate match with purpose
		//
		keyType = kgoJson.value<KeyGenerationOptionsJson::KeyType>(
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

		unsigned int keyLengthInBytes = kgoJson.value<unsigned int>(
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

		restictToClientApplicationsIdPrefixes = kgoJson.value<const std::vector<std::string>>(
			KeyGenerationOptionsJson::FieldNames::restictToClientApplicationsIdPrefixes,
			// Default to empty list containing the empty string, which is a prefix of all strings
			{""}
		);

		// All salt is ignored.
		// const string additionalSalt = kgoJson.value<std::string>(KeyGenerationOptionsJson::FieldNames::additionalSalt, "");
		if (!kgoJson.contains(KeyGenerationOptionsJson::FieldNames::slowerHashFunction)) {
			slowerHashFunction = new HashFunctionSHA256();
		} else {
			const auto jslowerHashFunction = kgoJson.at(KeyGenerationOptionsJson::FieldNames::slowerHashFunction);
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

		if (!kgoJson.contains(KeyGenerationOptionsJson::FieldNames::fasterHashFunction)) {
			fasterHashFunction = new HashFunctionSHA256();
		} else {
			const auto jfasterHashFunction = kgoJson.at(KeyGenerationOptionsJson::FieldNames::fasterHashFunction);
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
    includeOrientationOfFacesInKey = kgoJson.value<bool>(
			KeyGenerationOptionsJson::FieldNames::includeOrientationOfFacesInKey,
			false
		);
  }

};

class KeySqrCrypto {
private:
  inline static bool hasInitializedSodium = false;
public:

  KeySqrCrypto()
  {
    if (!hasInitializedSodium) {
      if (sodium_init() < 0) {
        throw "Could not initialize sodium";
        /* panic! the library couldn't be initialized, it is not safe to use */
      }
      hasInitializedSodium = true;
    }
  }

  const std::vector<unsigned char> getSymmetricKey()
  {
    //
    std::vector<char> key(crypto_secretbox_KEYBYTES);
    // FIXME
  }

  const std::vector<char> seal(
    const std::vector<unsigned char> message
  ) {
    const size_t message_len = message.size();
    const size_t ciphertext_len = crypto_secretbox_MACBYTES + message.size();
    std::vector<unsigned char> ciphertext(ciphertext_len);

    const std::vector<unsigned char> key = getSymmetricKey();

    std::vector<unsigned char> nonce(crypto_secretbox_NONCEBYTES);
    randombytes_buf(nonce.data(), nonce.size());

    //crypto_secretbox_keygen(key.data());
    crypto_secretbox_easy(ciphertext.data(), message.data(), message.size(), nonce.data(), key.data());

    // nonce must be sent along with ciphertext.  why not put them together?

    std::vector<unsigned char> unsealed(ciphertext.size() - crypto_secretbox_MACBYTES);
    if (crypto_secretbox_open_easy(unsealed.data(), ciphertext.data(), ciphertext.size(), nonce.data(), key.data()) != 0) {
        /* message forged! */
    }

  }




};




class KeySqrCryptoWithDiceKey : public KeySqrCrypto {
private:
  const KeySqr<IFace> &keysqr;

  KeySqrCryptoWithDiceKey(KeySqr<IFace> &_keysqr) :
    keysqr(_keysqr),
    KeySqrCrypto()
  {
  }
};