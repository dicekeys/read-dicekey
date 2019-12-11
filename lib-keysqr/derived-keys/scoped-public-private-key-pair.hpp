#include "scoped-public-key.hpp"
#include "public-private-key-pair.hpp"

class ScopedPublicPrivateKeyPair : public PublicPrivateKeyPair {

  public:
  ScopedPublicPrivateKeyPair(
    const KeySqr<Face> &keySqr,
    const std::string &keyDerivationOptionsJson,
    const std::string &clientsApplicationId = ""
  );

  const ScopedPublicKey getPublicKey() const {
    return ScopedPublicKey(publicKeyBytes, keyDerivationOptionsJson);
  }

  const SodiumBuffer unseal(
    const unsigned char* ciphertext,
    const size_t ciphertextLength
  ) const;

};