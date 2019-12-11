#include "public-key.hpp"

class PublicPrivateKeyPair: protected KeySqrDerivedKey {
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
//  const PublicKeySeed seed;
  const SodiumBuffer secretKey;
  const std::vector<unsigned char> publicKeyBytes;

  PublicPrivateKeyPair(
    const SodiumBuffer &derivedKeySeed,
    const std::string &keyDerivationOptionsJson,
    const SodiumBuffer &secretKey,
    const std::vector<unsigned char> publicKeyBytes
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
