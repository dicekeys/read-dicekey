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
        "../api-jni/*.cpp"
)

#source_group("Sources" FILES ${SRC_FILES})
#source_group("Headers" FILES ${HEADERS_FILES})

add_library(${KEYSQR_LIBRARIES_PROJECT_NAME}
    SHARED
   ${ALL_SRC_FILES})

#  -DOPENCV_EXTRA_MODULES_PATH="$myRepo"/opencv_contrib/modules
#  -DCMAKE_INSTALL_PREFIX="$myRepo"/install/"$RepoSource" "$myRepo/$RepoSource"
#        -DCMAKE_TOOLCHAIN_FILE=<path to the OpenCV source directory>/platforms/linux/arm-gnueabi.toolchain.cmake

# See https://docs.opencv.org/4.1.1/d0/d76/tutorial_arm_crosscompile_with_cmake.html

message("${KEYSQR_LIBRARIES_PROJECT_NAME}: Target OpenCV include dirs: ${OpenCV_INCLUDE_DIRS}")
target_include_directories(
    ${KEYSQR_LIBRARIES_PROJECT_NAME} 
        PUBLIC
        ${PROJECT_SOURCE_DIR}/lib-keysqr
        ${OpenCV_INCLUDE_DIRS}
)



# Link OpenCV and lib-keysqr so that they're both accessible as part of this add_library
# (public)

target_link_libraries(${KEYSQR_LIBRARIES_PROJECT_NAME}
    PRIVATE 
    ${OpenCV_LIBS}
    lib-keysqr
)


# Use C++ 11
set_target_properties(${KEYSQR_LIBRARIES_PROJECT_NAME}  PROPERTIES
	CXX_STANDARD 11
)
