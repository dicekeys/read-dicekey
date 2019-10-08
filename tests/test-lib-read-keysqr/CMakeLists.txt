package_add_test(test-keysqr-file-inputs test-keysqr-file-inputs.cpp "lib-read-keysqr")

target_link_libraries(test-keysqr-file-inputs
    PUBLIC
    ${OpenCV_LIBS}
)
target_include_directories(
    ${PROJECT_NAME} 
        PUBLIC
        ${OpenCV_INCLUDE_DIRS}
)