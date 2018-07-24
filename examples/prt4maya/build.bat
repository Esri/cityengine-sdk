@ECHO OFF
set ERRORLEVEL=0

if "%~1"=="" (
	echo ERROR: first argument must be path to prt/cmake, second argument must be path to maya
	set ERRORLEVEL=1
	exit /b %ERRORLEVEL%
)

setlocal

set prt_DIR=%~1
set maya_DIR=%~2
set GENERATOR="NMake Makefiles"

if "%CMAKE_EXECUTABLE%"=="" (set CMAKE_EXECUTABLE=cmake)

set VER_MAJOR=0
set VER_MINOR=0
set VER_MICRO=0
set CLIENT_TARGET=install
if not "%~3"=="" (
	set VER_MAJOR=%~3
	set VER_MINOR=%~4
	set VER_MICRO=%~5
	set CLIENT_TARGET=package
)

rd /S /Q build
rd /S /Q install

setlocal
pushd %ProgramFiles(x86)%\Microsoft Visual Studio\Installer
for /f "usebackq tokens=1* delims=: " %%i in (`vswhere -latest -requires Microsoft.Component.MSBuild`) do (
  if /i "%%i"=="installationPath" set InstallDir=%%j
)
popd
@ECHO ON
echo InstallDir: "%InstallDir%"
@ECHO OFF
pushd .
call "%InstallDir%\VC\Auxiliary\Build\vcvarsall.bat" x64 8.1 -vcvars_ver=14.11
popd
mkdir build\codec
pushd build\codec
"%CMAKE_EXECUTABLE%" -G %GENERATOR% -DCMAKE_BUILD_TYPE=Release -Dmaya_DIR=%maya_DIR% ../../src/codec
nmake
popd
endlocal

if %ERRORLEVEL% neq 0 (
     echo "error building codec: %ERRORLEVEL%"
     exit /b %ERRORLEVEL%
)

setlocal
call "%ProgramFiles(x86)%\Microsoft Visual Studio 11.0\VC\vcvarsall.bat" amd64
mkdir build\client
pushd build\client
"%CMAKE_EXECUTABLE%" -G %GENERATOR% -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../../install -Dmaya_DIR="%maya_DIR%" -DPRT4MAYA_VERSION_MAJOR=%VER_MAJOR% -DPRT4MAYA_VERSION_MINOR=%VER_MINOR% -DPRT4MAYA_VERSION_MICRO=%VER_MICRO% ../../src/client
nmake %CLIENT_TARGET%
popd
endlocal

endlocal
