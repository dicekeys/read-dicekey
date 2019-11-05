#include <string>
#include <jni.h>
#include <graphics/cv.h>

#include "read-keysqr.h"

extern "C" {

JNIEXPORT jstring JNICALL readKeySqr(
     JNIEnv* env,
     jobject obj,
     jint width,
     jint height,
     jint bytesPerRow,
     jobject byteBufferForGrayscaleChannel
 ) {
 	std::string jsonResult;
 	void *pointerToByteArrayForGrayscaleChannel = env->GetDirectBufferAddress(byteBufferForGrayscaleChannel);
 	if (pointerToByteArrayForGrayscaleChannel != NULL) {
 		try {
 			jsonResult = readKeySqrJson((int) width, (int) height, (size_t) bytesPerRow, (void*) pointerToByteArrayForGrayscaleChannel);
		} catch (...) {
 			jsonResult = "null";
 		}
 	}
 	return env->NewStringUTF(jsonResult.c_str());
}

JNIEXPORT jstring JNICALL Java_com_keysqr_readkeysqr_ReadKeySqr_helloFromOpenCV(JNIEnv *env, jobject thiz) {
    std::string name = "OPENCV Version ";
    name += CV_VERSION;

    return env->NewStringUTF(name.c_str());
}

}