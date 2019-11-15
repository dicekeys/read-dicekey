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
bool KeySqrImageReader::processImage(
		int width,
		int height,
		size_t bytesPerRow,
		void* pointerToGrayscaleChannelByteArray
) {
  const cv::Mat grayscaleImage(cv::Size(width, height), CV_8UC1, pointerToGrayscaleChannelByteArray, bytesPerRow);

	const ReadFaceResult facesRead = readFaces(grayscaleImage, false);

	whenLastRead = std::chrono::system_clock::now();
	if (!initialized) {
		whenFirstRead = whenLastRead;
		whenLastImproved = whenLastRead;
		initialized = true;
	}

	if (facesRead.success && facesRead.faces.size() == NumberOfFaces) {
		// We have successfully read in a full KeySqr of faces which
		// should update the any keySqr we have previously read in.

		const KeySqr<FaceRead> previousKeySqr = keySqr;
		
		if (previousKeySqr.isInitialized()) {
			// There may be useful data from the previous read to carry in,
			// as it could have read something this read missed.
			// Merge the old into the new
			keySqr = KeySqr<FaceRead>(facesRead.faces).mergePrevious(keySqr);
			if (keySqr.totalError() > previousKeySqr.totalError()) {
				//The new read reduces the magnitude of the read errors to resolve
				whenLastImproved = whenLastRead;
			}
		} else {
			// This is the first time that a set of faces has been read
			keySqr = KeySqr<FaceRead>(facesRead.faces);
			whenLastImproved = whenLastRead;
		}
	}

	// The process of repeatedly processing camera images should stop when either
	//   1. we have a scan that is free of errors
	//   2. the errors observed are correctable with high probability AND
	//      we haven't been able to process an image that corrects them for the
	//      period of time defined by millisecondsToTryToRemoveCorrectableErrors.
	const bool terminate = (
			// We have an error free scan, or...
			keySqr.totalError() == 0
		) || (
			// We have only correctable errors, and we've our budget of time hoping more
			// scanning will remove those errors.
			keySqr.maxError() <= maxCorrectableError &&
			std::chrono::duration_cast<std::chrono::milliseconds>(whenLastRead - whenLastImproved).count() >
				millisecondsToTryToRemoveCorrectableErrors
		);
	// auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(foo - now).count;
	return terminate;
}

// void KeySqrImageReader::augmentImage(

// ) {
// 	augmentedColorImage_BGR_CV_8UC3 = visualizeReadResults(sourceColorImageBGR_CV_8UC3, facesRead, false);
// }

std::string KeySqrImageReader::jsonKeySqr() {
	return keySqr.toJson();
}

bool KeySqrImageReader::isFinished() {
	return terminate;
}