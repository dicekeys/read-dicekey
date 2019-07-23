#pragma once
#include <string>

//const char* alphabet = "ABCDEFGHIJKLMNOPRSTUVWXYZ123456";
std::string DieLetters = "ABCDEFGHIJKLMNOPRSTUVWXYZ";
std::string DieDigits = "123456";

const float mmDieSize = 8;
const float mmDieMargin = 0.75;
const float mmDieUndoverlineLength = 6.0f;
const float mmDieUndoverlineWidth = 1.0f;
const float mmBetweenUndoverlines = mmDieSize - 2.0f * (mmDieMargin + mmDieUndoverlineWidth);
const float mmFromCenterOfUndoverlineToCenterOfDie = (mmBetweenUndoverlines + mmDieUndoverlineWidth) / 2.0f;

const float mmDieTextRegionWidth = 6.0f;
const float mmDieTextRegionHeight = mmBetweenUndoverlines - 0.6f;

const float undoverlineWidthOverLength = mmDieUndoverlineWidth / mmDieUndoverlineLength;


const float minWidthOverLength = undoverlineWidthOverLength / 1.5f;
const float maxWidthOverLength = undoverlineWidthOverLength * 1.5f;

static bool isRectangleShapedLikeUndoverline(RectangleDetected rect) {
	float shortToLongRatio = rect.shorterSideLength / rect.longerSideLength;
	return (
		shortToLongRatio >= minWidthOverLength &&
		shortToLongRatio <= maxWidthOverLength
		);
}
