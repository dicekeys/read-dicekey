#include <cassert>
#include <exception>
#include "../../includes/json.hpp"
// Must come after json.hpp
#include "../externally-generated/key-derivation-parameters.hpp"

#include "post-decryption-instructions.hpp"

PostDecryptionInstructions::PostDecryptionInstructions(
  const std::string &postDecryptionInstructionsJson
) {
  if (postDecryptionInstructionsJson.size() == 0) {
    // Empty post-decryption instructions
    return;
  }
  // Use the nlohmann::json library to read the JSON-encoded
  // key generation options.
  nlohmann::json decryptionOptionsObject =
    nlohmann::json::parse(postDecryptionInstructionsJson);

  clientApplicationIdMustHavePrefix =
    decryptionOptionsObject.value<const std::vector<std::string>>(
      PostDecryptionInstructionsJson::FieldNames::clientApplicationIdMustHavePrefix,
      // Default to empty list containing the empty string, which is a prefix of all strings
      {""}
    );

  userMustAcknowledgeThisMessage =
    decryptionOptionsObject.value<std::string>(
      PostDecryptionInstructionsJson::FieldNames::userMustAcknowledgeThisMessage,
      // Default to empty list containing the empty string, which is a prefix of all strings
      ""
    );
}

PostDecryptionInstructions::PostDecryptionInstructions(
		std::vector<std::string> clientApplicationIdMustHavePrefix,
		std::string userMustAcknowledgeThisMessage
) :
  clientApplicationIdMustHavePrefix(clientApplicationIdMustHavePrefix),
  userMustAcknowledgeThisMessage(userMustAcknowledgeThisMessage)
  {}

std::string	PostDecryptionInstructions::toJson(int indent,
  const char indent_char
) const {
	nlohmann::json asJson;  
  asJson[PostDecryptionInstructionsJson::FieldNames::userMustAcknowledgeThisMessage] =
    userMustAcknowledgeThisMessage;
  asJson[PostDecryptionInstructionsJson::FieldNames::clientApplicationIdMustHavePrefix] =
    clientApplicationIdMustHavePrefix;
  return asJson.dump(indent, indent_char);
}

bool PostDecryptionInstructions::isApplicationIdAllowed(const std::string &applicationId) const {
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

void PostDecryptionInstructions::validateApplicationId(const std::string &applicationId) const {
  if (!isApplicationIdAllowed(applicationId)) {
    throw std::exception( ("Invalid application ID: " + applicationId).c_str() );
  }
}