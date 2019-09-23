//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#pragma once

#include <string>
#include <vector>
#include "die-face-specification.h"
#include "utilities/bit-operations.h"
#include "decode-die.h"

UndoverlineTypeOrientationAndEncoding decodeUndoverline11Bits(unsigned int binaryCodingReadForwardOrBackward, bool isVertical) {
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
		return {false};
	}

	const bool wasReadInReverseOrder = lastBitRead;


	// Get the bits in the correct order, with the lowest-order
	// bit 0 (the always-black last bit in the underline)
	const unsigned int binaryEncoding = wasReadInReverseOrder ?
		reverseBits(binaryCodingReadForwardOrBackward, 11) : binaryCodingReadForwardOrBackward;

	const bool isOverline = (binaryEncoding >> 9) & 1;
	const unsigned char letterDigitEncoding = (binaryEncoding >> 1) & 0xff;
	return {
		true, // isValid = true, encdoing was valid
		wasReadInReverseOrder,
		isOverline,
//		orientationAs0to3ClockwiseTurnsFromUpright,
		letterDigitEncoding
	};
}
