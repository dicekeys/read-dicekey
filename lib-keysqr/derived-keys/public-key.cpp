#include "public-key.hpp"



const std::vector<unsigned char> PublicKey::seal(
  const unsigned char* message,
  const size_t messageLength,
  const std::vector<unsigned char> &publicKey
) {
  if (publicKey.size() != crypto_box_PUBLICKEYBYTES) {
    throw "Invalid key size exception";
  }
  if (messageLength <= 0) {
    throw "Invalid message length";
  }
  const size_t ciphertextLength =
    messageLength + crypto_box_SEALBYTES;
  std::vector<unsigned char> ciphertext(ciphertextLength);

  crypto_box_seal(
    ciphertext.data(),
    message,
    messageLength,
    publicKey.data()
  );

  return ciphertext;
}

const std::vector<unsigned char> seal(
  const SodiumBuffer &message,
  const std::vector<unsigned char> &publicKey
) {
  return PublicKey::seal(
    message.data, message.length, publicKey
  );
}

const std::vector<unsigned char> PublicKey::getPublicKeyBytes(
) const {
  return publicKeyBytes;
}

PublicPrivateKeyPair::PublicPrivateKeyPair(
  const SodiumBuffer &sk,
  const std::vector<unsigned char> pk
) : publicKeyBytes(pk), sk(sk) {}

PublicPrivateKeyPair::PublicPrivateKeyPair(
  const PublicPrivateKeyPair & other
) : publicKeyBytes(other.publicKeyBytes), sk(other.sk) {}

PublicPrivateKeyPair::PublicPrivateKeyPair(
  const KeySqr<Face> &keySqr,
  const std::string &keyDerivationOptionsJson,
  const std::string &clientsApplicationId,
  const KeyDerivationOptionsJson::Purpose mandatedPurpose
): PublicPrivateKeyPair(PublicPrivateKeyPair::create(
  keySqr, keyDerivationOptionsJson, clientsApplicationId, mandatedPurpose
)) {}


PublicPrivateKeyPair PublicPrivateKeyPair::create(
  const KeySqr<Face> &keySqr,
  const std::string &keyDerivationOptionsJson,
  const std::string &clientsApplicationId,
  const KeyDerivationOptionsJson::Purpose mandatedPurpose
) {
  const PublicKeySeed seed(keySqr, keyDerivationOptionsJson, clientsApplicationId, mandatedPurpose);
  SodiumBuffer secretKey(crypto_box_SECRETKEYBYTES);
  std::vector<unsigned char> publicKey(crypto_box_PUBLICKEYBYTES);
  crypto_box_seed_keypair(publicKey.data(), secretKey.data, seed.getSeed().data);
  return PublicPrivateKeyPair(secretKey, publicKey);
}


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
    sk.data
  );
  return plaintext;
}
