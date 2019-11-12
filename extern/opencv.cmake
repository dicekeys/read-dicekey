include(FetchContent)

set (OPENCV_VERSION "4.1.2" CACHE STRING "OpenCV Version to Use")
message("CMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}")

message("Downloading OpenCV. This may take many minutes and could TIMEOUT cmake")

if(CMAKE_SYSTEM_NAME STREQUAL "Android")

    FetchContent_Declare(
        opencv
        URL https://github.com/opencv/opencv/releases/download/${OPENCV_VERSION}/opencv-${OPENCV_VERSION}-android-sdk.zip
        UPDATE_DISCONNECTED # since we're using a fixed version, only download once
    )
	FetchContent_MakeAvailable(opencv)
    # To assist find_package in finding OpenCV, set the OpenCV_DIR based on
    # https://github.com/opencv/opencv/blob/65573784c488f6548770381e2786fbf15e0f1e32/cmake/OpenCVInstallLayout.cmake#L12
    set (OpenCV_DIR "${opencv_SOURCE_DIR}/sdk/native/jni")

elseif(CMAKE_SYSTEM_NAME STREQUAL "iOS")

    FetchContent_Declare(
        opencv
        URL https://github.com/opencv/opencv/releases/download/${OPENCV_VERSION}/opencv-${OPENCV_VERSION}-ios-framework.zip
        UPDATE_DISCONNECTED # since we're using a fixed version, only download once
    )
	FetchContent_MakeAvailable(opencv)

    # To assist find_package in finding OpenCV, set the OpenCV_DIR based on
    # https://github.com/opencv/opencv/blob/65573784c488f6548770381e2786fbf15e0f1e32/cmake/OpenCVInstallLayout.cmake#L68
    set (OpenCV_DIR "${opencv_SOURCE_DIR}/cmake/opencv4")

elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")

    FetchContent_Declare(
        opencv
        URL https://github.com/opencv/opencv/releases/download/${OPENCV_VERSION}/opencv-${OPENCV_VERSION}-vc14_vc15.exe
        UPDATE_DISCONNECTED # since we're using a fixed version, only download once
    )
	FetchContent_MakeAvailable(opencv)
    # To assist find_package in finding OpenCV, set the OpenCV_DIR based on
    # https://github.com/opencv/opencv/blob/65573784c488f6548770381e2786fbf15e0f1e32/cmake/OpenCVInstallLayout.cmake#L12
    set (OpenCV_DIR "${opencv_SOURCE_DIR}/build")
#    set (opencv_BINARY_DIR "${}")
    message("OpenCV_DIR ${OpenCV_DIR}")

else()
    FetchContent_Declare(
        # Downloading source doesn't give you working headers until make install
        # because OpenCV is designed to torture anyone who might try to incorporate
        # its source into a cmake project.
        opencv
        GIT_REPOSITORY https://github.com/opencv/opencv.git
        GIT_TAG        ${OPENCV_VERSION}
        UPDATE_DISCONNECTED # since we're using a fixed version, only download once
    )
    # To assist find_package in finding OpenCV, set the OpenCV_DIR based on
    # https://github.com/opencv/opencv/blob/65573784c488f6548770381e2786fbf15e0f1e32/cmake/OpenCVInstallLayout.cmake#L68
    set (OpenCV_DIR "${opencv_SOURCE_DIR}/cmake/opencv4")

	FetchContent_MakeAvailable(opencv)
endif()
message("OpenCV downloaded.")

find_package(OpenCV ${OPENCV_VERSION} REQUIRED COMPONENTS core imgproc imgcodecs)
add_library(opencv SHARED IMPORTED)

message("OpenCV_DIR = ${OpenCV_DIR}")
message("OpenCV_SOURCE_DIR = ${OpenCV_SOURCE_DIR}")
message("OpenCV_BINARY_DIR = ${OpenCV_BINARY_DIR}")
message("OpenCV_INCLUDE_DIR = ${OpenCV_INCLUDE_DIR}")
message("opencv_SOURCE_DIR = ${opencv_SOURCE_DIR}")
message("opencv_BINARY_DIR = ${opencv_BINARY_DIR}")
message("opencv_INCLUDE_DIR = ${opencv_INCLUDE_DIR}")
message("OpenCV_LIBS = ${OpenCV_LIBS}")
