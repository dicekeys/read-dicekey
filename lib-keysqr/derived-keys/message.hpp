#pragma once

#include <cassert>
#include <sodium.h>

#include "sodium-buffer.hpp"
#include "../keysqr.hpp"
#include "derived-key.hpp"
#include "public-key.hpp"
#include "post-decryption-instructions.hpp"
#include "sodium-initializer.hpp"
#include "public-key.hpp"


// Post decryption instructions
//   starts with "{": string is utf8 json string encoding the post-decryption instructions
//   is "\"prepended\"": encryptions are prepended to plaintext as null-terminated utf8 string
//   "": there are no post-decryption instructions

class Message
{
  public:
    const SodiumBuffer contents;
    const std::string postDecryptionInstructionsJson;

    Message::Message(
      const Message &other
    );

    Message(
      const SodiumBuffer &contents,
      const std::string &postDecryptionInstructionsJson
    );

    Message(
      const unsigned char* message,
      const size_t messagelength,
      const std::string &postDecryptionInstructionsJson
    );

    const Message embedPostDecryptionInstructions();

    const Message unembedPostDecryptionInstructions();

    static const Message createAndRemoveAnyEmbedding(
      const SodiumBuffer &contents,
      const std::string &postDecryptionInstructionsJson
    );

    static bool arePostDecryptionInstructionsEmbedded(
      const std::string &postDecryptionInstructionsJson     
    );
    bool arePostDecryptionInstructionsEmbedded() const;

    const std::string getPostDecryptionInstructionsJson() const;

    const PostDecryptionInstructions getPostDecryptionInstructions() const;

    const bool hasPostDecryptionInstructions() const;

    const SodiumBuffer getPlaintext() const;

    const std::vector<unsigned char> seal(
      const PublicKey &publicKey
    ) const;

    const std::vector<unsigned char> seal(
      const std::vector<unsigned char> &publicKeyBytes
    ) const;

};