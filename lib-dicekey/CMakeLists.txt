file(GLOB LIB_DICEKEY_SRCS
    "*.h"
    "*/*.h"
    "*.cpp"
    "*/*.cpp"
)

# This if statement is a hack until Stuart figures out
# how to get builds with shared libs working on windows
if(CMAKE_SYSTEM_NAME STREQUAL "Windows" OR CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    add_library(lib-dicekey
        STATIC
        ${LIB_DICEKEY_SRCS}
    )
else()
    add_library(lib-dicekey
        SHARED
        ${LIB_DICEKEY_SRCS}
    )
endif()

# Use C++ 11
set_target_properties(lib-dicekey PROPERTIES
	CXX_STANDARD 11
)
