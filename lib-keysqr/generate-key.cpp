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

  size_t slowHashPreimageLength =
    // length of the keysqr in human readable format
    keySqrInHumanReadableFormat.length() +
    // 1 character for a null char between the two strings
    1 +
    // length of the json string specifying the key generation options
    keyGenerationOptions.keyGenerationOptionsJsonString.length();

  unsigned char *slowHashPreimage = (unsigned char*)sodium_malloc(slowHashPreimageLength);
  if (slowHashPreimage == NULL) {
    throw "Insufficient memory";
  }

  memcpy(
    slowHashPreimage,
    keySqrInHumanReadableFormat.c_str(),
    keySqrInHumanReadableFormat.length()
  );
  keySqrInHumanReadableFormat[keySqrInHumanReadableFormat.length()] = '0';
  memcpy(
    slowHashPreimage + keySqrInHumanReadableFormat.length() + 1,
    keyGenerationOptions.keyGenerationOptionsJsonString.c_str(),
    keyGenerationOptions.keyGenerationOptionsJsonString.length()
  );

  keyGenerationOptions.hashFunction->hash(
    keyGeneratedOutput,
    slowHashPreimage,
    slowHashPreimageLength
  );

  // sodium_memzero(keySqrInHumanReadableFormat.c_str, keySqrInHumanReadableFormat.size());

  sodium_free(slowHashPreimage);
}
