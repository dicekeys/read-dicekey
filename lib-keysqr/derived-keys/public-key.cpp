#include "public-key.hpp"
#include "../../includes/json.hpp"
#include "convert.hpp"

namespace PublicKeyJsonFieldName {
  const std::string publicKeyBytesAsHexDigits = "publicKeyBytesAsHexDigits";
  const std::string keyDerivationOptionsJson = "keyDerivationOptionsJson";
}

PublicKey::PublicKey(
    const std::vector<unsigned char> publicKeyBytes,
    const std::string keyDerivationOptionsJson
  ) : publicKeyBytes(publicKeyBytes), keyDerivationOptionsJson(keyDerivationOptionsJson) {
    if (publicKeyBytes.size() != crypto_box_PUBLICKEYBYTES) {
      throw "Invalid key size exception";
    }
  }

PublicKey::PublicKey(std::string publicKeyAsJson) :
  PublicKey(create(publicKeyAsJson)) {}

PublicKey PublicKey::create(std::string publicKeyAsJson) {
  nlohmann::json jsonObject = nlohmann::json::parse(publicKeyAsJson);
  const std::string publicKeyBytesAsHexDigits = jsonObject.value<std::string>(
    PublicKeyJsonFieldName::publicKeyBytesAsHexDigits, "");
  const std::vector<unsigned char> publicKeyBytes = hexStrToByteVector(publicKeyBytesAsHexDigits);
  const std::string keyDerivationOptionsJson = jsonObject.value<std::string>(
    PublicKeyJsonFieldName::keyDerivationOptionsJson, ""
    );
  return PublicKey(publicKeyBytes, keyDerivationOptionsJson);
}

const std::string PublicKey::toJson(
  int indent,
  const char indent_char
) const {
	nlohmann::json asJson;  
  asJson[PublicKeyJsonFieldName::publicKeyBytesAsHexDigits] =
    getPublicKeyBytesAsHexDigits();
  asJson[PublicKeyJsonFieldName::keyDerivationOptionsJson] =
    keyDerivationOptionsJson;
  return asJson.dump(indent, indent_char);
};


const std::vector<unsigned char> PublicKey::seal(
  const unsigned char* message,
  const size_t messageLength,
  const std::vector<unsigned char> &publicKey
) {
  if (publicKey.size() != crypto_box_PUBLICKEYBYTES) {
    throw "Invalid key size exception";
  }
  if (messageLength <= 0) {
    throw "Invalid message length";
  }
  const size_t ciphertextLength =
    messageLength + crypto_box_SEALBYTES;
  std::vector<unsigned char> ciphertext(ciphertextLength);

  crypto_box_seal(
    ciphertext.data(),
    message,
    messageLength,
    publicKey.data()
  );

  return ciphertext;
}

const std::vector<unsigned char> seal(
  const SodiumBuffer &message,
  const std::vector<unsigned char> &publicKey
) {
  return PublicKey::seal(
    message.data, message.length, publicKey
  );
}

const std::vector<unsigned char> PublicKey::getPublicKeyBytes(
) const {
  return publicKeyBytes;
}

const std::string PublicKey::getPublicKeyBytesAsHexDigits(
) const {
  return toHexStr(publicKeyBytes);
}
