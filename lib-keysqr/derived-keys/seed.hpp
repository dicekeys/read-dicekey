#pragma once

#include "derived-key.hpp"

class Seed : KeySqrDerivedKey {
  public:
  Seed(
    const KeySqr<Face> &keySqr,
    const std::string &keyDerivationOptionsJson,
    const std::string &clientsApplicationId = ""
  ) : KeySqrDerivedKey(
    keySqr,
    keyDerivationOptionsJson,
    KeyDerivationOptionsJson::KeyType::Seed,
    clientsApplicationId
  ) {}

  const SodiumBuffer reveal(
  ) const;

};
