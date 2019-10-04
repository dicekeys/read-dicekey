#include "read-keysqr.h"
// #include "jni.h"

// JNIEXPORT jstring JNICALL readKeySqr(
//     JNIEnv* env,
//     jobject obj,
//     jint width,
//     jint height,
//     jint bytesPerRow,
//     jbyteArray jByteArrayForGrayscaleChannel)
// {
//     jbyte* pointerToByteArrayForGrayscaleChannel= env->GetByteArrayElements(jByteArrayForGrayscaleChannel, 0);
//     const std::string jsonResult = readKeySqrJson(width, height, pointerToByteArrayForGrayscaleChannel, bytesPerRow)
//     env->ReleaseByteArrayElements(p_data, pointerToByteArrayForGrayscaleChannel, 0);
//     return jsonResult;
// }