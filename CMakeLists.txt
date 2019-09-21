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

##################### Variables ############################
# Change if you want modify path or other values           #
############################################################

# Project
set(PROJECT_NAME read-dicekeys)
project(${PROJECT_NAME}
    VERSION 0.10
    DESCRIPTION "A library for working with DiceKeys and scanning images of them."
    LANGUAGES CXX
)
get_filename_component(PROJECT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE)
message("${PROJECT_NAME}: MAIN PROJECT CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
message("${PROJECT_NAME}: MAIN PROJECT CMAKE_CURRENT_BINARY_DIR: ${CMAKE_CURRENT_BINARY_DIR}")

# Outputs
set(BIN_DIR ${PROJECT_DIR}/bin)

################# CMake Project ############################
# The main options of project                              #
############################################################

project(${PROJECT_NAME} CXX)

# Define Release by default.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
  message(STATUS "Build type not specified: Use Release by default.")
endif(NOT CMAKE_BUILD_TYPE)

####################################################
# Modules
####################################################

##
# lib-dice-key
###

add_subdirectory("lib-dice-key")

file(GLOB LIB_DICE_KEY_SRCS
    "lib-dice-key/*.h"
    "lib-dice-key/*.cpp"
)

add_library(lib-dice-key
    STATIC
    ${LIB_DICE_KEY_SRCS}
)

#add_library(lib-dice-key-shared
#    SHARED
#    ${LIB_DICE_KEY_SRCS}
#)

#############################################################
# Testing
###########
#
# Must come after the libraries being tested
#
#############################################################
option(PACKAGE_TESTS "Build the tests" ON)
if(PACKAGE_TESTS)
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
endif()


#############################################################
# Master project:
#
#  ###############
#   read-dicekeys
#  ###############
#
#############################################################

include_directories(
    ${OpenCV_INCLUDE_DIRS}
    "lib-dice-key"
)

# Gather all the source files
# Source Files
file(GLOB SRC_FILES
    "read-dicekeys/*.cpp"
) 
file(GLOB HEADER_FILES
    "read-dicekeys/*.h"
)

source_group("Sources" FILES ${SRC_FILES})
source_group("Headers" FILES ${HEADERS_FILES})

# Add executable to build.
add_executable(${PROJECT_NAME} 
   ${SRC_FILES} ${HEADERS_FILES}
)

# Use C++ 17
set_target_properties(${PROJECT_NAME}  PROPERTIES
	CXX_STANDARD 17
)

# Ensure OpenCV is installed
find_package( OpenCV REQUIRED )

# Link OpenCV and lib-dice-key so that they're both accessible as part of this add_library
# (public)
target_link_libraries(${PROJECT_NAME}
    PUBLIC
    ${OpenCV_LIBS}
    lib-dice-key
)


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
    target_compile_definitions(${PROJECT_NAME} PRIVATE 
   -D_DEBUG 
   -D_CONSOLE 
    )
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE /W3 /Od /EHsc) # / Zi /MD
		message("*************************************")
    endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(${PROJECT_NAME} PRIVATE 
   -DNDEBUG 
   -D_CONSOLE 
    )
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE /MTd /W3 /GL /Oi /Gy /EHsc) # /Zi
    endif()
endif()

########### Link & Dependencies ############################
# Add project dependencies and Link to project             #
############################################################

if(MSVC)
#   target_link_libraries(${PROJECT_NAME} PRIVATE Ws2_32.lib )
endif(MSVC)

