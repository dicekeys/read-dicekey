#pragma once

#include <cassert>
#include <sodium.h>

#include "../keysqr.h"
#include "derived-key.hpp"
#include "public-key.hpp"
#include "../decryption-restrictions.hpp"
#include "../sodium-initializer.hpp"
#include "global-public-key.hpp"


class MessageWithDecryptionRestrictions: SodiumBuffer
 {
  public:

  MessageWithDecryptionRestrictions(
    const SodiumBuffer &buffer
  );

  MessageWithDecryptionRestrictions(
    const SodiumBuffer &message,
    const DecryptionRestrictions &decryptionRestrictions
  );

  MessageWithDecryptionRestrictions(
    const unsigned char* message,
    const size_t messagelength,
    const DecryptionRestrictions &decryptionRestrictions
  );

  MessageWithDecryptionRestrictions(
    const SodiumBuffer &message,
    const std::string &decryptionRestrictionsJson
  );

  MessageWithDecryptionRestrictions(
    const unsigned char* message,
    const size_t messagelength,
    const std::string &decryptionRestrictionsJson
  );

  const std::string getDecryptionRestrictionsJson() const;

  const DecryptionRestrictions getDecryptionRestrictions() const;

  const SodiumBuffer getPlaintext() const;

  const std::vector<unsigned char> seal(
    const GlobalPublicKey &publicKey
  ) const;

  const std::vector<unsigned char> seal(
    const std::vector<unsigned char> &publicKeyBytes
  ) const;

};