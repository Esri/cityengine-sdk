@ECHO OFF

set MAYA=%1
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
SET MAYA|FINDSTR /b "MAYA="|FINDSTR /i 2012 >nul
IF ERRORLEVEL 1 (set CLIENTVC=9.0)
SET MAYA|FINDSTR /b "MAYA="|FINDSTR /i 2014 >nul
IF ERRORLEVEL 1 (set CLIENTVC=10.0)

pushd client
call "C:\Program Files (x86)\Microsoft Visual Studio %CLIENTVC%\VC\vcvarsall.bat" amd64
rd /S /Q build install
mkdir build
cd build
cmake -G %GENERATOR% -DCMAKE_BUILD_TYPE=%BUILDTYPE% ../src
nmake install
popd
