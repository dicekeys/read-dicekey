#pragma once

#include "public-key.hpp"

class ScopedPublicKey : public PublicKey {
public:
  ScopedPublicKey(
    const std::vector<unsigned char> &publicKeyBytes,
    const std::string &keyDerivationOptionsJson
  ): PublicKey(publicKeyBytes, keyDerivationOptionsJson)
  {}

  ScopedPublicKey(std::string publicKeyAsJson) : PublicKey(publicKeyAsJson) {}


  const std::vector<unsigned char> seal(
    const unsigned char* message,
    const size_t messageLength
  ) const;

  const std::vector<unsigned char> seal(
    const SodiumBuffer& message
  ) const;

};
