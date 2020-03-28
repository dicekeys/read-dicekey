#pragma once

#include "derived-key.hpp"

class Seed : KeySqrDerivedKey {
  public:
  Seed(
    const std::string& seed,
    const std::string &keyDerivationOptionsJson,
    const std::string &clientsApplicationId = ""
  ) : KeySqrDerivedKey(
    seed,
    keyDerivationOptionsJson,
    KeyDerivationOptionsJson::KeyType::Seed,
    clientsApplicationId
  ) {}

  const SodiumBuffer reveal(
  ) const;

};
