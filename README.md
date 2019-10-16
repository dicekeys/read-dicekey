 © 2019 Stuart Edward Schechter (Github: @uppajung)
 
# KeySqr scanning algorithm

## Clone the repository with the ``--recursive`` option

Since we include GoogleTest and opencv as submodules, you will need to clone this repository using the ``--recursive`` directory so that the submodule will be downloaded. (If you forgot, use ``git submodule update --init --recursive``.)

```
git clone --recursive https://github.com/UppaJung/read-keysqr.git
cd read-keysqr
```

Install Android Studio and the NDK
```
bash scripts/install-android-studio-and-ndk.sh
```



### On MacOS/unix
```
brew install cmake
```

### Before compiling OpenCV on Windows

Install Windows Subsystem for Linux (WSL) and run script labeled setup-android-studio.sh
derived from https://gist.github.com/jjvillavicencio/18feb09f0e93e017a861678bc638dcb0

Install cmake from the installer from https://cmake.org/download/.

Download the cross-platform build tools as per recommendations of [the CMAKE ARM cross-compilation tutorial](https://docs.opencv.org/4.1.1/d0/d76/tutorial_arm_crosscompile_with_cmake.html):
```
sudo apt-get install gcc-arm-linux-gnuea
sudo apt-get install gcc-arm-linux-gnueabihf
```

## Installing OpenCV

Some references materials
[1](https://developer.android.com/ndk/guides/other_build_systems)
[2](https://www.sisik.eu/blog/android/ndk/opencv-without-java)
[3](https://amin-ahmadi.com/2019/02/03/how-to-build-opencv-4-x-for-native-android-development/)

You'll need the Android NDK (Native Development Kit installed) for cross compiler support.

You need to compile support for the following architectures:

* armeabi-v7a
* arm64-v8a
* x86
* x86_64

```
cd extern/
bash compile-open-cv.sh
cd ..
```

FIXME for ios.


### Compiling the library

```
cd read-keysqr
cmake -B build
cd build
make
ctest
```
#### Important note if using Visual Studio (Windows without WSL) with this project

Visual Studio unfortunatly defaults to overriding the working directory for Google Test set by CMAKE. If you don't fix this before running tests, they will fail due to being unable to find the test files.

 To fix this go to Visual Studio's debug menu or tool menu, choose the "options" item, and then go to the "Test Adapter For GoogleTest" tab.
Clear the "Working Directory" field to an empty string

