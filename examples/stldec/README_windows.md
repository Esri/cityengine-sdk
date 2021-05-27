# STLDEC - CityEngine SDK Custom STL Decoder

## PREREQUISITES

Before you start working with this example, please make sure you follow the main installation instructions for the CityEngine SDK in the README.md file at the root of this example repository. This will explain how to get the sdk binaries and example data.

## SOFTWARE REQUIREMENTS

* see "General Software Requirements" (Windows)
* Make sure you use the **exact** compiler for PRT extensions

## BUILD INSTRUCTIONS

1. Open a `VS2019 x64 Native Tools Command Prompt`
1. Change into the example directory: `cd <your path to>\cityengine-sdk\examples\stldec`
1. Create a build directory: `mkdir build`
1. Change into the build directory: `cd build`
1. Run cmake: `cmake -G "NMake Makefiles" ..\src`
1. Compile: `nmake install`
1. The build result will appear in the `install` directory in parallel to the `build` directory.

## INSTALLATION INSTRUCTIONS FOR CITYENGINE

1. Locate the `stldec` extension library in the `install` directory above, e.g. at:
   `<your path to>\cityengine-sdk\examples\stldec\install\lib\prt_stldec.dll`
1. Copy `prt_stldec.dll` into `<CityEngine installation location>\plugins\com.esri.prt.clients.ce.win32.win32.x86_64_1.0.0\lib\`
1. Start CityEngine and verify that STL files are now previewed in the file navigator.

## LICENSING

The CityEngine SDK is free for personal, educational, and non-commercial use. Commercial use requires at least one commercial license of the latest CityEngine version installed in the organization. Redistribution or web service offerings are not allowed unless expressly permitted.

The CityEngine SDK is licensed under the Esri Terms of Use:

* <https://www.esri.com/en-us/legal/terms/full-master-agreement>
* <https://www.esri.com/en-us/legal/terms/product-specific-scope-of-use>
* All content in the "Examples" directory/section is licensed under the APACHE 2.0 license. You may obtain a copy of this license at <https://www.apache.org/licenses/LICENSE-2.0>.
* For questions or enquiries regarding licensing, please contact the CityEngine team at cityengine-info@esri.com.
