#include "public-private-key-pair.hpp"

PublicPrivateKeyPair::PublicPrivateKeyPair(
  const SodiumBuffer &derivedKeySeed,
  const std::string &keyDerivationOptionsJson,
  const SodiumBuffer &secretKey,
  const std::vector<unsigned char> publicKeyBytes
) : KeySqrDerivedKey(derivedKeySeed, keyDerivationOptionsJson), publicKeyBytes(publicKeyBytes), secretKey(secretKey) {}

PublicPrivateKeyPair::PublicPrivateKeyPair(
  const PublicPrivateKeyPair & other
) :
  KeySqrDerivedKey(other.derivedKey, other.keyDerivationOptionsJson),
  publicKeyBytes(other.publicKeyBytes),
  secretKey(other.secretKey) {}

PublicPrivateKeyPair::PublicPrivateKeyPair(
  const KeySqr<Face> &keySqr,
  const std::string &keyDerivationOptionsJson,
  const std::string &clientsApplicationId,
  const KeyDerivationOptionsJson::Purpose mandatedPurpose
) : KeySqrDerivedKey( 
  KeySqrDerivedKey::validateAndGenerateKey(
    keySqr, keyDerivationOptionsJson, clientsApplicationId, mandatedPurpose, crypto_box_SEEDBYTES ),
    keyDerivationOptionsJson
) {
  SodiumBuffer secretKey(crypto_box_SECRETKEYBYTES);
  std::vector<unsigned char> publicKey(crypto_box_PUBLICKEYBYTES);
  crypto_box_seed_keypair(publicKey.data(), secretKey.data, derivedKey.data);
  PublicPrivateKeyPair(derivedKeySeed, keyDerivationOptionsJson, secretKey, publicKey);
}


// PublicPrivateKeyPair PublicPrivateKeyPair::create(
//   SodiumBuffer
//   const KeySqr<Face> &keySqr,
//   const std::string &keyDerivationOptionsJson,
//   const std::string &clientsApplicationId,
//   const KeyDerivationOptionsJson::Purpose mandatedPurpose
// ) {
//   SodiumBuffer secretKey(crypto_box_SECRETKEYBYTES);
//   std::vector<unsigned char> publicKey(crypto_box_PUBLICKEYBYTES);
//   crypto_box_seed_keypair(publicKey.data(), secretKey.data, derivedKey.data);
//   return PublicPrivateKeyPair(secretKey, publicKey);
// }


const SodiumBuffer PublicPrivateKeyPair::unsealCiphertext(
  const unsigned char* ciphertext,
  const size_t ciphertextLength
) const {
  if (ciphertextLength <= crypto_box_SEALBYTES) {
    throw "Invalid message length";
  }
  SodiumBuffer plaintext(ciphertextLength -crypto_box_SEALBYTES);

  crypto_box_seal_open(
    plaintext.data,
    ciphertext,
    ciphertextLength,
    publicKeyBytes.data(),
    secretKey.data
  );
  return plaintext;
}
