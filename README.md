 © 2019 Stuart Edward Schechter (Github: @uppajung)
 
# DiceKeys scanning algorithm


## Clone the repository with the ``--recursive`` option

Since we include GoogleTest as a submoulde, you will need to clone this repository using the ``--recursive`` directory so that the submodule will be downloaded. (If you forgot, use ``git submodule update --init --recursive``.)

```
git clone --recursive https://github.com/dicekeys/read-dicekey.git
cd read-dicekey
```

#### Prerequisite installs

 - cmake >= 3.15.0
 - ninja

#### Windows
Open this directory in Visual Studio 2019 to automatically build and load tests into test explorer.

#### Compiling and running tests on unix/macos

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

