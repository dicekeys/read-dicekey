# Gather all the source files
# Source Files
file(GLOB SRC_FILES
    "*.cpp"
    "*/*.cpp"
) 
file(GLOB HEADER_FILES
    "*.h"
    "*/*.h"
)
file(GLOB ALL_SRC_FILES
    "*.h"
    "*/*.h"
    "*.cpp"
    "*/*.cpp"
)

#source_group("Sources" FILES ${SRC_FILES})
#source_group("Headers" FILES ${HEADERS_FILES})

add_library(${PROJECT_NAME}
    STATIC
   ${ALL_SRC_FILES})

target_include_directories(
    ${PROJECT_NAME} 
        PUBLIC
        "../lib-keysqr"
        # Removed ${OpenCV_INCLUDE_DIRS} 
        ${PROJECT_DIR}/extern/opencv/modules/core/include
        ${PROJECT_DIR}/extern/opencv/modules/imgproc/include
        ${PROJECT_DIR}/extern/opencv/modules/imgcodecs/include
        ${CMAKE_BINARY_DIR}
)

message("OpenCV Lib locations: ${CMAKE_BINARY_DIR}/opencv_cored.pdb, "
    "${CMAKE_BINARY_DIR}/opencv_imgproc.pdb"
)
# Link OpenCV and lib-keysqr so that they're both accessible as part of this add_library
# (public)
target_link_libraries(${PROJECT_NAME}
    PUBLIC
    "${CMAKE_BINARY_DIR}/opencv_cored.pdb"
    "${CMAKE_BINARY_DIR}/opencv_imgproc.pdb"
    lib-keysqr
)


# Use C++ 11
set_target_properties(${PROJECT_NAME}  PROPERTIES
	CXX_STANDARD 11
)
