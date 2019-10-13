
package_add_test(
    test-keysqr-file-inputs
    test-keysqr-file-inputs.cpp
    "lib-keysqr,lib-read-keysqr,${OPENCV_LIB_DIR}/libopencv_core,${OPENCV_LIB_DIR}/libopencv_imgproc,${OPENCV_LIB_DIR}/libopencv_imgcodecs"
)

target_include_directories(
    test-keysqr-file-inputs
        PUBLIC
        ${OpenCV_INCLUDE_DIRS}
        ../../lib-read-keysqr
)