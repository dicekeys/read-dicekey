#include <cassert>
#include <vector>

#include <sodium.h>

#include "keysqr.h"
#include "derive-key.hpp"


void generateKey(
  void* keyGeneratedOutput,
  size_t keyDerivationOutputLengthInBytes,
  const KeySqr<Face> &keySqr,
  const KeyDerivationOptions &keyDerivationOptions,
  const KeyDerivationOptionsJson::Purpose mandatePurpose
) {
  if (
    mandatePurpose != KeyDerivationOptionsJson::_INVALID_PURPOSE_ &&
    mandatePurpose != keyDerivationOptions.purpose  
  ) {
    throw ("Key generation options must have purpose " + std::to_string(mandatePurpose));
  }
  if(keyDerivationOutputLengthInBytes != keyDerivationOptions.keyLengthInBytes) {
    throw "Invalid length of key to generate";
  };
  std::string keySqrInHumanReadableForm =
    keySqr.toHumanReadableForm(keyDerivationOptions.includeOrientationOfFacesInKey);

  size_t slowHashPreimageLength =
    // length of the keysqr in human readable format
    keySqrInHumanReadableForm.length() +
    // 1 character for a null char between the two strings
    1 +
    // length of the json string specifying the key generation options
    keyDerivationOptions.keyDerivationOptionsJsonString.length();

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
    keyDerivationOptions.keyDerivationOptionsJsonString.c_str(),
    keyDerivationOptions.keyDerivationOptionsJsonString.length()
  );

  const int nonZeroHashResultMeansOutOfMemoryError = keyDerivationOptions.hashFunction->hash(
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
  const KeyDerivationOptions &keyDerivationOptions,
  const KeyDerivationOptionsJson::Purpose mandatePurpose
) {
  return generateKey(
    keyGeneratedOutput.data(),
    keyGeneratedOutput.size(),
    keySqr,
    keyDerivationOptions,
    mandatePurpose
  );
}
