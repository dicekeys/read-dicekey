// OCR font data for Inconsolata 700
#pragma once

#include <vector>

#ifndef OCR_ALPHABET
#define OCR_ALPHABET

struct UShortPoint {
	unsigned short x;
	unsigned short y;
};

struct OcrChar {
	char character;
	const std::vector<unsigned char> ifPixelIsWhite;
	const std::vector<unsigned char> ifPixelIsBlack;
	const std::vector<UShortPoint> outlinePoints;
};

struct OcrAlphabet {
	const std::vector<OcrChar> characters;
	const int ocrCharWidthInPixels = 53;
	const int ocrCharHeightInPixels = 70;
	const int outlineCharWidthInPixels = 213;
	const int outlineCharHeightInPixels = 280;
};

#endif

namespace Inconsolata700 {  
	extern const float charWidthOverFontSize;
	extern const float charHeightOverFontSize;
	extern const float fontBaselineFraction;
	
	extern const int ocrCharWidthInPixels;
	extern const int ocrCharHeightInPixels;
	extern const int outlineCharWidthInPixels;
	extern const int outlineCharHeightInPixels;

	extern const OcrAlphabet letters;
	extern const OcrAlphabet digits;

} // end of namespace