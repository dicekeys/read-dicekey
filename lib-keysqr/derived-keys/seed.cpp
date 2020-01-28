#include "./application-key.hpp"

const SodiumBuffer Seed::reveal(
) const {
  return derivedKey;
};
