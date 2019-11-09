include(FetchContent)

if(CMAKE_SYSTEM_NAME STREQUAL "Android")

    FetchContent_Declare(
        opencv
        URL https://github.com/opencv/opencv/releases/download/4.1.2/opencv-4.1.2-android-sdk.zip
    )

else()

    FetchContent_Declare(
        opencv
        GIT_REPOSITORY https://github.com/opencv/opencv.git
    )
    set(BUILD_LIST "core,imgproc,imgcodecs" CACHE STRING "")

endif()

FetchContent_MakeAvailable(opencv)

if(CMAKE_SYSTEM_NAME STREQUAL "Android")
    set (OpenCV_DIR "${opencv_SOURCE_DIR}/sdk/native/jni")
endif()

find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs)
