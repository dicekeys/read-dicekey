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

**Important**: Visual Studio unfortunatly defaults to overriding the working directory for Google Test set by CMAKE.  To fix this go to Visual Studio's debug menu or tool menu, choose the "options" item, and then go to the "Test Adapter For GoogleTest" tab.
Clear the "Working Directory" field to an empty string


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

