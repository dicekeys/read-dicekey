#pragma once

#include <cassert>
#include "hash-functions.hpp"
#include "../../includes/json.hpp"
// Must come after json.hpp
#include "../externally-generated/key-derivation-parameters.hpp"

/**
 * This class represents key generation options,
 * provided in JSON format, as an immutable class.
 */
class DecryptionRestrictions {

public:
	std::vector<std::string> clientApplicationIdMustHavePrefix;
	std::string userMustAcknowledgeThisMessage;

  /**
   * Create a DecryptionRestrictions class from the JSON representation
   * of the key generation options.
   **/
  DecryptionRestrictions(
		const std::string &decryptionRestrictionsJson
	);

	DecryptionRestrictions(
		std::vector<std::string> clientApplicationIdMustHavePrefix,
		std::string userMustAcknowledgeThisMessage = ""
	);

	bool isApplicationIdAllowed(const std::string &applicationId) const;
	void validateApplicationId(const std::string &applicationId) const;

	std::string	toJson(
		int indent = -1,
	  const char indent_char = ' '
	) const;
};
