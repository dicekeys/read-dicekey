#pragma once

#include <cassert>
#include <sodium.h>

#include "sodium-buffer.hpp"
#include "../keysqr.hpp"

class SignatureVerificationKey {
public:
  const std::vector<unsigned char> verificationKeyBytes;
  const std::string keyDerivationOptionsJson;
 
  SignatureVerificationKey(
    const std::vector<unsigned char> &publicKeyBytes,
    const std::string &keyDerivationOptionsJson
  );

  //SignatureVerificationKey(const std::string &publicKeyAsJson);

  const std::string toJson(
    int indent = -1,
    const char indent_char = ' '
  ) const;


  static bool verify(
    const std::vector<unsigned char>& signatureVerificationKey,
    const unsigned char* message,
    const size_t messageLength,
    const std::vector<unsigned char>& signature
  );

  bool verify(
    const unsigned char* message,
    const size_t messageLength,
    const std::vector<unsigned char>& signature
  ) const;

  bool verify(
    const std::vector<unsigned char>& message,
    const std::vector<unsigned char>& signature
  ) const;

  bool verify(
    const SodiumBuffer& message,
    const std::vector<unsigned char>& signature
  ) const;


  const std::vector<unsigned char> getKeyBytes() const;

  const std::string getKeyBytesAsHexDigits() const;

  const std::string getKeyDerivationOptionsJson() const {
    return keyDerivationOptionsJson; 
  }

};

