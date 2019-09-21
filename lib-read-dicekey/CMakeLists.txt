# Gather all the source files
# Source Files
file(GLOB SRC_FILES
    "*.cpp"
) 
file(GLOB HEADER_FILES
    "*.h"
)

source_group("Sources" FILES ${SRC_FILES})
source_group("Headers" FILES ${HEADERS_FILES})

add_library(${PROJECT_NAME}
    STATIC
   ${SRC_FILES} ${HEADERS_FILES})

target_include_directories(
    ${PROJECT_NAME} 
        PUBLIC
        "../lib-dicekey"
        ${OpenCV_INCLUDE_DIRS}
)

# Use C++ 17
set_target_properties(${PROJECT_NAME}  PROPERTIES
	CXX_STANDARD 17
)
