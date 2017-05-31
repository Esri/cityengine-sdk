STLDEC - CityEngine SDK Custom STL Decoder
------------------------------------------


PREREQUISITES
-------------

Before you start working with this example, please make sure you follow
the main installation instructions for the CityEngine SDK in the
README.md file at the root of this example repository. This will
explain how to get the sdk binaries and example data.


SOFTWARE REQUIREMENTS
---------------------

- see "General Software Requirements" (Windows)
- Make sure you use the **exact** compiler for PRT extensions


BUILD INSTRUCTIONS
------------------

1. Open a `VS2015 x64 Native Tools Command Prompt`
1. Change into the example directory: `cd <your path to>\esri-cityengine-sdk\examples\stldec`
1. Create a build directory: `mkdir build`
1. Change into the build directory: `cd build`
1. Run cmake: `cmake -G "NMake Makefiles" ..\src`
1. Compile: `nmake install`
1. The build result will appear in the `install` directory in parallel to the `build` directory.


INSTALLATION INSTRUCTIONS FOR CITYENGINE
----------------------------------------

1. Locate the `stldec` extension library in the `install` directory above, e.g. at:
   `<your path to>\esri-cityengine-sdk\examples\stldec\install\lib\prt_stldec.dll`
1. Copy `prt_stldec.dll` into `<CityEngine installation location>\plugins\com.esri.prt.clients.ce.win32.win32.x86_64_1.0.0\lib\`
1. Start CityEngine and verify that STL files are now previewed in the file navigator.


LICENSING
---------

Copyright (c) 2017 Esri

You may not use the content of this repository except in compliance with the following Licenses:
  1. All content of all directories **except "examples"** is licensed under the CityEngine EULA, see [license/EULA.pdf](license/EULA.pdf).
  2. All content in the "examples" directory tree is licensed under the APACHE 2.0 license. You may obtain a copy of this license at http://www.apache.org/licenses/LICENSE-2.0. A copy of the license is also available in the repository at [license/APACHE-LICENSE-2.0.txt](license/APACHE-LICENSE-2.0.txt).
