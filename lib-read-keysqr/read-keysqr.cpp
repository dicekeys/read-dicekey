//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#include <float.h>
#include <chrono>
#include <iostream>
#include <math.h>

#include "utilities/bit-operations.h"
#include "graphics/cv.h"
#include "keysqr.hpp"
#include "read-faces.h"
#include "read-keysqr.hpp"
#include "visualize-read-results.h"
#include <opencv2/imgproc/imgproc.hpp>


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
bool DiceKeyImageProcessor::processImage(
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
		angleInRadiansNonCanonicalForm = facesRead.angleInRadiansNonCanonicalForm;
		pixelsPerFaceEdgeWidth = facesRead.pixelsPerFaceEdgeWidth;

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
	terminate = (
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

bool DiceKeyImageProcessor::processRGBAImage (
		int width,
		int height,
		const uint32_t* pointerToRGBAByteArray
) {
	// Create an OpenCV Matrix (Mat) representation of the RGBA data input
  	const cv::Mat colorImage(cv::Size(width, height), CV_8UC4, (void*) pointerToRGBAByteArray, 4 * width);
	// Create a grayscale matrix for the analysis
	cv::Mat grayscale(width, height, CV_8UC1);
	// Convert the RGBA image into a grayscale image
	cv::cvtColor(colorImage, grayscale, cv::COLOR_RGBA2GRAY);
	// Process the grayscale image.
	return processImage(width, height, grayscale.step, grayscale.data);
}



/*
ByteBuffer = buffer = ByteBuffer.allocateDirect( 4 * width * height );
// call my API using the bytebuffer, which can later be replaced with an API that
// writes directly into a canvas
int[] colors = 
// createBitmap(int[] colors, int width, int height, Bitmap.Config config)
Bitmap bitmap = createArray( buffer.asIntBuffer().array(), width, height, Bitmap.Config.ARGB_8888)
// The bitmap can be written into a canvas.
// Later, we can repace the image augmentation code with code that writes directly into the canvas.
*/
/*
 * Note that the rgba array from android will be in big endian format
 */
void DiceKeyImageProcessor::renderAugmentationOverlay(	
		const int width,
		const int height,
		uint32_t* rgbaArrayPtr
) {
	// Make all pixels transparent
	uint32_t* pixelPtr = rgbaArrayPtr; 
	uint32_t* end = pixelPtr + ((size_t) width) * ((size_t) height);
	while (pixelPtr < end) {
		// 100% transparent (black with opacity 0)
		(*pixelPtr++) = 0;
	}
	// Then augment the transparent image with the rendering of DiceKeys
	augmentRGBAImage(width, height, rgbaArrayPtr);
}

void DiceKeyImageProcessor::augmentRGBAImage(	
		const int width,
		const int height,
		uint32_t* rgbaArrayPtr
) {
	// Make all pixels transparent
	if (keySqr.isInitialized() && keySqr.faces.size() == NumberOfFaces) {
		cv::Mat overlayImage_RGBA_CV(cv::Size(width, height), CV_8UC4, rgbaArrayPtr);
		visualizeReadResults(
			overlayImage_RGBA_CV,
			keySqr.faces,
			angleInRadiansNonCanonicalForm,
			pixelsPerFaceEdgeWidth
		);
	}
}


std::string DiceKeyImageProcessor::jsonKeySqrRead() {
	return keySqr.toJson();
}

bool DiceKeyImageProcessor::isFinished() {
	return terminate;
}