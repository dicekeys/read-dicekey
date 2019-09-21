file(GLOB LIB_DICE_KEY_SRCS
    "*.h"
    "*.cpp"
)

add_library(lib-dicekey
    STATIC
    ${LIB_DICE_KEY_SRCS}
)

# Use C++ 17
set_target_properties(lib-dicekey PROPERTIES
	CXX_STANDARD 17
)
