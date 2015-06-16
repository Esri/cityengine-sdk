@ECHO OFF

if "%~1"=="" (
	echo "ERROR: first argument must be maya path"
	set ERRORLEVEL=1
	exit /b %ERRORLEVEL%
)

setlocal

set maya_DIR=%~1
set MAYA_VERSION=%maya_DIR:~-4%

set CLIENT_TARGET=install
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

set BUILDTYPE=Release
set GENERATOR="NMake Makefiles"

setlocal
pushd codec
call "%ProgramFiles(x86)%\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" amd64
rd /S /Q build install
mkdir build
cd build
"%CMAKE_EXECUTABLE%" -G %GENERATOR% -DCMAKE_BUILD_TYPE=%BUILDTYPE% ../src
nmake install
popd
endlocal

set CLIENTVC=0.0
if "%MAYA_VERSION%" == "2016" (set CLIENTVC=11.0)
if "%CLIENTVC%"=="0.0" (
	echo "ERROR: could not derive the compiler version from the maya path, please check maya path"
	endlocal
	set ERRORLEVEL=1
	exit /b %ERRORLEVEL%
)

setlocal
pushd client
call "%ProgramFiles(x86)%\Microsoft Visual Studio %CLIENTVC%\VC\vcvarsall.bat" amd64
rd /S /Q build install
mkdir build
cd build
"%CMAKE_EXECUTABLE%" -G %GENERATOR% -DCMAKE_BUILD_TYPE=%BUILDTYPE% -DPRT4MAYA_VERSION_MAJOR=%VER_MAJOR% -DPRT4MAYA_VERSION_MINOR=%VER_MINOR% -DPRT4MAYA_VERSION_MICRO=%VER_MICRO% -DMAYA_VERSION=%MAYA_VERSION% ../src
nmake %CLIENT_TARGET%
popd
endlocal

endlocal
