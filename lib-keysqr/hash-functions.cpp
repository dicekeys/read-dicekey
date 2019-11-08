#include <cassert>
#include "hash-functions.hpp"

void HashFunction::hash(
  std::vector<unsigned char>hash_output,
  const void* message,
  unsigned long long message_length
) const {
  assert(hash_output.size() == hash_size_in_bytes());
  hash(hash_output.data(), (const unsigned*)message, message_length);
};

void HashFunction::hash(
  std::vector<unsigned char>hash_output,
  const std::vector<unsigned char>message
) const {
  assert(hash_output.size() == hash_size_in_bytes());
  hash(hash_output.data(), message.data(), message.size());
};


size_t HashFunctionBlake2b::hash_size_in_bytes() const { return crypto_generichash_BYTES; }
	
int HashFunctionBlake2b::hash(
  void* hash_output,
  const void* message,
  unsigned long long message_length
) const {
  return crypto_generichash(
    (unsigned char*)hash_output, crypto_generichash_BYTES,
    (const unsigned char*)message, message_length,
    NULL, 0);
}

size_t HashFunctionSHA256::hash_size_in_bytes() const { return crypto_hash_sha256_BYTES; }

int HashFunctionSHA256::hash(
  void* hash_output,
  const void* message,
  unsigned long long message_length
) const {
  return crypto_hash_sha256((unsigned char*)hash_output, (const unsigned char*)message, message_length);
}

HashFunctionArgon2id::HashFunctionArgon2id(
  unsigned long long _hash_output_size_in_bytes,
  unsigned long long _opslimit,
  size_t _memlimit
) :
  hash_output_size_in_bytes(_hash_output_size_in_bytes),
  opslimit(_opslimit),
  memlimit(_memlimit)
{
  assert(
    hash_output_size_in_bytes >= crypto_pwhash_argon2id_BYTES_MIN &&
    hash_output_size_in_bytes <= crypto_pwhash_argon2id_BYTES_MAX
  );
}

size_t HashFunctionArgon2id::hash_size_in_bytes() const { return hash_output_size_in_bytes; }
	
int HashFunctionArgon2id::hash(
  void* hash_output,
  const void* message,
  unsigned long long message_length
) const {
  // Argon2id requires a 16-byte salt.
  // Since this is only used with messages that are already salted,
  // we use a salt of 16 zero bytes.
  static const unsigned char zero_bytes_for_salt[crypto_pwhash_argon2id_SALTBYTES] =
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  return crypto_pwhash(
    (unsigned char*)hash_output, hash_size_in_bytes(),
    (const char*)message, message_length,
    zero_bytes_for_salt,
    opslimit,
    memlimit,
    crypto_pwhash_ALG_ARGON2ID13
  );
};
