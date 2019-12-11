#include "derived-public-private-key-pair.hpp"


DerivedPublicPrivateKeyPair::DerivedPublicPrivateKeyPair(
  const SodiumBuffer &derivedKeySeed,
  const std::string &keyDerivationOptionsJson,
  const SodiumBuffer &secretKey,
  const std::vector<unsigned char> publicKeyBytes
) :
  KeySqrDerivedKey(derivedKeySeed, keyDerivationOptionsJson),
  PublicPrivateKeyPair(secretKey, publicKeyBytes)
  {}

DerivedPublicPrivateKeyPair::DerivedPublicPrivateKeyPair(
  const DerivedPublicPrivateKeyPair & other
) :
  KeySqrDerivedKey(other.derivedKey, other.keyDerivationOptionsJson),
  PublicPrivateKeyPair(other.secretKey, other.publicKeyBytes)
  {}

DerivedPublicPrivateKeyPair::DerivedPublicPrivateKeyPair(
  const KeySqr<Face> &keySqr,
  const std::string &keyDerivationOptionsJson,
  const std::string &clientsApplicationId,
  const KeyDerivationOptionsJson::Purpose mandatedPurpose
) : DerivedPublicPrivateKeyPair( DerivedPublicPrivateKeyPair::create(
    keySqr, keyDerivationOptionsJson, clientsApplicationId, mandatedPurpose
  )) {}

DerivedPublicPrivateKeyPair DerivedPublicPrivateKeyPair::create(
  const KeySqr<Face> &keySqr,
  const std::string &keyDerivationOptionsJson,
  const std::string &clientsApplicationId,
  const KeyDerivationOptionsJson::Purpose mandatedPurpose
) {
  const SodiumBuffer derivedKey = KeySqrDerivedKey::validateAndGenerateKey(
    keySqr, keyDerivationOptionsJson, clientsApplicationId, mandatedPurpose, crypto_box_SEEDBYTES
  );
  SodiumBuffer secretKey(crypto_box_SECRETKEYBYTES);
  std::vector<unsigned char> publicKey(crypto_box_PUBLICKEYBYTES);
  crypto_box_seed_keypair(publicKey.data(), secretKey.data, derivedKey.data);
  return DerivedPublicPrivateKeyPair(derivedKey, keyDerivationOptionsJson, secretKey, publicKey);
}
