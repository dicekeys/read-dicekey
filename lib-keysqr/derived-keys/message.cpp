#define __STDC_WANT_LIB_EXT1__ 1
#include "message.hpp"
#include <string.h>

size_t safeStrLength(const unsigned char *str, size_t maxLength) {
  size_t i = 0;
  while (i < maxLength && str[i] != '\0') i++;
  return i;
}

Message::Message(
  const Message &other
) : SodiumBuffer(other) {}

Message::Message(
  const SodiumBuffer &buffer,
  const bool isAlreadyEncoded
): SodiumBuffer(
  isAlreadyEncoded ?
  // We're just copying in a buffer here
  buffer:
  // We need to prepend the buffer to be copied in with 1 to indicate
  // that there are no post-decryption instructions
  SodiumBuffer(buffer.length + 1)  
) {
  if (!isAlreadyEncoded) {
    // Encode a 0 at the first byte to indicate there are
    // no post-decryption instructions
    data[0] = '\0';
    memcpy(data + 1, buffer.data, buffer.length);
  }
}


const Message Message::reconstituteFromEncodedBuffer(
  const SodiumBuffer &buffer
) {
  return Message(buffer, true);
};

const Message Message::createFromPlaintextBuffer(
  const SodiumBuffer &buffer
) {
  return Message(buffer, false);
};

Message::Message(
  const SodiumBuffer &message,
  const std::string &postDecryptionInstructionsJson
):
  SodiumBuffer(postDecryptionInstructionsJson.size() + 1 + message.length) 
{
  memcpy(
    data, postDecryptionInstructionsJson.data(), postDecryptionInstructionsJson.size()
  );
  data[postDecryptionInstructionsJson.size()] = 0;
  memcpy(
    data + postDecryptionInstructionsJson.size() + 1,
    message.data,
    message.length
  );
}

Message::Message(
  const SodiumBuffer &message,
  const PostDecryptionInstructions &postDecryptionInstructions
): Message(message, postDecryptionInstructions.toJson())
{}


Message::Message(
  const unsigned char* message,
  const size_t messageLength,
  const std::string &postDecryptionInstructionsJson
): Message(SodiumBuffer(messageLength, message), postDecryptionInstructionsJson)
{}

Message::Message(
  const unsigned char* message,
  const size_t messageLength,
  const PostDecryptionInstructions &postDecryptionInstructions
): Message(SodiumBuffer(messageLength, message), postDecryptionInstructions.toJson())
{}

const std::string Message::getPostDecryptionInstructionsJson() const {
  const size_t jsonLength = safeStrLength(data, length);
  return std::string( (const char*)data, jsonLength );
}

const bool Message::hasPostDecryptionInstructions() const {
  return length >= 1 && data[0] != '\0';
}

const PostDecryptionInstructions Message::getPostDecryptionInstructions() const {
  return PostDecryptionInstructions(getPostDecryptionInstructionsJson());
}

const std::vector<unsigned char> Message::seal(
  const GlobalPublicKey &publicKey
) const {
  return PublicKey::seal(
    data,
    length,
    publicKey.getPublicKeyBytes()
  );
};

const std::vector<unsigned char> Message::seal(
  const std::vector<unsigned char> &publicKeyBytes
) const {
  return PublicKey::seal(
    data,
    length,
    publicKeyBytes
  );
}

const SodiumBuffer Message::getPlaintext() const {
  const size_t prefixLength = safeStrLength(data, length)  + 1;
  const  size_t plaintextLength = prefixLength > length ? 0 : length - prefixLength;
  return SodiumBuffer(plaintextLength, data + length - plaintextLength);
};
