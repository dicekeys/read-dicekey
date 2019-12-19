#include <exception>
#include "symmetric-key.hpp"

void _crypto_secretbox_nonce_salted(
  unsigned char *nonce,
  const unsigned char *secret_key,
  const unsigned char *message,
  const size_t message_length,
  const char* salt,
  const size_t salt_length
) {
    crypto_generichash_state st;

    crypto_generichash_init(&st, NULL, 0U, crypto_box_NONCEBYTES);
    crypto_generichash_update(&st, secret_key, crypto_box_PUBLICKEYBYTES);
    crypto_generichash_update(&st, message, message_length);
    if (salt_length > 0) {
      crypto_generichash_update(&st, (const unsigned char*) salt, salt_length);
    }
    crypto_generichash_final(&st, nonce, crypto_box_NONCEBYTES);
}

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
  const size_t messageLength,
  const std::string &postDecryptionInstructionsJson
) const {
  if (messageLength <= 0) {
    throw std::exception("Invalid message length");
  }
  const size_t compositeCiphertextLength =
    crypto_secretbox_NONCEBYTES + messageLength + crypto_secretbox_MACBYTES;
  std::vector<unsigned char> compositeCiphertext(compositeCiphertextLength);
  unsigned char* noncePtr = compositeCiphertext.data();
  unsigned char* secretBoxStartPtr = noncePtr + crypto_secretbox_NONCEBYTES;

  // Write a nonce derived from the message and symmeetric key
  _crypto_secretbox_nonce_salted(
    noncePtr, derivedKey.data, message, messageLength,
    postDecryptionInstructionsJson.c_str(), postDecryptionInstructionsJson.length());
  
  // Create the ciphertext as a secret box
  crypto_secretbox_easy(
    secretBoxStartPtr,
    message,
    messageLength,
    noncePtr,
    derivedKey.data
  );

  return compositeCiphertext;
}

const std::vector<unsigned char> SymmetricKey::seal(
  const SodiumBuffer &message,
  const std::string &postDecryptionInstructionsJson
) const {
  return seal(message.data, message.length, postDecryptionInstructionsJson);
}


const std::vector<unsigned char> SymmetricKey::seal(
  const Message &message
) const {
  return SymmetricKey::seal(
    message.contents,
    message.postDecryptionInstructionsJson
  );
};

const Message SymmetricKey::unseal(
  const unsigned char* compositeCiphertext,
  const size_t compositeCiphertextLength,
  const std::string &postDecryptionInstructionsJson
) const {
  if (compositeCiphertextLength <= (crypto_secretbox_MACBYTES + crypto_secretbox_NONCEBYTES)) {
    throw std::exception("Invalid message length");
  }
  SodiumBuffer plaintextBuffer(compositeCiphertextLength - (crypto_secretbox_MACBYTES + crypto_secretbox_NONCEBYTES));
  const unsigned char* noncePtr = compositeCiphertext;
  const unsigned char* secretBoxStartPtr = noncePtr + crypto_secretbox_NONCEBYTES;

  const int result = crypto_secretbox_open_easy(
        plaintextBuffer.data,
        secretBoxStartPtr,
        compositeCiphertextLength,
        noncePtr,
        derivedKey.data
      );
   if (result != 0) {
     throw std::exception("Failed to unseal data because either the message or post-decryption instructions were modified or corrupted.");
   }

  // Recalculate nonce to validate that the provided
  // postDecryptionInstructionsJson is valid 
  unsigned char recalculatedNonce[crypto_secretbox_NONCEBYTES];
  _crypto_secretbox_nonce_salted(
    recalculatedNonce, derivedKey.data, plaintextBuffer.data, plaintextBuffer.length,
    postDecryptionInstructionsJson.c_str(), postDecryptionInstructionsJson.length()
  );
  if (memcmp(recalculatedNonce, noncePtr, crypto_secretbox_NONCEBYTES) != 0) {
     throw std::exception("Failed to unseal data because either the message or post-decryption instructions were modified or corrupted.");
  }

  return Message::createAndRemoveAnyEmbedding(plaintextBuffer, postDecryptionInstructionsJson);
};

const Message SymmetricKey::unseal(
  const std::vector<unsigned char> &compositeCiphertext,
  const std::string &postDecryptionInstructionsJson
) const {
  return unseal(compositeCiphertext.data(), compositeCiphertext.size(), postDecryptionInstructionsJson);
}
