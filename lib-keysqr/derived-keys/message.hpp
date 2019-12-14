#pragma once

#include <cassert>
#include <sodium.h>

#include "../keysqr.hpp"s
#include "derived-key.hpp"
#include "public-key.hpp"
#include "post-decryption-instructions.hpp"
#include "sodium-initializer.hpp"
#include "global-public-key.hpp"



// 1 byte: type/version, 24 json length
// '\0', no json instructions plain text
// '{', json instructions/restrictions inline, followed by null terminator
// 1, 32 byte hash of instructions follows

// message with decryption restrictions -> message with instructions
// 1, json instructions/restrictions separate, 32-byte hash follows, followed by plaintext

class Message: SodiumBuffer
{
  protected:
    Message(
      const SodiumBuffer &buffer,
      const bool isAlreadyEncoded
    );

  public:

    Message(
      const Message &other
    );

    static const Message reconstituteFromEncodedBuffer(
      const SodiumBuffer &buffer
    );

    static const Message createFromPlaintextBuffer(
      const SodiumBuffer &plaintext
    );

    Message(
      const SodiumBuffer &message,
      const PostDecryptionInstructions &postDecryptionInstructions
    );

    Message(
      const unsigned char* message,
      const size_t messagelength,
      const PostDecryptionInstructions &postDecryptionInstructions
    );

    Message(
      const SodiumBuffer &message,
      const std::string &postDecryptionInstructionsJson
    );

    Message(
      const unsigned char* message,
      const size_t messagelength,
      const std::string &postDecryptionInstructionsJson
    );

    const std::string getPostDecryptionInstructionsJson() const;

    const bool hasPostDecryptionInstructions() const;

    const PostDecryptionInstructions getPostDecryptionInstructions() const;

    const SodiumBuffer getPlaintext() const;

    const std::vector<unsigned char> seal(
      const GlobalPublicKey &publicKey
    ) const;

    const std::vector<unsigned char> seal(
      const std::vector<unsigned char> &publicKeyBytes
    ) const;

};