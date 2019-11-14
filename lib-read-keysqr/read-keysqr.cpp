//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#include <float.h>
#include <chrono>
#include <iostream>
#include <math.h>

#include "utilities/bit-operations.h"
#include "graphics/cv.h"
#include "keysqr.h"
#include "read-faces.h"
#include "read-keysqr.h"
#include "visualize-read-results.h"



KeySqr<FaceRead> facesReadToKeySqr(
	const std::vector<FaceRead> &facesRead //,
//	bool reportErrsToStdErr
) {
	if (facesRead.size() != 25) {
		throw std::string("A KeySqr must contain 25 faces but only has " + std::to_string(facesRead.size()));
	}
	return KeySqr<FaceRead>(facesRead);
}


KeySqr<FaceRead> readKeySqr(
	const cv::Mat &grayscaleImage,
	bool outputErrors
) {
  const ReadFaceResult facesRead = readFaces(grayscaleImage, outputErrors);
  if (!facesRead.success || facesRead.faces.size() != NumberOfFaces) {
    return KeySqr<FaceRead>();
  } else {
    return KeySqr<FaceRead>(facesRead.faces);
  }
}

std::string readKeySqrJson(
	const cv::Mat &grayscaleImage
) {
	try {
	  KeySqr<FaceRead> keySqr = readKeySqr(grayscaleImage, false);
	  return keySqr.toJson();
	} catch (...) {
		return "null";
	}
}


std::string readKeySqrJson (
	int width,
	int height,
	size_t bytesPerRow,
	void* data
) {
  const cv::Mat grayscaleImage(cv::Size(width, height), CV_8UC1, data, bytesPerRow);
  return readKeySqrJson(grayscaleImage);
}

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

	const KeySqr<FaceRead> latestKeySqr = (facesRead.success && facesRead.faces.size() == NumberOfFaces) ?
		KeySqr<FaceRead>(facesRead.faces) :
		KeySqr<FaceRead>();
	
	const KeySqr<FaceRead> mergedKeySqr = (!result->initialized) ? latestKeySqr :
		latestKeySqr.isInitialized() ?
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