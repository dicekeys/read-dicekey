#include "read-keysqr.h"
// #include "jni.h"

// JNIEXPORT jstring JNICALL readKeySqr(
//     JNIEnv* env,
//     jobject obj,
//     jint width,
//     jint height,
//     jint bytesPerRow,
//     jobject byteBufferForGrayscaleChannel)
// {
// 	const std::string jsonResult;
//  jbyte pointerToByteArrayForGrayscaleChannel = env->GetDirectBufferAddress(env, byteBufferForGrayscaleChannel);
// 	if (pointerToByteArrayForGrayscaleChannel != null) {
// 		try {
// 			jsonResult = readKeySqrJson(width, height, pointerToByteArrayForGrayscaleChannel, bytesPerRow)
// 		} catch (...) {
// 			jsonResult = "null";
// 		}
// 	}
// 	return jsonResult;
// }

// read-keysqr
// base c++ project
//   dependent on OpenCV being available
//   should be entirely platform independent
//   if absolutely necessary, can use ifdefs for ANDROID, IOS, etc.

// keysqr-jvm
//   exposes android api using JNI
//     imports c++ directly
//   importable either by
//     `git submodule` or
//     `jitpack`

// keysqr-ios
//   exposes ios api

// react-native-camera
//   existing project we are forking
//   imports keysqr-android for android side, keysqr-ios for ios side

