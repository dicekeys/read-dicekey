//  © 2019 Stuart Edward Schechter (Github: @uppajung)
#include <float.h>
#include "utilities/statistics.h"
#include "graphics/cv.h"
#include "graphics/geometry.h"
#include "find-undoverlines.h"
#include "read-faces.h"
#include "find-faces.h"
#include "keysqr-element-face-specification.h"

FacesAndStrayUndoverlinesFound findFacesAndStrayUndoverlines(
	const cv::Mat &grayscaleImage
) {
	const auto undoverlines = findReadableUndoverlines(grayscaleImage);

	std::vector<Undoverline> underlines(undoverlines.underlines);
	std::vector<Undoverline> overlines(undoverlines.overlines);

	std::vector<float> underlineLengths = vmap<Undoverline, float>(underlines,
		[](const Undoverline *underline) { return lineLength(underline->line); });
	const float medianUnderlineLengthInPixels = medianInPlace(underlineLengths);
	const float pixelsPerFaceEdgeWidth = medianUnderlineLengthInPixels / ElementDimensionsFractional::undoverlineLength;
	const float maxDistanceBetweenInferredCenters = pixelsPerFaceEdgeWidth / 4; // 2mm

	std::vector<Undoverline> strayUndoverlines(0);
	std::vector<FaceRead> facesFound;

	for (auto underline : underlines) {
		// Search for overline with inferred face center near that of underline.
		bool found = false;
		for (size_t i = 0; i < overlines.size() && !found; i++) {
			if (distance2f(underline.inferredCenterOfFace, overlines[i].inferredCenterOfFace) <= maxDistanceBetweenInferredCenters) {
				// We have a match
				found = true;
				// Re-infer the center of the face and its angle by drawing a line from
				// the center of the to the center of the overline.
				const Line lineFromUnderlineCenterToOverlineCenter = {
					midpointOfLine(underline.line), midpointOfLine(overlines[i].line)
				};
				// The center of the face is the midpoint of that line.
				const cv::Point2f center = midpointOfLine(lineFromUnderlineCenterToOverlineCenter);
				// The angle of the face is the angle of that line, plus 90 degrees clockwise
				const float angleOfLineFromUnderlineToOverlineCenterInRadians =
					angleOfLineInSignedRadians2f(lineFromUnderlineCenterToOverlineCenter);
				float angleInRadians = angleOfLineFromUnderlineToOverlineCenterInRadians +
					NinetyDegreesAsRadians;
				if (angleInRadians > (M_PI)) {
					angleInRadians -= float(2 * M_PI);
				}
				facesFound.push_back({
					underline, overlines[i], center, angleInRadians
				});
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
