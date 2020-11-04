//  © 2019 Stuart Edward Schechter (Github: @uppajung)
#include <float.h>
#include "utilities/statistics.h"
#include "graphics/cv.h"
#include "graphics/geometry.h"
#include "find-undoverlines.h"
#include "read-faces.h"
#include "find-faces.h"
#include "../lib-dicekey/externally-generated/dicekey-face-specification.h"

FaceAndStrayUndoverlinesFound findFacesAndStrayUndoverlines(
	const cv::Mat &grayscaleImage
) {
	const auto undoverlines = findReadableUndoverlines(grayscaleImage);

	std::vector<Undoverline> underlines(undoverlines.underlines);
	std::vector<Undoverline> overlines(undoverlines.overlines);

	std::vector<float> underlineLengths = vmap<Undoverline, float>(underlines,
		[](const Undoverline *underline) { return lineLength(underline->line); });
	const float medianUnderlineLengthInPixels = medianInPlace(underlineLengths);
	const float pixelsPerFaceEdgeWidth = medianUnderlineLengthInPixels / FaceDimensionsFractional::undoverlineLength;
	const float maxErrorDistance = pixelsPerFaceEdgeWidth / 4; // 2mm

	std::vector<Undoverline> strayUndoverlines(0);
	std::vector<FaceUndoverlines> facesFound;

	for (auto underline : underlines) {
		// For each underline, find the overline that appears to the belong to the same face
		// by examining each overline and finding the one with the smallest error distance.

		// The error distance is the difference between the location of the overline,
		// and the location it should be if we use the underline to derive it's location.
		// Add to that the difference between the location of the underline, and the 
		// location we would derive it should be from the information we have about the overline.

		// Search for overline with inferred face center near that of underline.
		int bestMatchIndex = -1;
		float bestMatchErrorDistance = maxErrorDistance;
		for (int i = 0; i < overlines.size(); i++) {
			const Undoverline& overline = overlines[i];
			const float errorDistance =
				distance2f(underline.center, overline.inferredOpposingUndoverlineCenter) +
				distance2f(overline.center, underline.inferredOpposingUndoverlineCenter);
			if (errorDistance <= bestMatchErrorDistance) {
				bestMatchIndex = i;
				bestMatchErrorDistance = errorDistance;
			}
		}
		if (bestMatchIndex != -1) {
			// We found the closest match, and it's close enough, so we now have a complete face.
			facesFound.push_back(FaceUndoverlines(
				underline, overlines[bestMatchIndex]
			));
			// Remove the paired overline
			overlines.erase(overlines.begin() + bestMatchIndex);
		} else {
			strayUndoverlines.push_back(underline);
		}
	}

	// Concatenate the stray overlines onto the list of stray underlines to complete the list of
	// stray undoverlines.
	strayUndoverlines.insert(strayUndoverlines.end(), overlines.begin(), overlines.end());

	return { facesFound, strayUndoverlines, pixelsPerFaceEdgeWidth };
}
