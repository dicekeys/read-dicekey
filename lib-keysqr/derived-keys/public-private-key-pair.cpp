#include "public-private-key-pair.hpp"
#include "crypto_box_seal_salted.h"
#include "key-derivation-options.hpp"

PublicPrivateKeyPair:: PublicPrivateKeyPair(
  const SodiumBuffer &secretKey,
  const std::vector<unsigned char> &publicKeyBytes,
  const std::string &KeyDerivationOptionsJson
) :
  PublicKey(publicKeyBytes, KeyDerivationOptionsJson),
  secretKey(secretKey)
  {}

PublicPrivateKeyPair::PublicPrivateKeyPair(
  const PublicPrivateKeyPair &other
):
  PublicKey(publicKeyBytes, keyDerivationOptionsJson),
  secretKey(other.secretKey)
  {}

PublicPrivateKeyPair::PublicPrivateKeyPair(
  const KeySqr<Face> &keySqr,
  const std::string &keyDerivationOptionsJson,
  const std::string &clientsApplicationId
) : PublicPrivateKeyPair(
  PublicPrivateKeyPair::create(keySqr, keyDerivationOptionsJson, clientsApplicationId)
) {}


PublicPrivateKeyPair PublicPrivateKeyPair::create(
  const KeySqr<Face> &keySqr,
  const std::string &keyDerivationOptionsJson,
  const std::string &clientsApplicationId
) {
  const SodiumBuffer derivedKey = KeySqrDerivedKey::validateAndGenerateKey(
    keySqr, keyDerivationOptionsJson, clientsApplicationId,
    KeyDerivationOptionsJson::Purpose::ForPublicKeySealedMessages,
    crypto_box_SEEDBYTES
  );
  SodiumBuffer secretKey(crypto_box_SECRETKEYBYTES);
  std::vector<unsigned char> publicKey(crypto_box_PUBLICKEYBYTES);
  crypto_box_seed_keypair(publicKey.data(), secretKey.data, derivedKey.data);
  return PublicPrivateKeyPair(secretKey, publicKey, keyDerivationOptionsJson);
}


const Message PublicPrivateKeyPair::unseal(
  const unsigned char* ciphertext,
  const size_t ciphertextLength,
  const std::string &postDecryptionInstructionsJson
) const {
  if (ciphertextLength <= crypto_box_SEALBYTES) {
    throw std::exception("Invalid message length");
  }
  SodiumBuffer plaintext(ciphertextLength -crypto_box_SEALBYTES);

  const int result = crypto_box_salted_seal_open(
    plaintext.data,
    ciphertext,
    ciphertextLength,
    publicKeyBytes.data(),
    secretKey.data,
    postDecryptionInstructionsJson.c_str(),
    postDecryptionInstructionsJson.length()
  );
  if (result != 0) {
    throw std::exception("crypto_box_seal_open failed.  message forged or corrupted.");
  }
  return Message::createAndRemoveAnyEmbedding(plaintext, postDecryptionInstructionsJson);
}

const Message PublicPrivateKeyPair::unseal(
  const std::vector<unsigned char> ciphertext,
  const std::string& postDecryptionInstructionsJson
) const {
  return unseal(ciphertext.data(), ciphertext.size(), postDecryptionInstructionsJson
  );
};

const PublicKey PublicPrivateKeyPair::getPublicKey() const {
  return PublicKey(publicKeyBytes, keyDerivationOptionsJson);
}