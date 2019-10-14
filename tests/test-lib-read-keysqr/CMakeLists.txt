
package_add_test(
    test-keysqr-file-inputs
    test-keysqr-file-inputs.cpp
    "lib-keysqr,lib-read-keysqr,${OPENCV_LIB_DIR}/libopencv_core,${OPENCV_LIB_DIR}/libopencv_imgproc,${OPENCV_LIB_DIR}/libopencv_imgcodecs"
)

message("Test include directories sourced from project source dir ${PROJECT_SOURCE_DIR}")
target_include_directories(
    test-keysqr-file-inputs
        PRIVATE
        ${OPENCV_INCLUDE_DIR}
        ${PROJECT_SOURCE_DIR}/lib-keysqr
        ${PROJECT_SOURCE_DIR}/lib-read-keysqr
)