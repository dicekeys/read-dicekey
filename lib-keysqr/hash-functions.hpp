#pragma once

#include <vector>
#include "sodium.h"


class HashFunction {
	public:
	virtual size_t hash_size_in_bytes() const = 0;
	virtual int hash(
		void* hash_output,
		const void* message,
		unsigned long long message_length
	) const = 0;

	void hash(
		std::vector<unsigned char>hash_output,
		const void* message,
		unsigned long long message_length
	) const;

	void hash(
		std::vector<unsigned char>hash_output,
		const std::vector<unsigned char>message
	) const;
};

class HashFunctionBlake2b : public HashFunction {
	size_t hash_size_in_bytes() const { return crypto_generichash_BYTES; }
	
	int hash(
		void* hash_output,
		const void* message,
		unsigned long long message_length
	) const;
};

class HashFunctionSHA256 : public HashFunction {
	size_t hash_size_in_bytes() const;
	
	int hash(
		void* hash_output,
		const void* message,
		unsigned long long message_length
	) const;
};


class HashFunctionArgon2id : public HashFunction {
	private:
		unsigned long long hash_output_size_in_bytes;
		unsigned long long opslimit;
		size_t memlimit;
	public:

	HashFunctionArgon2id(
		unsigned long long _hash_output_size_in_bytes,
		unsigned long long _opslimit,
		size_t _memlimit
	);
  size_t hash_size_in_bytes() const { return hash_output_size_in_bytes; }
	
  int hash(
    void* hash_output,
    const void* message,
    unsigned long long message_length
  ) const;
};

class HashFunctionScrypt : public HashFunction {
	private:
		unsigned long long hash_output_size_in_bytes;
		unsigned long long opslimit;
		size_t memlimit;
	public:

	HashFunctionScrypt(
		unsigned long long _hash_output_size_in_bytes,
		unsigned long long _opslimit,
		size_t _memlimit
	);
  size_t hash_size_in_bytes() const { return hash_output_size_in_bytes; }
	
  int hash(
    void* hash_output,
    const void* message,
    unsigned long long message_length
  ) const;
};
