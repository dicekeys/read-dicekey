package_add_test(test-face test-face.cpp lib-keysqr)

target_include_directories(
    test-face
        PRIVATE
        ${PROJECT_SOURCE_DIR}/lib-keysqr
)

package_add_test(test-keysqr test-keysqr.cpp lib-keysqr)

target_include_directories(
    test-keysqr
        PRIVATE
        ${PROJECT_SOURCE_DIR}/lib-keysqr
)

package_add_test(test-key-generation-options test-key-generation-options.cpp lib-keysqr)

target_include_directories(
    test-key-generation-options
        PRIVATE
        ${PROJECT_SOURCE_DIR}/lib-keysqr
        ${libsodium_SOURCE_DIR}/libsodium/src/libsodium/include
)

