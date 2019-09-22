#######################################################################
# IMPORTANT SHORTCOMINGS OF THIS CMAKE CONFIG FOR VISUAL STUDIO USERS
#
# CMake fails to configure the working directory for running
# tests in Visual Studio using Google Test.
# Before running, got to Visual Studio's debug menu,
# choose the "options" item, and then go to the
# "TestAdapterForGoogleTest" tab.
# Set "Working Directory" to "$(SolutionDir)"
# (note, those are parens around SolutionDir, and not braces
#  as one might become accustomed to seeing in CMake).
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

##################### Variables ############################
# Change if you want modify path or other values           #
############################################################

# Project
set(PROJECT_NAME lib-read-dicekey)
project(${PROJECT_NAME}
    VERSION 0.10
    DESCRIPTION "Libraries for working with DiceKeys and scanning images of them."
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
# Libraries to build
####################################################
add_subdirectory("lib-dicekey")
add_subdirectory("lib-read-dicekey")


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


# Ensure OpenCV is installed
find_package( OpenCV REQUIRED )

# Link OpenCV and lib-dicekey so that they're both accessible as part of this add_library
# (public)
target_link_libraries(${PROJECT_NAME}
    PUBLIC
    ${OpenCV_LIBS}
    lib-dicekey
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
