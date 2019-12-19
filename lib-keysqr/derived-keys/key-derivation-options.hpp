#pragma once

#pragma warning( disable : 26812 )
#include <cassert>
#include "hash-functions.hpp"
#include "../../includes/json.hpp"
// Must come after json.hpp
#include "../externally-generated/key-derivation-parameters.hpp"

/**
 * This class represents key generation options,
 * provided in JSON format, as an immutable class.
 */
class KeyDerivationOptions {

private:
	nlohmann::json keyDerivationOptionsExplicit;
public:
	const std::string keyDerivationOptionsJson;
	KeyDerivationOptionsJson::Purpose purpose;
	KeyDerivationOptionsJson::KeyType keyType;
  unsigned int keyLengthInBytes;
	std::vector<std::string> restictToClientApplicationsIdPrefixes;
	HashFunction *hashFunction;
	bool includeOrientationOfFacesInKey;

	~KeyDerivationOptions();

  /**
   * Create a KeyDerivationOptions class from the JSON representation
   * of the key generation options.
   **/
  KeyDerivationOptions(
		const std::string &keyDerivationOptionsJson
	);

	const void validate(
		const std::string applicationId,
		const KeyDerivationOptionsJson::Purpose mandatePurpose = KeyDerivationOptionsJson::Purpose::_INVALID_PURPOSE_
	) const;

	KeyDerivationOptions(
		const std::string &keyDerivationOptionsJson,
		const std::string applicationId,
		const KeyDerivationOptionsJson::Purpose mandatePurpose = KeyDerivationOptionsJson::Purpose::_INVALID_PURPOSE_
	) : KeyDerivationOptions(keyDerivationOptionsJson) {
		validate(applicationId, mandatePurpose);
	}


	const std::string jsonKeyDerivationOptionsWithAllOptionalParametersSpecified(
		int indent = -1,
	  const char indent_char = ' '
	) const;

};
