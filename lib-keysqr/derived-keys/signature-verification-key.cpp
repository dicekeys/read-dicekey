#include "signature-verification-key.hpp"
#include "../../includes/json.hpp"
#include "convert.hpp"

namespace SignatureVerificationKeyJsonFieldName {
  const std::string verificationKeyBytesAsHexDigits = "verificationKeyBytesAsHexDigits";
  const std::string keyDerivationOptionsJson = "keyDerivationOptionsJson";
}

SignatureVerificationKey::SignatureVerificationKey(
    const std::vector<unsigned char> &_verificationKeyBytes,
    const std::string &_keyDerivationOptionsJson
  ) : verificationKeyBytes(_verificationKeyBytes), keyDerivationOptionsJson(_keyDerivationOptionsJson) {
    if (verificationKeyBytes.size() != crypto_sign_PUBLICKEYBYTES) {
      throw std::invalid_argument("Invalid key size exception");
    }
  }

//SignatureVerificationKey::SignatureVerificationKey(const std::string &verificationKeyAsJson) :
//  SignatureVerificationKey(create(verificationKeyAsJson)) {}
//
//SignatureVerificationKey SignatureVerificationKey::create(const std::string& verificationKeyAsJson) {
//  nlohmann::json jsonObject = nlohmann::json::parse(publicKeyAsJson);
//  const std::string verificationKeyBytesAsHexDigits = jsonObject.value<std::string>(
//    SignatureVerificationKeyJsonFieldName::verificationKeyBytesAsHexDigits, "");
//  const std::vector<unsigned char> verificationKeyBytes = hexStrToByteVector(verificationKeyBytesAsHexDigits);
//  const std::string keyDerivationOptionsJson = jsonObject.value<std::string>(
//    SignatureVerificationKeyJsonFieldName::keyDerivationOptionsJson, ""
//    );
//  return SignatureVerificationKey(verificationKeyBytes, keyDerivationOptionsJson);
//}

const std::string SignatureVerificationKey::toJson(
  int indent,
  const char indent_char
) const {
  nlohmann::json asJson;
  asJson[SignatureVerificationKeyJsonFieldName::verificationKeyBytesAsHexDigits] =
    getKeyBytesAsHexDigits();
  asJson[SignatureVerificationKeyJsonFieldName::keyDerivationOptionsJson] =
    keyDerivationOptionsJson;
  return asJson.dump(indent, indent_char);
}

const std::vector<unsigned char> SignatureVerificationKey::getKeyBytes(
) const {
  return verificationKeyBytes;
}

const std::string SignatureVerificationKey::getKeyBytesAsHexDigits(
) const {
  return toHexStr(verificationKeyBytes);
}


bool SignatureVerificationKey::verify(
  const std::vector<unsigned char>& signatureVerificationKey,
  const unsigned char* message,
  const size_t messageLength,
  const std::vector<unsigned char>& signature
) {
  // FIXME -verify that comparing to 0 is the correct thing to do here
  return crypto_sign_verify_detached(signature.data(), message, messageLength, signatureVerificationKey.data()) == 0;
}

bool SignatureVerificationKey::verify(
  const unsigned char* message,
  const size_t messageLength,
  const std::vector<unsigned char>& signature
) const {
  return verify(verificationKeyBytes, message, messageLength, signature);
}

bool SignatureVerificationKey::verify(
  const std::vector<unsigned char>& message,
  const std::vector<unsigned char>& signature
) const {
  return verify(verificationKeyBytes, message.data(), message.size(), signature);
}

bool SignatureVerificationKey::verify(
  const SodiumBuffer& message,
  const std::vector<unsigned char>& signature
) const {
  return verify(verificationKeyBytes, message.data, message.length, signature);
}