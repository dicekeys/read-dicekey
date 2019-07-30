#pragma once

#include <string>
#include <vector>
#include "die.h"


static unsigned int reverseBits(unsigned int bitsToReverse, unsigned int lengthInBits)
{
	unsigned int reversedBits = 0;
	for (unsigned int i = 0; i < lengthInBits; i++) {
		reversedBits <<= 1;
		if (bitsToReverse & 1) {
			reversedBits += 1;
		}
		bitsToReverse >>= 1;
	}
	return reversedBits;
}

struct decodeUndoverlineBitsResult {
//	bool binaryCodingReadForwardOrBackward
	bool isValid;
	bool wasReadInReverseOrder;
	bool isOverline;
	char letter;
	char digit;
	unsigned char numberOf90DegreeeClockwiseRotationsFromUpright;
};

static decodeUndoverlineBitsResult decodeUndoverlineBits(unsigned int binaryCodingReadForwardOrBackward, bool isVertical) {
	decodeUndoverlineBitsResult result;
	result.isValid = false;
	result.letter = 0;
	result.digit = 0;
	result.numberOf90DegreeeClockwiseRotationsFromUpright = 0;
	result.wasReadInReverseOrder = 0;

	bool decodeErrorPresent = false;

	// The binary coding has 11 bits,
	// from most significant (10) to least (0)
	//   Bit 10:   always 1
	//   Bit  9:   1 if overline, 0 if underline
	//   Bits 8-1: byte encoding letter and digit
	//   Bit  0:   always 0

	// First, we need to determine if we read the code in the
	// forward order (the first bit is 1) or the reverse order
	// (such that the last bit is 1).
	const bool firstBitRead = ( binaryCodingReadForwardOrBackward >> (NumberOfDotsInUndoverline - 1) ) == 1;
	const bool lastBitRead = binaryCodingReadForwardOrBackward & 1;
	if ( (firstBitRead ^ lastBitRead) != 1 ) {
		// The direction bit is a white box at the left (starting) edge
		// of the underline.
		// Thus, either the first or last bit should be set to 1,
		// but never neither or both.
		result.isValid = false;
		return result;
	}

	result.wasReadInReverseOrder = lastBitRead;

	result.numberOf90DegreeeClockwiseRotationsFromUpright = isVertical ? (
			// vertical
			result.wasReadInReverseOrder ? 3 : 1
		) : (
			// horizontal
			result.wasReadInReverseOrder ? 2 : 0
	);

	// Get the bits in the correct order, with the lowest-order
	// bit 0 (the always-black last bit in the underline)
	const unsigned int binaryEncoding = result.wasReadInReverseOrder ?
		reverseBits(binaryCodingReadForwardOrBackward, 11) : binaryCodingReadForwardOrBackward;

	result.isOverline = (binaryEncoding >> 9) & 1;
	const unsigned char letterDigitEncodingByte = (binaryEncoding >> 1) & 0xff;

	const auto dieFaceCodes = result.isOverline ?
	overlineCodeToLetterIndexTimesSixPlusDigitIndex[letterDigitEncodingByte] :
	underlineCodeToLetterIndexTimesSixPlusDigitIndex[letterDigitEncodingByte];

	decodeErrorPresent = dieFaceCodes.letter == 0;
	result.letter = dieFaceCodes.letter;
	result.digit = dieFaceCodes.digit;

	result.isValid = !decodeErrorPresent;
	return result;
}