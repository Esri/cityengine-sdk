# Esri CityEngine SDK Example: PRT4Maya 

## Build Requirements for Windows:
- Visual Studio 2010 SP1
- (Visual Studio 2008)
- Boost
- cmake

## Build Requirements for Linux / OSX
- GCC
- cmake
- Boost

## Build Instructions (CMake)
  1. open shell and change to the examples/prt4maya directory
  2. edit common.cmake to your requirements
  3. run build.bat or build.sh
  4. add the following line to your <HOME>/maya/2012-x64/Maya.env file:```
MAYA_MODULE_PATH=<path the sdk>/esri-cityengine-sdk/examples/prt4maya/client/install
```
  5. (windows only) add the plug-ins directory to your PATH variable (either global or just for maya):```
set PATH=<path to sdk>\esri-cityengine-sdk\examples\prt4maya\client\install\plug-ins;%PATH%
```
  6. set license env vars:
    * ESRI_CE_SDK_LIC_FEATURE to one of { CityEngBas, CityEngBasFx, CityEngAdv, CityEngAdvFx }
    * ESRI_CE_SDK_LIC_HOST to <port>@<host> in case you selected a non-floating license (CityEng*Fx)
  7. run maya: prt4maya should appear in the plugins list

## Build Instructions (Visual Studio)
(TODO)