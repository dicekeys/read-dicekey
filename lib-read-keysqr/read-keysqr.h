#pragma once

//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#include <string>
#include <vector>
#include <limits>
#include <chrono>

#include "keysqr.hpp"
#include "read-faces.h"

// KeySqr facesReadToKeySqr(
// 	const std::vector<FaceRead> facesRead,
// 	bool reportErrsToStdErr = false
// );

// KeySqr readKeySqr(
// 	const cv::Mat &grayscaleImage,
// 	bool outputErrors = false
// );

std::string readKeySqrJson(
	const cv::Mat &grayscaleImage
);

std::string readKeySqrJson (
	int width,
	int height,
	size_t bytesPerRow,
	void* data
);

static const std::chrono::time_point<std::chrono::system_clock> minTimePoint =
	std::chrono::time_point<std::chrono::system_clock>::min();

/**
 * This structure is used as the second parameter to scanAndAugmentKeySqrImage,
 * and is used both to input the result of the prior call and to return results
 * from the current call.
 **/
class KeySqrImageReader {
private:
	// This value is true if the result was returned from a call to readKeySqr and
	// is false when a default result is constructed by the caller and a pointer is
	// passed to it.
	bool initialized = false;
	float angleInRadiansNonCanonicalForm;
	float pixelsPerFaceEdgeWidth;
	// This value is set the first time scanAndAugmentKeySqrImage is called
	std::chrono::time_point<std::chrono::system_clock> whenFirstRead = minTimePoint;
	// The value is set the first time scanAndAugmentKeySqrImage is called
	// and updated every time a scan reduces the number of errors that
	// have the be resolved before we can return the result.
	std::chrono::time_point<std::chrono::system_clock> whenLastImproved = minTimePoint;
	// The value is set every time scanAndAugmentKeySqrImage is called.
	std::chrono::time_point<std::chrono::system_clock> whenLastRead = minTimePoint;
	// The KeySqr that has been read is stored in this field, which also
	// keeps track of any errors that you have to be resolved during reading.
	KeySqr<FaceRead> keySqr = KeySqr<FaceRead>();
	// After each call, the image passed to scanAndAugmentKeySqrImage
	// is augmented with the scan results and copied here. 
	cv::Mat augmentedColorImage_BGR_CV_8UC3 = cv::Mat();
	// This field is set to true if we've reached the termination condition
	// for the scanning loop.  This is the same value returned as the
	// result of the scanAndAugmentKeySqrImage function.
	bool terminate = false;

public:
	bool processImage(
		int width,
		int height,
		size_t bytesPerRow,
		void* pointerToByteArray
	);

	void renderAugmentationOverlay(	
		int width,
		int height,
		uint32_t* rgbaArrayPtr
	);

	KeySqr<FaceRead> keySqrRead() { return keySqr; }

	std::string jsonKeySqrRead();

	bool isFinished();

};



// https://developer.android.com/reference/android/graphics/ImageFormat.html#YUV_420_888
