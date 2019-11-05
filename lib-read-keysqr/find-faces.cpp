//  © 2019 Stuart Edward Schechter (Github: @uppajung)
#include <float.h>
#include "utilities/statistics.h"
#include "graphics/cv.h"
#include "graphics/geometry.h"
#include "find-undoverlines.h"
#include "read-faces.h"
#include "find-faces.h"
#include "keysqr-face-specification.h"

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
	const float maxDistanceBetweenInferredCenters = pixelsPerFaceEdgeWidth / 4; // 2mm

	std::vector<Undoverline> strayUndoverlines(0);
	std::vector<FaceUndoverlines> facesFound;

	for (auto underline : underlines) {
		// Search for overline with inferred face center near that of underline.
		bool found = false;
		for (size_t i = 0; i < overlines.size() && !found; i++) {
			if (distance2f(underline.inferredCenterOfFace, overlines[i].inferredCenterOfFace) <= maxDistanceBetweenInferredCenters) {
				// We have a match
				found = true;
				// Re-infer the center of the face and its angle by drawing a line from
				// the center of the to the center of the overline.
				facesFound.push_back( FaceUndoverlines(
					underline, overlines[i]
				));
				// Remove the ith element of overlines
				overlines.erase(overlines.begin() + i);
			}
		}
		if (!found) {
			strayUndoverlines.push_back(underline);
		}
	}

	strayUndoverlines.insert(strayUndoverlines.end(), overlines.begin(), overlines.end());

	return { facesFound, strayUndoverlines, pixelsPerFaceEdgeWidth };
}
