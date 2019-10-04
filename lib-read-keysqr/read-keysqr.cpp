//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#include <float.h>
#include <chrono>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include <iostream>
#include <math.h>

#include "keysqr.h"
#include "read-faces.h"
#include "read-keysqr.h"
#include "visualize-read-results.h"

const unsigned int maxCorrectableError = 2;
const int millisecondsToTryToRemoveCorrectableErrors = 4000;

/**
 * This function is the base for an augmented reality loop in which
 * we scan images from the camera repeatedly until we have successfully
 * scanned a KeySqr.
 * 
 * The first parameter should be the last frame from the camera,
 * as an OpenCV Mat (image) in 8-bit unsigned BGR (blue, green, red)
 * format.
 * 
 * Your event loop should allocate a ResultOfScanAndAugmentKeySqrImage
 * struct and pass it to every call.  This function will consume your
 * previous result to compare it with the new scan, then write the
 * new result over the old one.
 * 
 * The function will return when there is a KeySqr to return to
 * the caller and false if scanning should continue.
 * 
 * If generating an API for callers that can consume the
 * ResultOfScanAndAugmentKeySqrImage struct directly, simply
 * return that struct on terminating. 
 * For APIs that cannot consume the struct directly, call the toJson()
 * method of the keySqr field (result.keySqr.toJson()) to get a
 * std::string in JSON format that can be returned back to any
 * consumer that can parse JSON format.
 **/
bool scanAndAugmentKeySqrImage(
	cv::Mat &sourceColorImageBGR_CV_8UC3,
	ResultOfScanAndAugmentKeySqrImage* result
) {
	const ReadFaceResult facesRead = readFaces(sourceColorImageBGR_CV_8UC3, false);

	const KeySqr latestKeySqr = facesRead.success ? facesReadToKeySqr(facesRead.faces, false) : KeySqr();
	
	const KeySqr mergedKeySqr = (!result->initialized) ? latestKeySqr :
		latestKeySqr.initialized ?
			latestKeySqr.mergePrevious(result->keySqr) :
			latestKeySqr;

	result->whenLastRead = std::chrono::system_clock::now();
	result->whenFirstRead = (result->initialized) ? result->whenFirstRead : result->whenLastRead;
	result->whenLastImproved =
		(!result->initialized || result->keySqr.totalError() > mergedKeySqr.totalError()) ?
			result->whenLastRead : result->whenLastImproved;

	// We're done when we either have an error-free scan, or no face that can't be improved after
	const bool terminate = (
			// We have an error free scan, or...
			mergedKeySqr.totalError() == 0
		) || (
			// We have only correctable errors, and we've our budget of time hoping more
			// scanning will remove those errors.
			mergedKeySqr.maxError() <= maxCorrectableError &&
			std::chrono::duration_cast<std::chrono::milliseconds>(result->whenLastRead - result->whenLastImproved).count() >
				millisecondsToTryToRemoveCorrectableErrors
		);
	// auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(foo - now).count;

	result->keySqr = mergedKeySqr;
	result->augmentedColorImage_BGR_CV_8UC3 = visualizeReadResults(sourceColorImageBGR_CV_8UC3, facesRead, false);
	return terminate;
}