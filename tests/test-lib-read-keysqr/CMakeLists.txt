package_add_test(test-keysqr-file-inputs test-keysqr-file-inputs.cpp "lib-read-keysqr")

target_link_libraries(test-keysqr-file-inputs
    PUBLIC
    "${CMAKE_BINARY_DIR}/opencv_cored.pdb"
    "${CMAKE_BINARY_DIR}/opencv_imgproc.pdb"
    "${CMAKE_BINARY_DIR}/opencv_imgcodecs.dll"
)
