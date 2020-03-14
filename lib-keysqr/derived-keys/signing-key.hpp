#pragma once

#include "sodium-buffer.hpp"
#include "signature-verification-key.hpp"

class SigningKey {
protected:
  const SodiumBuffer signingKey;
  const std::string keyDerivationOptionsJson;

  SigningKey(
    const SodiumBuffer &signingKey,
    const std::string &KeyDerivationOptionsJson
  );

  SigningKey(
    const SigningKey& other
  );

public:
  SigningKey(
    const KeySqr<Face> &keySqr,
    const std::string &keyDerivationOptionsJson,
    const std::string clientsApplicationId = ""
  );

  const SignatureVerificationKey getSignatureVerificationKey() const;

  const std::vector<unsigned char> generateSignature(
    const unsigned char* message,
    const size_t messageLength
  ) const;

  const std::vector<unsigned char> generateSignature(
    const std::vector<unsigned char> &message
  ) const;


 
private:
  static SigningKey create(
    const KeySqr<Face> &keySqr,
    const std::string &keyDerivationOptionsJson,
    const std::string &clientsApplicationId
  );

};
