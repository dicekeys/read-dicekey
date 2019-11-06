#pragma once

#include "keysqr.h"
// #include "sodium.h"
#include "../includes/sodium.h"

class KeySqrCrypto {
private:
  inline static bool hasInitializedSodium = false;
public:

  KeySqrCrypto()
  {
    if (!hasInitializedSodium) {
      if (sodium_init() < 0) {
        throw "Could not initialize sodium";
        /* panic! the library couldn't be initialized, it is not safe to use */
      }
      hasInitializedSodium = true;
    }
  }

  const std::vector<unsigned char> getSymmetricKey()
  {
    //
    std::vector<char> key(crypto_secretbox_KEYBYTES);
    // FIXME
  }

  const std::vector<char> seal(
    const std::vector<unsigned char> message
  ) {
    const size_t message_len = message.size();
    const size_t ciphertext_len = crypto_secretbox_MACBYTES + message.size();
    std::vector<unsigned char> ciphertext(ciphertext_len);

    const std::vector<unsigned char> key = getSymmetricKey();

    std::vector<unsigned char> nonce(crypto_secretbox_NONCEBYTES);
    randombytes_buf(nonce.data(), nonce.size());

    //crypto_secretbox_keygen(key.data());
    crypto_secretbox_easy(ciphertext.data(), message.data(), message.size(), nonce.data(), key.data());

    // nonce must be sent along with ciphertext.  why not put them together?

    std::vector<unsigned char> unsealed(ciphertext.size() - crypto_secretbox_MACBYTES);
    if (crypto_secretbox_open_easy(unsealed.data(), ciphertext.data(), ciphertext.size(), nonce.data(), key.data()) != 0) {
        /* message forged! */
    }

  }




};




class KeySqrCryptoWithDiceKey : public KeySqrCrypto {
private:
  const KeySqr<IFace> &keysqr;

  KeySqrCryptoWithDiceKey(KeySqr<IFace> &_keysqr) :
    keysqr(_keysqr),
    KeySqrCrypto()
  {
  }
};