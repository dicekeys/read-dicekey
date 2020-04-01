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

package_add_test(test-key-derivation-options test-key-derivation-options.cpp lib-seeded)

target_include_directories(
    test-key-derivation-options
        PRIVATE
        ${PROJECT_SOURCE_DIR}/lib-seeded
        ${libsodium_SOURCE_DIR}/libsodium/src/libsodium/include
)

package_add_test(test-crypto test-crypto.cpp lib-seeded)

target_include_directories(
    test-crypto
        PRIVATE
        ${PROJECT_SOURCE_DIR}/lib-seeded
        ${libsodium_SOURCE_DIR}/libsodium/src/libsodium/include
)

