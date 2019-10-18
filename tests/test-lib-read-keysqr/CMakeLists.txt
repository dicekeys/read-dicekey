message("Tests for file inputs")
package_add_test(
    test-keysqr-file-inputs
    test-keysqr-file-inputs.cpp
    "lib-keysqr;lib-read-keysqr"
)


target_link_libraries(
    test-keysqr-file-inputs    
    PRIVATE
    ${OPENCV_LIBS}
)

message("Test include directories sourced from project source dir ${PROJECT_SOURCE_DIR}")
target_include_directories(
    test-keysqr-file-inputs
        PRIVATE
        ${OPENCV_LIBS}
        ${PROJECT_SOURCE_DIR}/lib-keysqr
        ${PROJECT_SOURCE_DIR}/lib-read-keysqr
)