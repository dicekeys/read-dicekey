file(GLOB LIB_DICE_KEY_SRCS
    "*.h"
    "*/*.h"
    "*.cpp"
    "*/*.cpp"
)

add_library(lib-keysqr
    STATIC
    ${LIB_DICE_KEY_SRCS}
)

# Use C++ 17
set_target_properties(lib-keysqr PROPERTIES
	CXX_STANDARD 17
)
