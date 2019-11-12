include(FetchContent)

set (OPENCV_VERSION "4.1.2" CACHE STRING "OpenCV Version to Use")
message("CMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}")
if(CMAKE_SYSTEM_NAME STREQUAL "Android")
    FetchContent_Declare(
        opencv
        URL https://github.com/opencv/opencv/releases/download/${OPENCV_VERSION}/opencv-${OPENCV_VERSION}-android-sdk.zip
        UPDATE_DISCONNECTED # since we're using a fixed version, only download once
    )
	FetchContent_MakeAvailable(opencv)
    set (OpenCV_DIR "${opencv_SOURCE_DIR}/sdk/native/jni")
	find_package(OpenCV ${OPENCV_VERSION} REQUIRED COMPONENTS core imgproc imgcodecs)

elsif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message("Downloading OpenCV for windows(this may take many minutes and possibly timeout!)")
    FetchContent_Declare(
        opencv
#       We had previously downloaded the source git, but
#       that doesn't have working headers because OpenCV
#       sucks 
#        GIT_REPOSITORY https://github.com/opencv/opencv.git
        URL https://github.com/opencv/opencv/releases/download/${OPENCV_VERSION}/opencv-${OPENCV_VERSION}-vc14_vc15.exe
        UPDATE_DISCONNECTED # since we're using a fixed version, only download once
    )
	FetchContent_MakeAvailable(opencv)
    message("Finished downloading opencv")
    set (OpenCV_DIR "${opencv_SOURCE_DIR}/build")
    message("OpenCV_DIR ${OpenCV_DIR}")

	find_package(OpenCV 4.1.2 REQUIRED COMPONENTS core imgproc imgcodecs)

	message("OpenCV_DIR = ${OpenCV_DIR}")
	message("OpenCV_SOURCE_DIR = ${OpenCV_SOURCE_DIR}")
	message("OpenCV_BINARY_DIR = ${OpenCV_BINARY_DIR}")
	message("OpenCV_INCLUDE_DIR = ${OpenCV_INCLUDE_DIR}")
	message("OpenCV_LIBS = ${OpenCV_LIBS}")
    add_library(opencv SHARED IMPORTED)

else()
    ## FIXME -- need a solution that works on mac & linux
endif()
