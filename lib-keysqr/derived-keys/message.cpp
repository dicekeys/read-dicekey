#define __STDC_WANT_LIB_EXT1__ 1
#include "message.hpp"
#include <string.h>
#include <exception>


const std::string jsonStringEmbedded = "\"embedded\"";

size_t safeStrLength(const unsigned char *str, size_t maxLength) {
  size_t i = 0;
  while (i < maxLength && str[i] != '\0') i++;
  return i;
}

Message::Message(
  const Message &other
) :
  contents(other.contents),
  postDecryptionInstructionsJson(other.postDecryptionInstructionsJson)
  {}

Message::Message(
  const SodiumBuffer &contents,
  const std::string &postDecryptionInstructionsJson
) :
  contents(contents),
  postDecryptionInstructionsJson(postDecryptionInstructionsJson)
  {}

Message::Message(
  const unsigned char* message,
  const size_t messagelength,
  const std::string &postDecryptionInstructionsJson
) :
  contents(messagelength, message),
  postDecryptionInstructionsJson(postDecryptionInstructionsJson)
  {}

const Message Message::embedPostDecryptionInstructions(
) {
  SodiumBuffer newContents(
    contents.length + 1 + postDecryptionInstructionsJson.length()
  );
  // Copy the JSON instrucions at the start of the embedded content
  memcpy(
    newContents.data,
    postDecryptionInstructionsJson.c_str(),
    postDecryptionInstructionsJson.length()
  );
  // null terminate the instructions
  newContents.data[postDecryptionInstructionsJson.length()] = '\0';
  // Concatenated the old contents to the ned of hte new buffer
  memcpy(
    newContents.data + postDecryptionInstructionsJson.length() + 1,
    contents.data,
    contents.length
  );
  return Message(newContents, jsonStringEmbedded);
}

const Message Message::unembedPostDecryptionInstructions() {
  // If this message doesn't have embedded instructions,
  // there's no need to unembded them.  Just return itself.
  if (postDecryptionInstructionsJson != jsonStringEmbedded) {
    return *this;
  }

  const size_t nullCharIndex = safeStrLength(contents.data, contents.length);
  if (nullCharIndex == contents.length) {
    throw std::exception("Null string terminator missing from embedded instructions json");
  }
  const std::string extractedPostDecryptionInstructionsJson(
    (const char*) contents.data,
    nullCharIndex
  );  
  const SodiumBuffer extractedContents(
    contents.length - (nullCharIndex + 1),
    contents.data + (nullCharIndex + 1)
  );
  return Message(extractedContents, extractedPostDecryptionInstructionsJson);
}

const Message Message::createAndRemoveAnyEmbedding(
  const SodiumBuffer &contents,
  const std::string &postDecryptionInstructionsJson
) {
  if (Message::arePostDecryptionInstructionsEmbedded(postDecryptionInstructionsJson)) {
    return Message(contents, postDecryptionInstructionsJson).unembedPostDecryptionInstructions();
  } else {
    return Message(contents, postDecryptionInstructionsJson);
  }
}

bool Message::arePostDecryptionInstructionsEmbedded() const {
  return Message::arePostDecryptionInstructionsEmbedded(postDecryptionInstructionsJson);
}

bool Message::arePostDecryptionInstructionsEmbedded(
  const std::string &postDecryptionInstructionsJson     
) {
  return postDecryptionInstructionsJson == jsonStringEmbedded;
}

const std::string Message::getPostDecryptionInstructionsJson() const {
  return postDecryptionInstructionsJson;
}

const PostDecryptionInstructions Message::getPostDecryptionInstructions() const {
  if (arePostDecryptionInstructionsEmbedded()) {
    throw std::exception("Post-decryption instructions must be extracted first.");
  }
  return PostDecryptionInstructions(getPostDecryptionInstructionsJson());
}

const bool Message::hasPostDecryptionInstructions() const {
  return (postDecryptionInstructionsJson.length() > 0);
}

const SodiumBuffer Message::getPlaintext() const {
  return contents;
}


const std::vector<unsigned char> Message::seal(
  const PublicKey &publicKey
) const {
  return PublicKey::seal(
    contents,
    publicKey.getPublicKeyBytes(),
    postDecryptionInstructionsJson
  );
};

const std::vector<unsigned char> Message::seal(
  const std::vector<unsigned char> &publicKeyBytes
) const {
  return PublicKey::seal(
    contents,
    publicKeyBytes,
    postDecryptionInstructionsJson
  );
}
