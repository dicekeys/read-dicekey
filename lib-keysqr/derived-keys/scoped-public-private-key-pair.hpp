#include "scoped-public-key.hpp"
#include "derived-public-private-key-pair.hpp"

class ScopedPublicPrivateKeyPair : public DerivedPublicPrivateKeyPair {

  public:
  ScopedPublicPrivateKeyPair(
    const KeySqr<Face> &keySqr,
    const std::string &keyDerivationOptionsJson,
    const std::string &clientsApplicationId = ""
  );

  const ScopedPublicKey getPublicKey() const;

  const SodiumBuffer unseal(
    const unsigned char* ciphertext,
    const size_t ciphertextLength
  ) const;

};