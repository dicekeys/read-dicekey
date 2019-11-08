#include <cassert>
#include <vector>
// #include "sodium.h"

#include "keysqr.h"
#include "../includes/sodium.h"
#include "generate-key.hpp"

/**
 * All permission checks must take place BEFORE this is called.
 */
void generateKey(
  std::vector<unsigned char> keyGeneratedOutput,
  const KeySqr<Face> &keySqr,
  const KeyGenerationOptions &keyGenerationOptions
) {
  assert(keyGeneratedOutput.size() == keyGenerationOptions.keyLengthInBytes);
  std::string keySqrInHumanReadableFormat = keySqr.toHumanReadableForm(keyGenerationOptions.includeOrientationOfFacesInKey);
  size_t fasterHashSize = keyGenerationOptions.fasterHashFunction->hash_size_in_bytes();
  size_t slowHashPreimageLength = 2 * fasterHashSize;
  unsigned char *slowHashPreimage = (unsigned char*)sodium_malloc(slowHashPreimageLength);
  // [ fasterHash(keyGenerationOptionsJsonStirng) o fasterHash(keySquareInHumanReadableForm) ]
  keyGenerationOptions.fasterHashFunction->hash(
    slowHashPreimage,
    keyGenerationOptions.keyGenerationOptionsJsonString.c_str(),
    keyGenerationOptions.keyGenerationOptionsJsonString.length()
  );
  keyGenerationOptions.fasterHashFunction->hash(
    slowHashPreimage + fasterHashSize,
    keySqrInHumanReadableFormat.c_str(),
    keyGenerationOptions.keyGenerationOptionsJsonString.length()
  );
  keyGenerationOptions.slowerHashFunction->hash(
    keyGeneratedOutput,
    slowHashPreimage, slowHashPreimageLength
  );

  // sodium_memzero(keySqrInHumanReadableFormat.c_str, keySqrInHumanReadableFormat.size());

  sodium_free(slowHashPreimage);
}
