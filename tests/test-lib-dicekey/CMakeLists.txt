package_add_test(test-face test-face.cpp lib-dicekey)

target_include_directories(
    test-face
        PRIVATE
        ${PROJECT_SOURCE_DIR}/lib-dicekey
)

package_add_test(test-dicekey test-dicekey.cpp lib-dicekey)

target_include_directories(
    test-dicekey
        PRIVATE
        ${PROJECT_SOURCE_DIR}/lib-dicekey
)

