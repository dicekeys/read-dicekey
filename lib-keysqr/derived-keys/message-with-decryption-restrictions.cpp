#include "message-with-decryption-restrictions.hpp"

MessageWithDecryptionRestrictions::MessageWithDecryptionRestrictions(
  const SodiumBuffer &buffer
): SodiumBuffer(buffer) {}

MessageWithDecryptionRestrictions::MessageWithDecryptionRestrictions(
  const SodiumBuffer &message,
  const std::string &decryptionRestrictionsJson
):
  SodiumBuffer(decryptionRestrictionsJson.size() + 1 + message.length) 
{
  memcpy_s(
    data, length,
    decryptionRestrictionsJson.data(), decryptionRestrictionsJson.size()
  );
  data[decryptionRestrictionsJson.size()] = 0;
  memcpy_s(
    data + decryptionRestrictionsJson.size() + 1,
    length - (decryptionRestrictionsJson.size() + 1),
    message.data,
    message.length
  );
}

MessageWithDecryptionRestrictions::MessageWithDecryptionRestrictions(
  const SodiumBuffer &message,
  const DecryptionRestrictions &decryptionRestrictions
): MessageWithDecryptionRestrictions(message, decryptionRestrictions.toJson())
{}


MessageWithDecryptionRestrictions::MessageWithDecryptionRestrictions(
  const unsigned char* message,
  const size_t messageLength,
  const std::string &decryptionRestrictionsJson
): MessageWithDecryptionRestrictions(SodiumBuffer(messageLength, message), decryptionRestrictionsJson)
{}

MessageWithDecryptionRestrictions::MessageWithDecryptionRestrictions(
  const unsigned char* message,
  const size_t messageLength,
  const DecryptionRestrictions &decryptionRestrictions
): MessageWithDecryptionRestrictions(SodiumBuffer(messageLength, message), decryptionRestrictions.toJson())
{}

const std::string MessageWithDecryptionRestrictions::getDecryptionRestrictionsJson() const {
  const size_t jsonLength = strnlen_s((const char*)data, length); 
  return std::string( (const char*)data, jsonLength );
}

const DecryptionRestrictions MessageWithDecryptionRestrictions::getDecryptionRestrictions() const {
  return DecryptionRestrictions(getDecryptionRestrictionsJson());
}

const std::vector<unsigned char> MessageWithDecryptionRestrictions::seal(
  const GlobalPublicKey &publicKey
) const {
  return PublicKey::seal(
    data,
    length,
    publicKey.getPublicKeyBytes()
  );
};

const std::vector<unsigned char> MessageWithDecryptionRestrictions::seal(
  const std::vector<unsigned char> &publicKeyBytes
) const {
  return PublicKey::seal(
    data,
    length,
    publicKeyBytes
  );
}

const SodiumBuffer MessageWithDecryptionRestrictions::getPlaintext() const {
  const size_t prefixLength = strnlen_s((const char*)data, length)  + 1;
  const  size_t plaintextLength = prefixLength > length ? 0 : length - prefixLength;
  return SodiumBuffer(plaintextLength, data + length - plaintextLength);
};
