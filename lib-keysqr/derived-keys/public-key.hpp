#pragma once

#include <cassert>
#include <sodium.h>

#include "../keysqr.h"
#include "derived-key.hpp"



class PublicKey {
protected:
  const std::vector<unsigned char> publicKeyBytes;

  PublicKey(const std::vector<unsigned char> publicKeyBytes) : publicKeyBytes(publicKeyBytes) {
    if (publicKeyBytes.size() != crypto_box_PUBLICKEYBYTES) {
      throw "Invalid key size exception";
    }
  }
  
public:
  
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

};


class PublicPrivateKeyPair {
  protected:
    class PublicKeySeed: KeySqrDerivedKey {
      public:
      PublicKeySeed(
        const KeySqr<Face> &keySqr,
        const std::string &keyDerivationOptionsJson,
        const std::string &clientsApplicationId,
        const KeyDerivationOptionsJson::Purpose mandatedPurpose
      ) : KeySqrDerivedKey(keySqr, keyDerivationOptionsJson, clientsApplicationId, mandatedPurpose)
      {}

      const SodiumBuffer getSeed() const { return derivedKey; }
    };

  protected:
  const SodiumBuffer sk;
  const std::vector<unsigned char> publicKeyBytes;

  PublicPrivateKeyPair(
    const SodiumBuffer &sk,
    const std::vector<unsigned char> pk
  );

  PublicPrivateKeyPair(
    const PublicPrivateKeyPair & other
  );

  PublicPrivateKeyPair(
    const KeySqr<Face> &keySqr,
    const std::string &keyDerivationOptionsJson,
    const std::string &clientsApplicationId,
    const KeyDerivationOptionsJson::Purpose mandatedPurpose
  );

  static PublicPrivateKeyPair create(
    const KeySqr<Face> &keySqr,
    const std::string &keyDerivationOptionsJson,
    const std::string &clientsApplicationId,
    const KeyDerivationOptionsJson::Purpose mandatedPurpose
  );
  
  protected:

  const SodiumBuffer unsealCiphertext(
    const unsigned char* ciphertext,
    const size_t ciphertextLength
  ) const;


};
