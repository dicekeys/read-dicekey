file(GLOB LIB_KEYSQR_SRCS
    "*.h"
    "*/*.h"
    "*.cpp"
    "*/*.cpp"
)

add_library(lib-keysqr
    STATIC
    ${LIB_KEYSQR_SRCS}
)

# Use C++ 11
set_target_properties(lib-keysqr PROPERTIES
	CXX_STANDARD 11
)
