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
    KeyDerivationOptionsJson::Purpose::ForApplicationUse
  ) {}

  const SodiumBuffer reveal(
  ) const;

};
