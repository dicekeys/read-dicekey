#pragma once

#include "public-key.hpp"

class ScopedPublicKey : public PublicKey {
public:
  ScopedPublicKey(const std::vector<unsigned char> &publicKeyBytes): PublicKey(publicKeyBytes)
  {}

  const std::vector<unsigned char> seal(
    const unsigned char* message,
    const size_t messageLength
  ) const;

  const std::vector<unsigned char> seal(
    const SodiumBuffer& message
  ) const;

};

class ScopedPublicPrivateKeyPair : public PublicPrivateKeyPair {

  public:
  ScopedPublicPrivateKeyPair(
    const KeySqr<Face> &keySqr,
    const std::string &keyDerivationOptionsJson,
    const std::string &clientsApplicationId = ""
  );

  const ScopedPublicKey getPublicKey() const {
    return ScopedPublicKey(publicKeyBytes);
  }

  const SodiumBuffer unseal(
    const unsigned char* ciphertext,
    const size_t ciphertextLength
  ) const;

};