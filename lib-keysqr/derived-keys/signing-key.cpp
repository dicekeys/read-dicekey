#include "signing-key.hpp"
#include "key-derivation-options.hpp"
#include "derived-key.hpp"

SigningKey::SigningKey(
  const SodiumBuffer& _signingKey,
  const std::string& _keyDerivationOptionsJson
) :
  keyDerivationOptionsJson(_keyDerivationOptionsJson),
  signingKey(_signingKey)
  {}

SigningKey::SigningKey(
  const SigningKey& other
) :
  keyDerivationOptionsJson(other.keyDerivationOptionsJson),
  signingKey(other.signingKey)
  {}

SigningKey::SigningKey(
  const std::string& seed,
  const std::string& keyDerivationOptionsJson,
  const std::string clientsApplicationId
) : SigningKey(SigningKey::create(seed, keyDerivationOptionsJson, clientsApplicationId))
  {}

const SignatureVerificationKey SigningKey::getSignatureVerificationKey() const {
  std::vector<unsigned char> pk(crypto_sign_PUBLICKEYBYTES);
  crypto_sign_ed25519_sk_to_pk(pk.data(), signingKey.data);
  return SignatureVerificationKey(pk, keyDerivationOptionsJson);
}

SigningKey SigningKey::create(
  const std::string& seed,
  const std::string &keyDerivationOptionsJson,
  const std::string &clientsApplicationId
) {
  const SodiumBuffer derivedKey = KeySqrDerivedKey::validateAndGenerateKey(
    seed,
    keyDerivationOptionsJson,
    KeyDerivationOptionsJson::KeyType::Signing,
    clientsApplicationId,
    crypto_sign_SEEDBYTES
  );
  SodiumBuffer signingKey(crypto_sign_SECRETKEYBYTES);
  std::vector<unsigned char> signatureVerificationKeyBytes(crypto_sign_PUBLICKEYBYTES);
  crypto_sign_seed_keypair(signatureVerificationKeyBytes.data(), signingKey.data, derivedKey.data);
  return SigningKey(signingKey, keyDerivationOptionsJson);
}

const std::vector<unsigned char> SigningKey::generateSignature(
  const unsigned char* message,
  const size_t messageLength
) const {
  std::vector<unsigned char> signature(crypto_sign_BYTES);
  unsigned long long siglen_p;
  crypto_sign_detached(signature.data(), &siglen_p, message, messageLength, signingKey.data);
  return signature;
}

const std::vector<unsigned char> SigningKey::generateSignature(
  const std::vector<unsigned char>& message
) const {
  return generateSignature(message.data(), message.size());
}
