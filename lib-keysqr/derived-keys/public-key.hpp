#pragma once

#include <cassert>
#include <sodium.h>

#include "../keysqr.hpp"
#include "derived-key.hpp"



class PublicKey {
protected:
  const std::vector<unsigned char> publicKeyBytes;
  const std::string keyDerivationOptionsJson;

  PublicKey(
    const std::vector<unsigned char> publicKeyBytes,
    const std::string keyDerivationOptionsJson
  );

  PublicKey(std::string publicKeyAsJson);
  
public:

  const std::string toJson(
    int indent = -1,
    const char indent_char = ' '
  ) const;
  
  static const std::vector<unsigned char> seal(
    const SodiumBuffer &message,
    const std::vector<unsigned char> &publicKey
  );

  static const std::vector<unsigned char> seal(
    const unsigned char* message,
    const size_t messageLength,
    const std::vector<unsigned char> &publicKey
  );

  const std::vector<unsigned char> getPublicKeyBytes(
  ) const;

  const std::string getPublicKeyBytesAsHexDigits(
  ) const;

};

