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
