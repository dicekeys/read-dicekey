#######################################################################
# IMPORTANT SHORTCOMINGS OF THIS CMAKE CONFIG FOR VISUAL STUDIO USERS
#
# Visual Studio unfortunatly defaults to overriding the working directory
# for Google Test set by CMAKE.
# To fix this
#    go to Visual Studio's debug menu or tool menu,
#    choose the "options" item, and then
#    go to the "Test Adapter For GoogleTest" tab.
#    Clear the "Working Directory" field to an empty string.
#
#######################################################################
message("
*********************************************************************
 IMPORTANT SHORTCOMINGS OF THIS CMAKE CONFIG FOR VISUAL STUDIO USERS
*********************************************************************

CMake fails to configure the working directory for running
tests in Visual Studio using Google Test.
Before running, got to Visual Studio's debug menu,
choose the 'options' item, and then go to the

'TestAdapterForGoogleTest' tab.
Set 'Working Directory' to '$(SolutionDir)'
(note, those are parens around SolutionDir, and not braces
 as one might become accustomed to seeing in CMake).

 *********************************************************************
")


################## CMAKE Version ###########################
# Make sure we're using CMAKE 3.1 or higher, ideally the latest
# From https://cliutils.gitlab.io/modern-cmake/chapters/basics.html
############################################################
cmake_minimum_required(VERSION 3.1)

if(${CMAKE_VERSION} VERSION_LESS 3.15)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
else()
    cmake_policy(VERSION 3.15)
endif()


################# CMake Project ############################
# The main options of project                              #
############################################################
set(KEYSQR_LIBRARIES_PROJECT_NAME lib-read-keysqr)
project(${KEYSQR_LIBRARIES_PROJECT_NAME}
    VERSION 0.10
    DESCRIPTION "Libraries for working with KeySqrs and scanning images of them."
    LANGUAGES CXX
)
# Define Release by default.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
  message(STATUS "Build type not specified: Use Release by default.")
endif(NOT CMAKE_BUILD_TYPE)
get_filename_component(KEYSQR_LIBRARIES_PROJECT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE)

# set(ANDROID_ABI "x86_64" CACHE STRING "Default architecture to x86_64")

# Paths
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#set(CMAKE_PROJECT_BINARY_DIR "${KEYSQR_LIBRARIES_PROJECT_DIR}/build/bin" CACHE PATH FORCE)
#set(CMAKE_BINARY_DIR "${KEYSQR_LIBRARIES_PROJECT_DIR}/build/bin" CACHE PATH FORCE)
#set(CMAKE_LIBRARY_DIR "${KEYSQR_LIBRARIES_PROJECT_DIR}/build/lib" CACHE PATH FORCE)
#set(CMAKE_CURRENT_BINARY_DIR "${KEYSQR_LIBRARIES_PROJECT_DIR}/build/bin" CACHE PATH FORCE)
#set(CMAKE_CURRENT_LIBRARY_DIR "${KEYSQR_LIBRARIES_PROJECT_DIR}/build/lib" CACHE PATH FORCE)
#set(BIN_DIR ${KEYSQR_LIBRARIES_PROJECT_DIR}/bin)
#set(LIB_DIR ${KEYSQR_LIBRARIES_PROJECT_DIR}/lib)

message("${KEYSQR_LIBRARIES_PROJECT_NAME}: MAIN PROJECT CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message("${KEYSQR_LIBRARIES_PROJECT_NAME}: MAIN PROJECT CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
message("${KEYSQR_LIBRARIES_PROJECT_NAME}: MAIN PROJECT CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}")
message("${KEYSQR_LIBRARIES_PROJECT_NAME}: MAIN PROJECT CMAKE_CURRENT_BINARY_DIR: ${CMAKE_CURRENT_BINARY_DIR}")


####################################################
# External libraries to build
#
####################################################
# Ensure OpenCV is installed

# OpenCV

if (NOT ANDROID_ABI)
    find_package( OpenCV REQUIRED )
    set (OPENCV_INCLUDE_DIR ${OpenCV_INCLUDE_DIRS})
    set (OPENCV_LIBS "opencv_core;opencv_imgproc;opencv_imgcodecs")
else()
    message("Configuring for Android ${ANDROID_ABI}")
    set (OPENCV_INCLUDE_DIR "${KEYSQR_LIBRARIES_PROJECT_DIR}/builds/include")
    set (OPENCV_LIB_DIR "${KEYSQR_LIBRARIES_PROJECT_DIR}/builds/opencv/sdk/native/libs/${ANDROID_ABI}")
    set (OPENCV_LIBS "${OPENCV_LIB_DIR}/libopencv_core.so;${OPENCV_LIB_DIR}/libopencv_imgproc.so;${OPENCV_LIB_DIR}/libopencv_imgcodecs.so")

    set(BUILD_LIST "core,imgcodecs,imgproc" CACHE STRING "Build list for OpenCV")
    # Turn off features we don't need ...
    set(BUILD_EXAMPLES OFF CACHE BOOL "")
    set(BUILD_PERF_TESTS OFF CACHE BOOL "")
    set(BUILD_TESTS OFF CACHE BOOL "")
    set(BUILD_DOCS OFF CACHE BOOL "")
    set(BUILD_PYTHON OFF CACHE BOOL "")
    set(BUILD_ANDROID_PROJECTS OFF CACHE BOOL "")
    set(BUILD_ANDROID_EXAMPLES OFF CACHE BOOL "")

    set(WITH_TIFF OFF CACHE BOOL "")

    set(WITH_OPENCL OFF CACHE BOOL "")
    set(WITH_OPENMP OFF CACHE BOOL "")
    set(WITH_TBB OFF CACHE BOOL "")
    set(WITH_MKL OFF CACHE BOOL "")
    set(MKL_WITH_TBB OFF CACHE BOOL "")
    set(MKL_WITH_OPENMP OFF CACHE BOOL "")

#    set(OPENCV_SRC_DIR "${KEYSQR_LIBRARIES_PROJECT_DIR}/extern/opencv")
#    set(OPENCV_BIN_DIR "${KEYSQR_LIBRARIES_PROJECT_DIR}/extern/opencv/build")
#    message("OpenCV SrcDir: ${OPENCV_SRC_DIR} BinDIr: ${OPENCV_BIN_DIR}")
#    add_subdirectory("${KEYSQR_LIBRARIES_PROJECT_DIR}/extern/opencv")
    add_subdirectory(extern/opencv)
endif()
message("OPENCV_INCLUDE_DIR ${OPENCV_INCLUDE_DIR}")
message("OPENCV_LIBS ${OPENCV_LIBS}")


####################################################
# Libraries to build
####################################################
add_subdirectory("lib-keysqr")
add_subdirectory("lib-read-keysqr")

#############################################################
# Testing
###########
#
# Must come after the libraries being tested
#
#############################################################

option(KEYSQR_CREATE_AND_RUN_TESTS "Create and run tests" False)
message("KEYSQR_CREATE_AND_RUN_TESTS=${KEYSQR_CREATE_AND_RUN_TESTS}")
if ("${KEYSQR_CREATE_AND_RUN_TESTS}" STREQUAL "True")
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
endif()


######################### Flags ############################
# Defines Flags for Windows and Linux                      #
############################################################

if(NOT MSVC)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
   if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
       set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
   endif()
endif(NOT MSVC)

# Preprocessor definitions
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${KEYSQR_LIBRARIES_PROJECT_NAME} PRIVATE 
   -D_DEBUG 
   -D_CONSOLE
    )
    if(MSVC)
        target_compile_options(${KEYSQR_LIBRARIES_PROJECT_NAME} PRIVATE /W3 /Od /EHsc) # / Zi /MD
		message("*************************************")
    endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(${KEYSQR_LIBRARIES_PROJECT_NAME} PRIVATE 
   -DNDEBUG 
   -D_CONSOLE 
    )
    if(MSVC)
        target_compile_options(${KEYSQR_LIBRARIES_PROJECT_NAME} PRIVATE /MTd /W3 /GL /Oi /Gy /EHsc) # /Zi
    endif()
endif()

########### Link & Dependencies ############################
# Add project dependencies and Link to project             #
############################################################

if(MSVC)
#   target_link_libraries(${KEYSQR_LIBRARIES_PROJECT_NAME} PRIVATE Ws2_32.lib )
endif(MSVC)
