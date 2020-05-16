include(FetchContent)

set (OPENCV_VERSION "4.3.0" CACHE STRING "OpenCV Version to Use")
message("CMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}")

message("Downloading OpenCV. This may take many minutes and could TIMEOUT cmake")

if(CMAKE_SYSTEM_NAME STREQUAL "Android")
    message("Using Android distribution")
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
    message("Using ios framework distribution")
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
    message("Using Windows distribution")
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
elseif(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    # Load nothing
else()
    message("Using OpenCV sources")

    FetchContent_Declare(
        # Downloading source doesn't give you working headers until make install
        # because OpenCV is designed to torture anyone who might try to incorporate
        # its source into a cmake project.
        opencv
        GIT_REPOSITORY https://github.com/opencv/opencv.git
        GIT_TAG        ${OPENCV_VERSION}
        UPDATE_DISCONNECTED # since we're using a fixed version, only download once
    )
    # set (OpenCV_DIR "${opencv_SOURCE_DIR}/build")
    message("OpenCV_DIR ${OpenCV_DIR}")
    message("FetchContent_Declare completed, starting FetchContent_MakeAvailable")
	FetchContent_MakeAvailable(opencv)
    # To assist find_package in finding OpenCV, set the OpenCV_DIR based on
    # https://github.com/opencv/opencv/blob/65573784c488f6548770381e2786fbf15e0f1e32/cmake/OpenCVInstallLayout.cmake#L68
endif()
message("OpenCV downloaded.")

if (CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    SET(OpenCV_DIR "C:/Users/stuar/git/opencv/build/install/lib/cmake/opencv4") # FIXME
    find_package(OpenCV
        REQUIRED COMPONENTS core imgproc
    )
else()
    find_package(OpenCV ${OPENCV_VERSION} REQUIRED COMPONENTS core imgproc imgcodecs)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	file(INSTALL "${_OpenCV_LIB_PATH}/" DESTINATION "${CMAKE_BINARY_DIR}/bin" FILES_MATCHING PATTERN "*.dll" )
endif()

message("OpenCV_DIR = ${OpenCV_DIR}")
message("OpenCV_SOURCE_DIR = ${OpenCV_SOURCE_DIR}")
message("OpenCV_BINARY_DIR = ${OpenCV_BINARY_DIR}")
message("OpenCV_INCLUDE_DIR = ${OpenCV_INCLUDE_DIR}")
message("opencv_SOURCE_DIR = ${opencv_SOURCE_DIR}")
message("opencv_BINARY_DIR = ${opencv_BINARY_DIR}")
message("opencv_INCLUDE_DIR = ${opencv_INCLUDE_DIR}")
message("OpenCV_LIBS = ${OpenCV_LIBS}")
