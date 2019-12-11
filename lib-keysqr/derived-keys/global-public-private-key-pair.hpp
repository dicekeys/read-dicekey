#pragma once

#include <cassert>
#include <sodium.h>

#include "../keysqr.hpp"
#include "derived-key.hpp"
#include "public-key.hpp"
#include "global-public-key.hpp"
#include "derived-public-private-key-pair.hpp"
#include "message-with-decryption-restrictions.hpp"

class GlobalPublicPrivateKeyPair : public DerivedPublicPrivateKeyPair {
  
  public:
  GlobalPublicPrivateKeyPair(
    const KeySqr<Face> &keySqr,
    const std::string &keyDerivationOptionsJson,
    const std::string &clientsApplicationId = ""
  );

  const MessageWithDecryptionRestrictions unseal(
    const unsigned char* ciphertext,
    const size_t ciphertextLength
  ) const;

  const GlobalPublicKey getPublicKey() const {
    return GlobalPublicKey(publicKeyBytes, keyDerivationOptionsJson);
  }


};

