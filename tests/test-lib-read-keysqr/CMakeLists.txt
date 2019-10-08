
set(REDUCED_OpenCV_LIBS, "opencv_core;opencv_features2d;opencv_imgcodecs;opencv_imgproc;opencv_shape")

package_add_test(test-keysqr-file-inputs test-keysqr-file-inputs.cpp "lib-read-keysqr,${REDUCED_OpenCV_LIBS}")

target_include_directories(
    ${PROJECT_NAME} 
        PUBLIC
        ${OpenCV_INCLUDE_DIRS}
)