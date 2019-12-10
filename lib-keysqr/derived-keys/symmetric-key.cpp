#include "symmetric-key.hpp"

SymmetricKey::SymmetricKey(
  const KeySqr<Face> &keySqr,
  const std::string &keyDerivationOptionsJson,
  const std::string &clientsApplicationId
) : KeySqrDerivedKey(
  keySqr,
  keyDerivationOptionsJson,
  clientsApplicationId, 
  KeyDerivationOptionsJson::Purpose::ForSymmetricKeySealedMessages, 
  crypto_secretbox_KEYBYTES
) {}

const std::vector<unsigned char> SymmetricKey::seal(
  const unsigned char* message,
  const size_t messageLength
) const {
  if (messageLength <= 0) {
    throw "Invalid message length";
  }
  const size_t compositeCiphertextLength =
    crypto_secretbox_NONCEBYTES + messageLength + crypto_secretbox_MACBYTES;
  std::vector<unsigned char> compositeCiphertext(compositeCiphertextLength);
  unsigned char* noncePtr = compositeCiphertext.data();
  unsigned char* secretBoxStartPtr = noncePtr + crypto_secretbox_NONCEBYTES;

  randombytes_buf(noncePtr, crypto_secretbox_NONCEBYTES);
  
  crypto_secretbox_easy(
    secretBoxStartPtr,
    message,
    messageLength,
    noncePtr,
    derivedKey.data
  );
}

const SodiumBuffer SymmetricKey::unseal(
  const unsigned char* compositeCiphertext,
  const size_t compositeCiphertextLength
) const {
  if (compositeCiphertextLength <= (crypto_secretbox_MACBYTES + crypto_secretbox_NONCEBYTES)) {
    throw "Invalid message length";
  }
  SodiumBuffer plaintextBuffer(compositeCiphertextLength - (crypto_secretbox_MACBYTES + crypto_secretbox_NONCEBYTES));
  const unsigned char* noncePtr = compositeCiphertext;
  const unsigned char* secretBoxStartPtr = noncePtr + crypto_secretbox_NONCEBYTES;

  crypto_secretbox_open_easy(
    plaintextBuffer.data,
    secretBoxStartPtr,
    compositeCiphertextLength,
    noncePtr,
    derivedKey.data
  );

  return plaintextBuffer;
};
