package_add_test(test-face test-face.cpp lib-keysqr)

target_include_directories(
    test-face
        PUBLIC
        ../../lib-keysqr
)

package_add_test(test-keysqr test-keysqr.cpp lib-keysqr)

target_include_directories(
    test-keysqr
        PUBLIC
        ../../lib-keysqr
)

