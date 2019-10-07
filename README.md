 © 2019 Stuart Edward Schechter (Github: @uppajung)
 
# KeySqr scanning algorithm

## Before installing on MacOS/unix
```
brew install cmake
brew install opencv
```

## Before installing on Windows

```
vcpck install opencv
```
Install cmake from the installer downloadable from https://cmake.org/download/.

**Important**: Visual studio will not be able to read the test images from the right
directory if the preferences for the Google Test directory have been set within
Visual Studio.  You will need to clear that setting. Go t
Test Adapter for Google Test -> Test Exectuion -> Working Directory
Clear the "Working Directory" Field.

## Install

Since we include GoogleTest as a submodule, you will need to clone this repository using the ``--recursive`` directory so that the submodule will be downloaded. (If you forgot, use ``git submodule update --init --recursive``.)


```
git clone --recursive https://github.com/UppaJung/read-keysqr.git
cd read-keysqr
cmake .
make
ctest
```

