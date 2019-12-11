#include "scoped-public-private-key-pair.hpp"

ScopedPublicPrivateKeyPair::ScopedPublicPrivateKeyPair(
  const KeySqr<Face> &keySqr,
  const std::string &keyDerivationOptionsJson,
  const std::string &clientsApplicationId
) : DerivedPublicPrivateKeyPair(
  keySqr,
  keyDerivationOptionsJson,
  clientsApplicationId, 
  KeyDerivationOptionsJson::Purpose::ForPublicKeySealedMesssages
) {}

const SodiumBuffer ScopedPublicPrivateKeyPair::unseal(
  const unsigned char* ciphertext,
  const size_t ciphertextLength
) const {
  return unsealCiphertext(ciphertext, ciphertextLength);
}
