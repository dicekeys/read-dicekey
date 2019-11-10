include(FetchContent)

FetchContent_Declare(
    libsodium
    GIT_REPOSITORY https://github.com/robinlinden/libsodium-cmake.git
)

set(SODIUM_DISABLE_TESTS ON CACHE INTERNAL "")
FetchContent_MakeAvailable(libsodium)
