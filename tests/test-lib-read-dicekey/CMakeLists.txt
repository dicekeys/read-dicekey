message("Tests for file inputs")

package_add_test(
    test-dicekey-file-inputs
    test-dicekey-file-inputs.cpp
    "lib-dicekey;lib-read-dicekey"
)

target_link_libraries(
    test-dicekey-file-inputs    
    PRIVATE
    ${OpenCV_LIBS}
)

message("Test include directories sourced from project source dir ${OpenCV_INCLUDE_DIRS}")
target_include_directories(
    test-dicekey-file-inputs
        PRIVATE
#        ${OpenCV_LIBS}
        ${OpenCV_INCLUDE_DIRS}
        ${PROJECT_SOURCE_DIR}/lib-dicekey
        ${PROJECT_SOURCE_DIR}/lib-read-dicekey
)