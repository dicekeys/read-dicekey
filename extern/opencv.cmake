set (Extern_DIR "${CMAKE_SOURCE_DIR}/extern")

if(CMAKE_SYSTEM_NAME STREQUAL "Android")
    set (Cross_OpenCV_DIR "${Extern_DIR}/OpenCV-android-sdk/sdk/native/jni")
else()
    set (Cross_OpenCV_DIR "${Extern_DIR}/opencv/build")
endif()

set (OpenCV_DIR ${Cross_OpenCV_DIR})
find_package(OpenCV COMPONENTS core imgproc imgcodecs)

if(NOT OpenCV_core_FOUND)

    if(CMAKE_SYSTEM_NAME STREQUAL "Android")

        file(DOWNLOAD "https://github.com/opencv/opencv/releases/download/4.1.2/opencv-4.1.2-android-sdk.zip" "${Extern_DIR}/opencv-android.zip")
        execute_process(COMMAND "${CMAKE_COMMAND}" -E tar xzf "${Extern_DIR}/opencv-android.zip" WORKING_DIRECTORY "${Extern_DIR}")
        file(REMOVE "${Extern_DIR}/opencv-android.zip")

    else()

        message("Compiling OpenCV library")
        execute_process(COMMAND "${CMAKE_COMMAND}" -E make_directory "${Cross_OpenCV_DIR}")
        execute_process(COMMAND "${CMAKE_COMMAND}" "-DBUILD_LIST=core,imgproc,imgcodecs" ".." WORKING_DIRECTORY "${Cross_OpenCV_DIR}")
        execute_process(COMMAND "${CMAKE_COMMAND}" "--build" "." WORKING_DIRECTORY "${Cross_OpenCV_DIR}")

    endif()

    set (OpenCV_DIR ${Cross_OpenCV_DIR})
    find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs)
endif()