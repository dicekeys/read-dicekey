//////////////////////////////////////////////////////////////////
// KeyGenerationOptions JSON Specification
// - Automatically-generated file - NOT TO BE MODIFIED DIRECTLY)
//////////////////////////////////////////////////////////////////
//
// This c++ header file specifies the JSON parameter names
// for KeyGenerationOptions.
#pragma once

#include <string>

namespace KeyGenerationOptionsJson {
	namespace FieldNames {
		const std::string additionalSalt = "additionalSalt";
		const std::string hashFunction = "hashFunction";
		const std::string includeOrientationOfFacesInKey = "includeOrientationOfFacesInKey";
		const std::string keyLengthInBytes = "keyLengthInBytes";
		const std::string keyType = "keyType";
		const std::string purpose = "purpose";
		const std::string restictToClientApplicationsIdPrefixes = "restictToClientApplicationsIdPrefixes";
	}

	enum Purpose {
		_INVALID_PURPOSE_ = 0,
		ForApplicationUse,
		ForSymmetricKeySealedMessages,
		ForPublicKeySealedMesssages,
		ForPublicKeySealedMesssagesWithRestrictionsEnforcedPostDecryption
	};
	NLOHMANN_JSON_SERIALIZE_ENUM( Purpose, {
		{Purpose::_INVALID_PURPOSE_, nullptr},
		{Purpose::ForApplicationUse, "ForApplicationUse"},
		{Purpose::ForSymmetricKeySealedMessages, "ForSymmetricKeySealedMessages"},
		{Purpose::ForPublicKeySealedMesssages, "ForPublicKeySealedMesssages"},
		{Purpose::ForPublicKeySealedMesssagesWithRestrictionsEnforcedPostDecryption, "ForPublicKeySealedMesssagesWithRestrictionsEnforcedPostDecryption"}
	})
	

	enum KeyType {
		_INVALID_KEYTYPE_ = 0,
		XSalsa20Poly1305,
		X25519
	};
	NLOHMANN_JSON_SERIALIZE_ENUM( KeyType, {
		{KeyType::_INVALID_KEYTYPE_, nullptr},
		{KeyType::XSalsa20Poly1305, "XSalsa20Poly1305"},
		{KeyType::X25519, "X25519"}
	})
	

	enum HashFunction {
		_INVALID_HASHFUNCTION_ = 0,
		BLAKE2b,
		SHA256
	};
	NLOHMANN_JSON_SERIALIZE_ENUM( HashFunction, {
		{HashFunction::_INVALID_HASHFUNCTION_, nullptr},
		{HashFunction::BLAKE2b, "BLAKE2b"},
		{HashFunction::SHA256, "SHA256"}
	})
	

};

namespace HashAlgorithmJson {
	namespace FieldNames {
		const std::string algorithm = "algorithm";
		const std::string memLimit = "memLimit";
		const std::string opsLimit = "opsLimit";
	}	
	enum Algorithm {
		_INVALID_ALGORITHM_ = 0,
		Argon2id,
		Scrypt
	};
	NLOHMANN_JSON_SERIALIZE_ENUM( Algorithm, {
		{Algorithm::_INVALID_ALGORITHM_, nullptr},
		{Algorithm::Argon2id, "Argon2id"},
		{Algorithm::Scrypt, "Scrypt"}
	})
	
};

namespace Argoin2idDefaults {
	const unsigned long long opslimit = 2;
	const size_t memlimit = 67108864;
}

