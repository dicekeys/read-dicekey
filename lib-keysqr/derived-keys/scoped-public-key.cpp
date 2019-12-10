#include "scoped-public-key.hpp"


const std::vector<unsigned char> ScopedPublicKey::seal(
  const unsigned char* message,
  const size_t messageLength
) const {
  return PublicKey::seal(message, messageLength, publicKeyBytes);
}

const std::vector<unsigned char> ScopedPublicKey::seal(
  const SodiumBuffer& message
) const {
  return PublicKey::seal(message.data, message.length, publicKeyBytes);
}

ScopedPublicPrivateKeyPair::ScopedPublicPrivateKeyPair(
  const KeySqr<Face> &keySqr,
  const std::string &keyDerivationOptionsJson,
  const std::string &clientsApplicationId
) : PublicPrivateKeyPair(
  keySqr,
  keyDerivationOptionsJson,
  clientsApplicationId, 
  KeyDerivationOptionsJson::Purpose::ForPublicKeySealedMesssages
) {}

const SodiumBuffer ScopedPublicPrivateKeyPair::unseal(
  const unsigned char* ciphertext,
  const size_t ciphertextLength
) const {
  return PublicPrivateKeyPair::unsealCiphertext(ciphertext, ciphertextLength);
}
