#pragma once

#include "public-key.hpp"
#include "message.hpp"

class PublicPrivateKeyPair: PublicKey {
protected:
  const SodiumBuffer secretKey;
//  const std::vector<unsigned char> publicKeyBytes;
//  const std::string keyDerivationOptionsJson;

  PublicPrivateKeyPair(
    const SodiumBuffer &secretKey,
    const std::vector<unsigned char> &publicKeyBytes,
    const std::string &KeyDerivationOptionsJson
  );

  PublicPrivateKeyPair(
    const PublicPrivateKeyPair& other
  );

public:
  PublicPrivateKeyPair(
    const KeySqr<Face> &keySqr,
    const std::string &keyDerivationOptionsJson,
    const std::string &clientsApplicationId = ""
  );

  const PublicKey getPublicKey() const;

  const Message unseal(
    const unsigned char* ciphertext,
    const size_t ciphertextLength,
    const std::string &postDecryptionInstructionsJson = ""
  ) const;

  const Message unseal(
    const std::vector<unsigned char> ciphertext,
    const std::string& postDecryptionInstructionsJson = ""
  ) const;

private:
  static PublicPrivateKeyPair PublicPrivateKeyPair::create(
    const KeySqr<Face> &keySqr,
    const std::string &keyDerivationOptionsJson,
    const std::string &clientsApplicationId
  );

};
