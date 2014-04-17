@ECHO OFF

set maya_DIR=%1
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

set CLIENTVC=0.0
if %maya_DIR:~-4% == "2012" (set CLIENTVC=9.0)
if %maya_DIR:~-4% == "2014" (set CLIENTVC=10.0)

pushd client
call "C:\Program Files (x86)\Microsoft Visual Studio %CLIENTVC%\VC\vcvarsall.bat" amd64
rd /S /Q build install
mkdir build
cd build
cmake -G %GENERATOR% -DCMAKE_BUILD_TYPE=%BUILDTYPE% ../src
nmake install
popd
