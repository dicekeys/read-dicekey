#pragma once

#include "public-key.hpp"

class PublicPrivateKeyPair {
protected:
  const SodiumBuffer secretKey;
  const std::vector<unsigned char> publicKeyBytes;

  PublicPrivateKeyPair(
    const SodiumBuffer& secretKey,
    const std::vector<unsigned char> publicKeyBytes
  );

  PublicPrivateKeyPair(
    const PublicPrivateKeyPair& other
  );

  PublicPrivateKeyPair(
    const SodiumBuffer& seed
  );

public:
  const SodiumBuffer unsealCiphertext(
    const unsigned char* ciphertext,
    const size_t ciphertextLength
  ) const;

private:
  static const PublicPrivateKeyPair createFromSeed(
    const SodiumBuffer& seed
  );

};
