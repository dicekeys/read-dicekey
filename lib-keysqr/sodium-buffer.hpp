#pragma once

#include "sodium-initializer.hpp"
#include <sodium.h>
#include <memory.h>

class SodiumBuffer {
  public:
  unsigned char* data;
  const size_t length;

  SodiumBuffer(size_t length, const unsigned char* bufferData = NULL): length(length) {
    ensureSodiumInitialized();
    data = (unsigned char*) sodium_malloc(length);
    if (bufferData != NULL) {
      memcpy(data, bufferData, length);
    }
  }



  SodiumBuffer(const SodiumBuffer &other): length(other.length) {
    ensureSodiumInitialized();
    data = (unsigned char*) sodium_malloc(length);
    memcpy(data, other.data, length);
  }

  ~SodiumBuffer() {
    sodium_free(data);
  }

  const std::vector<unsigned char> toVector() const {
    std::vector<unsigned char> v(length);
    memcpy(v.data(), data, length);
    return v;
  }

};
