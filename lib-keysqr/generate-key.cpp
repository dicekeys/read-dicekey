#include <cassert>
#include <vector>

#include <sodium.h>

#include "keysqr.h"
#include "generate-key.hpp"


void generateKey(
  void* keyGeneratedOutput,
  size_t keyGenerationOutputLengthInBytes,
  const KeySqr<Face> &keySqr,
  const KeyGenerationOptions &keyGenerationOptions,
  const KeyGenerationOptionsJson::Purpose mandatePurpose
) {
  if (
    mandatePurpose != KeyGenerationOptionsJson::_INVALID_PURPOSE_ &&
    mandatePurpose != keyGenerationOptions.purpose  
  ) {
    throw ("Key generation options must have purpose " + mandatePurpose);
  }
  if(keyGenerationOutputLengthInBytes != keyGenerationOptions.keyLengthInBytes) {
    throw "Invalid length of key to generate";
  };
  std::string keySqrInHumanReadableForm =
    keySqr.toHumanReadableForm(keyGenerationOptions.includeOrientationOfFacesInKey);

  size_t slowHashPreimageLength =
    // length of the keysqr in human readable format
    keySqrInHumanReadableForm.length() +
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
    keySqrInHumanReadableForm.c_str(),
    keySqrInHumanReadableForm.length()
  );
  keySqrInHumanReadableForm[keySqrInHumanReadableForm.length()] = '0';
  memcpy(
    slowHashPreimage + keySqrInHumanReadableForm.length() + 1,
    keyGenerationOptions.keyGenerationOptionsJsonString.c_str(),
    keyGenerationOptions.keyGenerationOptionsJsonString.length()
  );

  const int nonZeroHashResultMeansOutOfMemoryError = keyGenerationOptions.hashFunction->hash(
    keyGeneratedOutput,
    slowHashPreimage,
    slowHashPreimageLength
  );

  // sodium_memzero(keySqrInHumanReadableForm.c_str, keySqrInHumanReadableForm.size());

  sodium_free(slowHashPreimage);

	if (nonZeroHashResultMeansOutOfMemoryError != 0) {
		throw "Insufficient memory";
	}

}


void generateKey(
  std::vector<unsigned char> &keyGeneratedOutput,
  const KeySqr<Face> &keySqr,
  const KeyGenerationOptions &keyGenerationOptions,
  const KeyGenerationOptionsJson::Purpose mandatePurpose
) {
  return generateKey(
    keyGeneratedOutput.data(),
    keyGeneratedOutput.size(),
    keySqr,
    keyGenerationOptions,
    mandatePurpose
  );
}
