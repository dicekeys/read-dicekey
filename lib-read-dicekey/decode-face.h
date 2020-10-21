//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#pragma once

class UndoverlineTypeOrientationAndEncoding {
public:
	bool isValid = false;
	bool wasReadInReverseOrder = false;
	bool isOverline = false;
	unsigned char letterDigitEncoding = 0;

	UndoverlineTypeOrientationAndEncoding() {
	}

	UndoverlineTypeOrientationAndEncoding(
		bool _wasReadInReverseOrder,
		bool _isOverline,
		unsigned char _letterDigitEncoding
	) {
		isValid = true;
		wasReadInReverseOrder = _wasReadInReverseOrder;
		isOverline = _isOverline;
		letterDigitEncoding = _letterDigitEncoding;
	}
};

UndoverlineTypeOrientationAndEncoding decodeUndoverline11Bits(
	unsigned int binaryCodingReadForwardOrBackward,
	bool isVertical
);