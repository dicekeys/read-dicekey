#pragma once

#include <cassert>
#include <sodium.h>

#include "../keysqr.h"
#include "derived-key.hpp"
#include "public-key.hpp"

class GlobalPublicKey: public PublicKey {
public:
  GlobalPublicKey(const std::vector<unsigned char> &publicKeyBytes): PublicKey(publicKeyBytes)
  {}

};
