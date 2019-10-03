#pragma once

#include <vector>

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

typedef std::vector<OcrChar> OcrAlphabet;

struct OcrFont {
	const float charWidthOverFontSize;
	const float charHeightOverFontSize;
	const float fontBaselineFraction;
	const int ocrCharWidthInPixels;
	const int ocrCharHeightInPixels;
	const int outlineCharWidthInPixels;
	const int outlineCharHeightInPixels;
	const OcrAlphabet letters;
	const OcrAlphabet digits;
};

