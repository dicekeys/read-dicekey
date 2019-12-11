#include "./application-key.hpp"

const SodiumBuffer ApplicationKey::reveal(
) const {
  return derivedKey;
};
