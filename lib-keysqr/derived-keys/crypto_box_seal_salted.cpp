/************************************
 * This is a small modification to crypto_box_seal.c in libsodium to
 * allow a salt to be added to public-key sealed boxes so that
 * DiceKeys can support messages that require processing of
 * post-decryption instructions
 */
#include <string.h>
#include "sodium.h"
#include "crypto_box_seal_salted.h"

void _crypto_box_seal_nonce_salted(
  unsigned char *nonce,
  const unsigned char *pk1, const unsigned char *pk2,
  const char* salt,
  const size_t salt_length
) {
    crypto_generichash_state st;

    crypto_generichash_init(&st, NULL, 0U, crypto_box_NONCEBYTES);
    crypto_generichash_update(&st, pk1, crypto_box_PUBLICKEYBYTES);
    crypto_generichash_update(&st, pk2, crypto_box_PUBLICKEYBYTES);
    if (salt_length > 0) {
      crypto_generichash_update(&st, (const unsigned char*) salt, salt_length);
    }
    crypto_generichash_final(&st, nonce, crypto_box_NONCEBYTES);
}

int
crypto_box_salted_seal(
  unsigned char *c, const unsigned char *m,
  unsigned long long mlen, const unsigned char *pk,
  const char* salt,
  const size_t salt_length  
)
{
    unsigned char nonce[crypto_box_NONCEBYTES];
    unsigned char epk[crypto_box_PUBLICKEYBYTES];
    unsigned char esk[crypto_box_SECRETKEYBYTES];
    int           ret;

    if (crypto_box_keypair(epk, esk) != 0) {
        return -1; /* LCOV_EXCL_LINE */
    }
    memcpy(c, epk, crypto_box_PUBLICKEYBYTES);
    _crypto_box_seal_nonce_salted(nonce, epk, pk, salt, salt_length);
    ret = crypto_box_easy(c + crypto_box_PUBLICKEYBYTES, m, mlen,
                          nonce, pk, esk);
    sodium_memzero(esk, sizeof esk);
    sodium_memzero(epk, sizeof epk);
    sodium_memzero(nonce, sizeof nonce);

    return ret;
}

int
crypto_box_salted_seal_open(
  unsigned char *m, const unsigned char *c,
  unsigned long long clen,
  const unsigned char *pk, const unsigned char *sk,
  const char* salt, const size_t salt_length
)
{
    unsigned char nonce[crypto_box_NONCEBYTES];

    if (clen < crypto_box_SEALBYTES) {
        return -1;
    }
    _crypto_box_seal_nonce_salted(nonce, c, pk, salt, salt_length);

    // COMPILER_ASSERT(crypto_box_PUBLICKEYBYTES < crypto_box_SEALBYTES);
    return crypto_box_open_easy(m, c + crypto_box_PUBLICKEYBYTES,
                                clen - crypto_box_PUBLICKEYBYTES,
                                nonce, c, sk);
}

