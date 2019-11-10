#pragma once

#include <cassert>
#include "hash-functions.hpp"
#include "json.hpp"
// Must come after json.hpp
#include "externally-generated/key-generation-parameters.hpp"

/**
 * This class represents key generation options,
 * provided in JSON format, as an immutable class.
 */
class KeyGenerationOptions {

private:
	nlohmann::json keyGenerationOptionsExplicit;
public:
	const std::string keyGenerationOptionsJsonString;
	KeyGenerationOptionsJson::Purpose purpose;
	KeyGenerationOptionsJson::KeyType keyType;
  unsigned int keyLengthInBytes;
	std::vector<std::string> restictToClientApplicationsIdPrefixes;
	HashFunction *hashFunction;
	bool includeOrientationOfFacesInKey;

	~KeyGenerationOptions();

  /**
   * Create a KeyGenerationOptions class from the JSON representation
   * of the key generation options.
   **/
  const KeyGenerationOptions(const std::string &keyGenerationOptionsJson);

	const std::string jsonKeyGenerationOptionsWithAllOptionalParametersSpecified(
		int indent = -1,
	  const char indent_char = ' '
	) const;

};
