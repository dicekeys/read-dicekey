#include "public-private-key-pair.hpp"

PublicPrivateKeyPair::PublicPrivateKeyPair(
  const SodiumBuffer &secretKey,
  const std::vector<unsigned char> publicKeyBytes
): secretKey(secretKey), publicKeyBytes(publicKeyBytes)
{}

PublicPrivateKeyPair::PublicPrivateKeyPair(
  const PublicPrivateKeyPair &other
): secretKey(other.secretKey), publicKeyBytes(other.publicKeyBytes)
{}

PublicPrivateKeyPair::PublicPrivateKeyPair(
  const SodiumBuffer &seed
): PublicPrivateKeyPair(createFromSeed(seed))
{}

const PublicPrivateKeyPair PublicPrivateKeyPair::createFromSeed (
  const SodiumBuffer &seed
) {
  SodiumBuffer secretKey(crypto_box_SECRETKEYBYTES);
  std::vector<unsigned char> publicKeyBytes(crypto_box_PUBLICKEYBYTES);
  crypto_box_seed_keypair(publicKeyBytes.data(), secretKey.data, seed.data);
  return PublicPrivateKeyPair(secretKey, publicKeyBytes);
}


const SodiumBuffer PublicPrivateKeyPair::unsealCiphertext(
  const unsigned char* ciphertext,
  const size_t ciphertextLength
) const {
  if (ciphertextLength <= crypto_box_SEALBYTES) {
    throw "Invalid message length";
  }
  SodiumBuffer plaintext(ciphertextLength -crypto_box_SEALBYTES);

  const int result = crypto_box_seal_open(
    plaintext.data,
    ciphertext,
    ciphertextLength,
    publicKeyBytes.data(),
    secretKey.data
  );
  if (result != 0) {
    throw "crypto_box_seal_open failed";
  }
  return plaintext;
}
