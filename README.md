 © 2019 Stuart Edward Schechter (Github: @uppajung)
 
# KeySqr scanning algorithm

## Make sure to clone with ``--recursive`` to get opencv
```
git clone --recursive https://github.com/UppaJung/read-keysqr.git
```

## Before installing on MacOS/unix
```
brew install cmake
```

## Before installing on Windows

Install Windows Subsystem for Linux (WSL) and run script labeled setup-android-studio.sh
derived from https://gist.github.com/jjvillavicencio/18feb09f0e93e017a861678bc638dcb0

Install cmake from the installer downloadable from https://cmake.org/download/.

**Important**: Visual Studio unfortunatly defaults to overriding the working directory for Google Test set by CMAKE.  To fix this go to Visual Studio's debug menu or tool menu, choose the "options" item, and then go to the "Test Adapter For GoogleTest" tab.
Clear the "Working Directory" field to an empty string

## Installing OpenCV

As per recommendations of https://docs.opencv.org/4.1.1/d0/d76/tutorial_arm_crosscompile_with_cmake.html
```
sudo apt-get install gcc-arm-linux-gnuea
sudo apt-get install gcc-arm-linux-gnueabihf
```

https://www.sisik.eu/blog/android/ndk/opencv-without-java

```
cd extern/
export EXTERN_DIR=`pwd`
mkdir builds
cd builds
mkdir opencv
cd opencv
mkdir arm64-v8a
cd arm64-v8a
cmake ../../../opencv -DBUILD_opencv_ittnotify=OFF -DBUILD_ITT=OFF -DCV_DISABLE_OPTIMIZATION=ON -DWITH_CUDA=OFF -DWITH_OPENCL=OFF -DWITH_OPENCLAMDFFT=OFF -DWITH_OPENCLAMDBLAS=OFF -DWITH_VA_INTEL=OFF -DCPU_BASELINE_DISABLE=ON -DBUILD_TESTING=OFF -DBUILD_PERF_TESTS=OFF -DBUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=RELEASE -DBUILD_EXAMPLES=OFF -DBUILD_DOCS=OFF -DBUILD_opencv_apps=OFF -DBUILD_SHARED_LIBS=OFF -DOpenCV_STATIC=ON -DWITH_1394=OFF -DWITH_ARITH_DEC=OFF -DWITH_ARITH_ENC=OFF -DWITH_CUBLAS=OFF -DWITH_CUFFT=OFF -DWITH_FFMPEG=OFF -DWITH_GDAL=OFF -DWITH_GSTREAMER=OFF -DWITH_GTK=OFF -DWITH_HALIDE=OFF -DWITH_JASPER=OFF -DWITH_NVCUVID=OFF -DWITH_OPENEXR=OFF -DWITH_PROTOBUF=OFF -DWITH_PTHREADS_PF=OFF -DWITH_QUIRC=OFF -DWITH_V4L=OFF -DWITH_WEBP=OFF -DBUILD_LIST="core,imgcodecs,imgproc" -DANDROID_NDK=$ANDROID_HOME/ndk-bundle -DCMAKE_TOOLCHAIN_FILE=$ANDROID_HOME/ndk-bundle/build/cmake/android.toolchain.cmake -DANDROID_NATIVE_API_LEVEL=android-21 -DBUILD_JAVA=OFF -DBUILD_ANDROID_EXAMPLES=OFF -DBUILD_ANDROID_PROJECTS=OFF -DANDROID_STL=c++_shared -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX:PATH=$EXTERN_DIR -DANDROID_ABI=arm64-v8a
make
make install
cd ..
mkdir armeabi-v7a
cd armeabi-v7a
cmake ../../../opencv -DBUILD_opencv_ittnotify=OFF -DBUILD_ITT=OFF -DCV_DISABLE_OPTIMIZATION=ON -DWITH_CUDA=OFF -DWITH_OPENCL=OFF -DWITH_OPENCLAMDFFT=OFF -DWITH_OPENCLAMDBLAS=OFF -DWITH_VA_INTEL=OFF -DCPU_BASELINE_DISABLE=ON -DBUILD_TESTING=OFF -DBUILD_PERF_TESTS=OFF -DBUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=RELEASE -DBUILD_EXAMPLES=OFF -DBUILD_DOCS=OFF -DBUILD_opencv_apps=OFF -DBUILD_SHARED_LIBS=OFF -DOpenCV_STATIC=ON -DWITH_1394=OFF -DWITH_ARITH_DEC=OFF -DWITH_ARITH_ENC=OFF -DWITH_CUBLAS=OFF -DWITH_CUFFT=OFF -DWITH_FFMPEG=OFF -DWITH_GDAL=OFF -DWITH_GSTREAMER=OFF -DWITH_GTK=OFF -DWITH_HALIDE=OFF -DWITH_JASPER=OFF -DWITH_NVCUVID=OFF -DWITH_OPENEXR=OFF -DWITH_PROTOBUF=OFF -DWITH_PTHREADS_PF=OFF -DWITH_QUIRC=OFF -DWITH_V4L=OFF -DWITH_WEBP=OFF -DBUILD_LIST="core,imgcodecs,imgproc" -DANDROID_NDK=$ANDROID_HOME/ndk-bundle -DCMAKE_TOOLCHAIN_FILE=$ANDROID_HOME/ndk-bundle/build/cmake/android.toolchain.cmake -DANDROID_NATIVE_API_LEVEL=android-21 -DBUILD_JAVA=OFF -DBUILD_ANDROID_EXAMPLES=OFF -DBUILD_ANDROID_PROJECTS=OFF -DANDROID_STL=c++_shared -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX:PATH=$EXTERN_DIR -DANDROID_ABI=armeabi-v7a
make
make install
cd ..
mkdir x86
cd x86
cmake ../../../opencv -DBUILD_opencv_ittnotify=OFF -DBUILD_ITT=OFF -DCV_DISABLE_OPTIMIZATION=ON -DWITH_CUDA=OFF -DWITH_OPENCL=OFF -DWITH_OPENCLAMDFFT=OFF -DWITH_OPENCLAMDBLAS=OFF -DWITH_VA_INTEL=OFF -DCPU_BASELINE_DISABLE=ON -DBUILD_TESTING=OFF -DBUILD_PERF_TESTS=OFF -DBUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=RELEASE -DBUILD_EXAMPLES=OFF -DBUILD_DOCS=OFF -DBUILD_opencv_apps=OFF -DBUILD_SHARED_LIBS=OFF -DOpenCV_STATIC=ON -DWITH_1394=OFF -DWITH_ARITH_DEC=OFF -DWITH_ARITH_ENC=OFF -DWITH_CUBLAS=OFF -DWITH_CUFFT=OFF -DWITH_FFMPEG=OFF -DWITH_GDAL=OFF -DWITH_GSTREAMER=OFF -DWITH_GTK=OFF -DWITH_HALIDE=OFF -DWITH_JASPER=OFF -DWITH_NVCUVID=OFF -DWITH_OPENEXR=OFF -DWITH_PROTOBUF=OFF -DWITH_PTHREADS_PF=OFF -DWITH_QUIRC=OFF -DWITH_V4L=OFF -DWITH_WEBP=OFF -DBUILD_LIST="core,imgcodecs,imgproc" -DANDROID_NDK=$ANDROID_HOME/ndk-bundle -DCMAKE_TOOLCHAIN_FILE=$ANDROID_HOME/ndk-bundle/build/cmake/android.toolchain.cmake -DANDROID_NATIVE_API_LEVEL=android-21 -DBUILD_JAVA=OFF -DBUILD_ANDROID_EXAMPLES=OFF -DBUILD_ANDROID_PROJECTS=OFF -DANDROID_STL=c++_shared -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX:PATH=$EXTERN_DIR -DANDROID_ABI=x86
make
make install
cd ..
mkdir x86_64
cd x86_64
cmake ../../../opencv -DBUILD_opencv_ittnotify=OFF -DBUILD_ITT=OFF -DCV_DISABLE_OPTIMIZATION=ON -DWITH_CUDA=OFF -DWITH_OPENCL=OFF -DWITH_OPENCLAMDFFT=OFF -DWITH_OPENCLAMDBLAS=OFF -DWITH_VA_INTEL=OFF -DCPU_BASELINE_DISABLE=ON -DBUILD_TESTING=OFF -DBUILD_PERF_TESTS=OFF -DBUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=RELEASE -DBUILD_EXAMPLES=OFF -DBUILD_DOCS=OFF -DBUILD_opencv_apps=OFF -DBUILD_SHARED_LIBS=OFF -DOpenCV_STATIC=ON -DWITH_1394=OFF -DWITH_ARITH_DEC=OFF -DWITH_ARITH_ENC=OFF -DWITH_CUBLAS=OFF -DWITH_CUFFT=OFF -DWITH_FFMPEG=OFF -DWITH_GDAL=OFF -DWITH_GSTREAMER=OFF -DWITH_GTK=OFF -DWITH_HALIDE=OFF -DWITH_JASPER=OFF -DWITH_NVCUVID=OFF -DWITH_OPENEXR=OFF -DWITH_PROTOBUF=OFF -DWITH_PTHREADS_PF=OFF -DWITH_QUIRC=OFF -DWITH_V4L=OFF -DWITH_WEBP=OFF -DBUILD_LIST="core,imgcodecs,imgproc" -DANDROID_NDK=$ANDROID_HOME/ndk-bundle -DCMAKE_TOOLCHAIN_FILE=$ANDROID_HOME/ndk-bundle/build/cmake/android.toolchain.cmake -DANDROID_NATIVE_API_LEVEL=android-21 -DBUILD_JAVA=OFF -DBUILD_ANDROID_EXAMPLES=OFF -DBUILD_ANDROID_PROJECTS=OFF -DANDROID_STL=c++_shared -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX:PATH=$EXTERN_DIR -DANDROID_ABI=x86_64
make
make install
cd ..
```

You'll need the Android NDK (Native Development Kit installed) for cross compiler support.

You need to compile support for the following architectures:

* armeabi-v7a
* arm64-v8a
* x86
* x86_64

NDK on windows: %CSIDL_LOCAL_APPDATA%\Android\Sdk

Additional reading

Building OpenCV:
``
cd ~/opencv/platforms/linux
mkdir -p bu
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../android.toolchain.cmake
``
// cmake -DCMAKE_TOOLCHAIN_FILE=../arm-gnueabi.toolchain.cmake ../../..

https://developer.android.com/ndk/guides/other_build_systems
Note: For 32-bit ARM, the compiler is prefixed with armv7a-linux-androideabi, but the binutils tools are prefixed with arm-linux-androideabi. For other architectures, the prefixes are the same for all tools.

FIXME for ios.

## Install

Since we include GoogleTest as a submodule, you will need to clone this repository using the ``--recursive`` directory so that the submodule will be downloaded. (If you forgot, use ``git submodule update --init --recursive``.)


```
git clone --recursive https://github.com/UppaJung/read-keysqr.git
cd read-keysqr
cmake -B build
cd build
make
ctest
```

