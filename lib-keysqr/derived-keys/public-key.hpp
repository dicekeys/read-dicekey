#pragma once

#include <cassert>
#include <sodium.h>

#include "../keysqr.hpp"
#include "derived-key.hpp"
#include "message.hpp"

class PublicKey {
protected:
  const std::vector<unsigned char> publicKeyBytes;
  const std::string keyDerivationOptionsJson;
  
public:
  PublicKey(
    const std::vector<unsigned char> &publicKeyBytes,
    const std::string &keyDerivationOptionsJson
  );

  PublicKey(const std::string &publicKeyAsJson);

  const std::string toJson(
    int indent = -1,
    const char indent_char = ' '
  ) const;
  
  static const std::vector<unsigned char> seal(
    const SodiumBuffer &message,
    const std::vector<unsigned char> &publicKey,
    const std::string &postDecryptionInstructionsJson = ""
  );

  static const std::vector<unsigned char> seal(
    const unsigned char* message,
    const size_t messageLength,
    const std::vector<unsigned char> &publicKey,
    const std::string &postDecryptionInstructionsJson = ""
  );

  const std::vector<unsigned char> seal(
    const unsigned char* message,
    const size_t messageLength,
    const std::string &postDecryptionInstructionsJson = ""
  ) const;

  const std::vector<unsigned char> seal(
    const SodiumBuffer &message,
    const std::string &postDecryptionInstructionsJson = ""
  ) const;

  const std::vector<unsigned char> seal(
    const Message &message
  ) const;

  const std::vector<unsigned char> getPublicKeyBytes() const;

  const std::string getPublicKeyBytesAsHexDigits() const;

  const std::string getKeyDerivationOptionsJson() const {
    return keyDerivationOptionsJson; 
  }

protected:
  static PublicKey create(const std::string &publicKeyAsJson);

};

