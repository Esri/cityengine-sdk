@ECHO OFF

if "%~1"=="" (
	echo ERROR: first argument must be path to prt/cmake, second argument must be path to maya
	set ERRORLEVEL=1
	exit /b %ERRORLEVEL%
)

setlocal

set prt_DIR=%~1
set maya_DIR=%~2

set VER_MAJOR=0
set VER_MINOR=0
set VER_MICRO=0
if not "%~4"=="" (
	set VER_MAJOR=%~2
	set VER_MINOR=%~3
	set VER_MICRO=%~4
	set CLIENT_TARGET=package
)

IF "%CMAKE_EXECUTABLE%"=="" (set CMAKE_EXECUTABLE=cmake)

set GENERATOR="NMake Makefiles"

rd /S /Q build

setlocal
call "%ProgramFiles(x86)%\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" amd64
mkdir build\codec
pushd build\codec
"%CMAKE_EXECUTABLE%" -G %GENERATOR% -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../../install ../../src/codec
nmake install
popd
endlocal

setlocal
call "%ProgramFiles(x86)%\Microsoft Visual Studio 11.0\VC\vcvarsall.bat" amd64
mkdir build\client
pushd build\client
"%CMAKE_EXECUTABLE%" -G %GENERATOR% -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../../install -DPRT4MAYA_VERSION_MAJOR=%VER_MAJOR% -DPRT4MAYA_VERSION_MINOR=%VER_MINOR% -DPRT4MAYA_VERSION_MICRO=%VER_MICRO% ../../src/client
nmake install
popd
endlocal

endlocal
