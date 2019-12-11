#include <cassert>
#include "../../includes/json.hpp"
// Must come after json.hpp
#include "../externally-generated/key-derivation-parameters.hpp"

#include "decryption-restrictions.hpp"

DecryptionRestrictions::DecryptionRestrictions(
  const std::string &decryptionRestrictionsJson
) {
  // Use the nlohmann::json library to read the JSON-encoded
  // key generation options.
  nlohmann::json decryptionOptionsObject =
    nlohmann::json::parse(decryptionRestrictionsJson);

  clientApplicationIdMustHavePrefix =
    decryptionOptionsObject.value<const std::vector<std::string>>(
      DecryptionRestrictionsJson::FieldNames::clientApplicationIdMustHavePrefix,
      // Default to empty list containing the empty string, which is a prefix of all strings
      {""}
    );

  userMustAcknowledgeThisMessage =
    decryptionOptionsObject.value<std::string>(
      DecryptionRestrictionsJson::FieldNames::userMustAcknowledgeThisMessage,
      // Default to empty list containing the empty string, which is a prefix of all strings
      ""
    );
}

DecryptionRestrictions::DecryptionRestrictions(
		std::vector<std::string> clientApplicationIdMustHavePrefix,
		std::string userMustAcknowledgeThisMessage
) :
  clientApplicationIdMustHavePrefix(clientApplicationIdMustHavePrefix),
  userMustAcknowledgeThisMessage(userMustAcknowledgeThisMessage)
  {}

std::string	DecryptionRestrictions::toJson(int indent,
  const char indent_char
) const {
	nlohmann::json asJson;  
  asJson[DecryptionRestrictionsJson::FieldNames::userMustAcknowledgeThisMessage] =
    userMustAcknowledgeThisMessage;
  asJson[DecryptionRestrictionsJson::FieldNames::clientApplicationIdMustHavePrefix] =
    clientApplicationIdMustHavePrefix;
  return asJson.dump(indent, indent_char);
}

bool DecryptionRestrictions::isApplicationIdAllowed(const std::string &applicationId) const {
  if (clientApplicationIdMustHavePrefix.size() == 0) {
    // The applicationId is not required to match a prefix 
    return true;
  }
  // Check to see if the applicationId starts with one of the approved prefixes
  for (const std::string prefix : clientApplicationIdMustHavePrefix) {
    if (applicationId.substr(0, prefix.length()) == prefix) {
      // The applicationId is permitted as it matched this prefix Id
      return true;
    }
  }
  // The applicationId must start with one of the prefixes on the list,
  // but it does not.
  return false;
}

void DecryptionRestrictions::validateApplicationId(const std::string &applicationId) const {
  if (!isApplicationIdAllowed(applicationId)) {
    throw "Invalid application ID: " + applicationId;
  }
}
