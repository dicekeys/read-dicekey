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

#  -DOPENCV_EXTRA_MODULES_PATH="$myRepo"/opencv_contrib/modules
#  -DCMAKE_INSTALL_PREFIX="$myRepo"/install/"$RepoSource" "$myRepo/$RepoSource"
#        -DCMAKE_TOOLCHAIN_FILE=<path to the OpenCV source directory>/platforms/linux/arm-gnueabi.toolchain.cmake

# See https://docs.opencv.org/4.1.1/d0/d76/tutorial_arm_crosscompile_with_cmake.html

#add_custom_command(TARGET ${PROJECT_NAME}
#    PRE_BUILD
#    COMMAND cmake \
#        -G"$CMAKE_CONFIG_GENERATOR" \
#        CMAKE_OPTIONS='-DBUILD_PERF_TESTS:BOOL=OFF \
#        -DBUILD_TESTS:BOOL=OFF \
#        -DBUILD_DOCS:BOOL=OFF \
#        -DWITH_CUDA:BOOL=OFF \
#        -DBUILD_EXAMPLES:BOOL=OFF \
#        -DINSTALL_CREATE_DISTRIB=ON' \
#        "${PROJECT_NAME}/extern/opencv/build/"
#)

target_include_directories(
    ${PROJECT_NAME} 
        PUBLIC
        "../lib-keysqr"
        ${OpenCV_INCLUDE_DIRS}
)


set(REDUCED_OpenCV_LIBS, "opencv_core;opencv_features2d;opencv_imgproc")

message("All OpenCV Lib locations: ${OpenCV_LIBS}")
message("Reduced OpenCV libs: ${REDUCED_OpenCV_LIBS}")
# Link OpenCV and lib-keysqr so that they're both accessible as part of this add_library
# (public)
target_link_libraries(${PROJECT_NAME}
    PUBLIC
    ${REDUCED_OpenCV_LIBS}
    lib-keysqr
)


# Use C++ 11
set_target_properties(${PROJECT_NAME}  PROPERTIES
	CXX_STANDARD 11
)
