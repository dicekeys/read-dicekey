#include "./seed.hpp"

const SodiumBuffer Seed::reveal(
) const {
  return derivedKey;
};
