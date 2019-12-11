#pragma once

#include "public-private-key-pair.hpp"

class DerivedPublicPrivateKeyPair: protected KeySqrDerivedKey, protected PublicPrivateKeyPair {
  // protected:
  //   class PublicKeySeed: KeySqrDerivedKey {
  //     public:
  //     PublicKeySeed(
  //       const KeySqr<Face> &keySqr,
  //       const std::string &keyDerivationOptionsJson,
  //       const std::string &clientsApplicationId,
  //       const KeyDerivationOptionsJson::Purpose mandatedPurpose
  //     ) : KeySqrDerivedKey(keySqr, keyDerivationOptionsJson, clientsApplicationId, mandatedPurpose)
  //     {}

  //     const SodiumBuffer getSeed() const { return derivedKey; }
  //   };

protected:

  DerivedPublicPrivateKeyPair(
    const DerivedPublicPrivateKeyPair& other
  );

  DerivedPublicPrivateKeyPair(
    const SodiumBuffer &derivedKeySeed,
    const std::string &keyDerivationOptionsJson,
    const SodiumBuffer &secretKey,
    const std::vector<unsigned char> publicKeyBytes
  );

  DerivedPublicPrivateKeyPair(
    const KeySqr<Face> &keySqr,
    const std::string &keyDerivationOptionsJson,
    const std::string &clientsApplicationId,
    const KeyDerivationOptionsJson::Purpose mandatedPurpose
  );

  private:
    static DerivedPublicPrivateKeyPair create(
      const KeySqr<Face> &keySqr,
      const std::string &keyDerivationOptionsJson,
      const std::string &clientsApplicationId,
      const KeyDerivationOptionsJson::Purpose mandatedPurpose
    );
  


};
