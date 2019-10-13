
package_add_test(
    test-keysqr-file-inputs
    test-keysqr-file-inputs.cpp
)

target_link_libraries(test-keysqr-file-inputs
    PUBLIC
    lib-read-keysqr
    ${OPENCV_LIB_DIR}/libopencv_core      #.so
    ${OPENCV_LIB_DIR}/libopencv_imgcodecs #.so
    ${OPENCV_LIB_DIR}/libopencv_imgproc   #.so
    lib-keysqr
)

target_include_directories(
    test-keysqr-file-inputs
        PUBLIC
        ${OpenCV_INCLUDE_DIRS}
        ${KEYSQR_LIBRARIES_PROJECT_DIR}/lib-keysqr
        ${KEYSQR_LIBRARIES_PROJECT_DIR}/lib-read-keysqr
)