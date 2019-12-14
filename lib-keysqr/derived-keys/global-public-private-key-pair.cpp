#include "global-public-private-key-pair.hpp"

GlobalPublicPrivateKeyPair::GlobalPublicPrivateKeyPair(
  const KeySqr<Face> &keySqr,
  const std::string &keyDerivationOptionsJson,
  const std::string &clientsApplicationId
) : DerivedPublicPrivateKeyPair (
  keySqr,
  keyDerivationOptionsJson,
  clientsApplicationId, 
  KeyDerivationOptionsJson::Purpose::ForPublicKeySealedMessagesWithRestrictionsEnforcedPostDecryption
) {}

const Message GlobalPublicPrivateKeyPair::unseal(
  const unsigned char* ciphertext,
  const size_t ciphertextLength
) const {
  return Message::reconstituteFromEncodedBuffer(PublicPrivateKeyPair::unsealCiphertext(ciphertext, ciphertextLength));
}

const Message GlobalPublicPrivateKeyPair::unseal(
  const std::vector<unsigned char> &ciphertext
) const {
  return Message::reconstituteFromEncodedBuffer(PublicPrivateKeyPair::unsealCiphertext(ciphertext.data(), ciphertext.size()));
}

const GlobalPublicKey GlobalPublicPrivateKeyPair::getPublicKey() const {
  return GlobalPublicKey(publicKeyBytes, keyDerivationOptionsJson);
}
