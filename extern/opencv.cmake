include(FetchContent)

FetchContent_Declare(
    opencv
    GIT_REPOSITORY https://github.com/opencv/opencv.git
)
set(BUILD_LIST "core,imgproc,imgcodecs" CACHE STRING "")
FetchContent_MakeAvailable(opencv)

find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs)
