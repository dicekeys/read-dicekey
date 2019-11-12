file(GLOB LIB_KEYSQR_SRCS
    "*.h"
    "*/*.h"
    "*.cpp"
    "*/*.cpp"
)

add_library(lib-keysqr
# must be turned on for android, but currently breaks testing on windows
    SHARED
#    STATIC
    ${LIB_KEYSQR_SRCS}
)

target_link_libraries(lib-keysqr
    PRIVATE
        sodium
)

# Use C++ 11
set_target_properties(lib-keysqr PROPERTIES
	CXX_STANDARD 11
)
