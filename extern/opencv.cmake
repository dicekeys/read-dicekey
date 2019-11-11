include(FetchContent)

set (OPENCV_VERSION "4.1.2" CACHE STRING "OpenCV Version to Use")
message("CMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}")
if(CMAKE_SYSTEM_NAME STREQUAL "Android")
    FetchContent_Declare(
        opencv
        URL https://github.com/opencv/opencv/releases/download/4.1.2/opencv-${OPENCV_VERSION}-android-sdk.zip
    )
	FetchContent_MakeAvailable(opencv)
    set (OpenCV_DIR "${opencv_SOURCE_DIR}/sdk/native/jni")
	find_package(OpenCV ${OPENCV_VERSION} REQUIRED COMPONENTS core imgproc imgcodecs)

else()
#    message("Candidate SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}/extern/opencv")
#    message("Candidate BINARY_DIR: ${CMAKE_CURRENT_BINARY_DIR}/opencv")
    FetchContent_Declare(
        opencv
        URL https://github.com/opencv/opencv/archive/${OPENCV_VERSION}.zip
#        GIT_REPOSITORY https://github.com/opencv/opencv.git
#        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/opencv
#        BINARY_DIR: ${CMAKE_CURRENT_BINARY_DIR}/opencv
    )
    set(BUILD_LIST "core,imgproc,imgcodecs" CACHE STRING "")
	FetchContent_MakeAvailable(opencv)
#    install(opencv)
#	find_package(OpenCV ${OPENCV_VERSION} REQUIRED COMPONENTS core imgproc imgcodecs)

    # set(OpenCV_LIBS "opencv_core;opencv-imgproc;opencv-imgcodecs")
	#    set (OpenCV_DIR "${CMAKE_BINARY_DIR}")
	message("OpenCV_DIR = ${OpenCV_DIR}")
	message("OpenCV_SOURCE_DIR = ${OpenCV_SOURCE_DIR}")
	message("OpenCV_BINARY_DIR = ${OpenCV_BINARY_DIR}")
	message("OpenCV_INCLUDE_DIR = ${OpenCV_INCLUDE_DIR}")
	message("OpenCV_LIBS = ${OpenCV_LIBS}")

    add_library(opencv SHARED IMPORTED)
    # set_target_properties(opencv PROPERTIES
    #  INTERFACE_INCLUDE_DIRECTORIES "${OpenCV_INCLUDE_DIRS}"
    #  IMPORTED_LOCATION "${OpenCV_LIBS}")

endif()

# add_library(opencv SHARED IMPORTED)
#set_target_properties(opencv PROPERTIES
#  INTERFACE_INCLUDE_DIRECTORIES "${OpenCV_INCLUDE_DIRS}"
#  IMPORTED_LOCATION "${OpenCV_LIBS}")