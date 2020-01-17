#pragma once

#include "derived-key.hpp"

class ApplicationKey : KeySqrDerivedKey {
  public:
  ApplicationKey(
    const KeySqr<Face> &keySqr,
    const std::string &keyDerivationOptionsJson,
    const std::string &clientsApplicationId = ""
  ) : KeySqrDerivedKey(
    keySqr,
    keyDerivationOptionsJson,
    clientsApplicationId, 
    KeyDerivationOptionsJson::KeyType::Seed
  ) {}

  const SodiumBuffer reveal(
  ) const;

};
