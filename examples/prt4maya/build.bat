@ECHO OFF

set BUILDTYPE=Release
set GENERATOR="NMake Makefiles"

pushd codec
call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" amd64
rd /S /Q build install
mkdir build
cd build
cmake -G %GENERATOR% -DCMAKE_BUILD_TYPE=%BUILDTYPE% ../src
nmake install
popd

pushd client
call "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\vcvarsall.bat" amd64
rd /S /Q build install
mkdir build
cd build
cmake -G %GENERATOR% -DCMAKE_BUILD_TYPE=%BUILDTYPE% ../src
nmake install
popd