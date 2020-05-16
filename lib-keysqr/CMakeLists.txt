file(GLOB LIB_KEYSQR_SRCS
    "*.h"
    "*/*.h"
    "*.cpp"
    "*/*.cpp"
)

# This if statement is a hack until Stuart figures out
# how to get builds with shared libs working on windows
if(CMAKE_SYSTEM_NAME STREQUAL "Windows" OR CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    add_library(lib-keysqr
        STATIC
        ${LIB_KEYSQR_SRCS}
    )
else()
    add_library(lib-keysqr
        SHARED
        ${LIB_KEYSQR_SRCS}
    )
endif()

# Use C++ 11
set_target_properties(lib-keysqr PROPERTIES
	CXX_STANDARD 11
)
