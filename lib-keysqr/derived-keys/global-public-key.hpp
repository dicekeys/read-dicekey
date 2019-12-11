#pragma once

#include <cassert>
#include <sodium.h>

#include "derived-key.hpp"
#include "public-key.hpp"

class GlobalPublicKey: public PublicKey {
public:
  GlobalPublicKey(
    const std::vector<unsigned char> &publicKeyBytes,
    const std::string &keyDerivationOptionsJson
  ): PublicKey(publicKeyBytes, keyDerivationOptionsJson)
  {}

};
