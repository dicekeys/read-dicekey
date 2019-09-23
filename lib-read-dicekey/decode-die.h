//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#pragma once

struct UndoverlineTypeOrientationAndEncoding {
	bool isValid = false;
	bool wasReadInReverseOrder = false;
	bool isOverline = false;
	unsigned char letterDigitEncoding = 0;
};

UndoverlineTypeOrientationAndEncoding decodeUndoverline11Bits(
	unsigned int binaryCodingReadForwardOrBackward,
	bool isVertical
);