PRT4CMD - CityEngine SDK Command Line Utility
---------------------------------------------


DISCLAIMER
----------

This is an example for using the CityEngine SDK. There is no support for this application. 
If you find a bug or want to enhance functionality you have to fix/do this yourself.


PREREQUISITES
-------------

Before you start working with this example, please make sure you follow
the main installation instructions for the CityEngine SDK in the
README.md file at the root of this example repository. This will 
explain how to get the sdk binaries and example data.


SOFTWARE REQUIREMENTS
---------------------

- See "General Software Requirements" (Windows)


BUILD INSTRUCTIONS
------------------

1. Open a `VS2017 x64 Native Tools Command Prompt`
1. Change into the example directory: `cd <your path to>\esri-cityengine-sdk\examples\prt4cmd`
1. Prepare build directory: `mkdir build`
1. And change into it: `cd build`
1. Run cmake: `cmake -G "NMake Makefiles" ..\src`
1. Compile: `nmake install`
1. The build result will appear in an `install` directory in parallal to the `build` directory.


USING PRT4CMD
-------------

1. Open a cmd shell and `cd` into the above `install` directory.
1. Type `bin\prt4cmd.exe` or `bin\prt4cmd.exe -h` to see the list of options.
1. Type the following to generate the "candler" building model in the wavefront obj format:
   ```
   bin\prt4cmd -l 3 -g <your path to>\esri-cityengine-sdk\data\candler_footprint.obj -p <your path to>\esri-cityengine-sdk\data\candler.rpk -a ruleFile:string=bin/candler.cgb -a startRule:string=Default$Footprint -e com.esri.prt.codecs.OBJEncoder -z baseName:string=theCandler
   ```
1. The result is placed in the output directory:
   - `theCandler_0.obj`
   - `theCandler.mtl`
   - Bunch of texture files
1. To regenerate the same model with a different height, use the following command (note the additional `-a BuildingHeight...` statement):
   ```
   bin\prt4cmd -l 3 -g <your path to>\esri-cityengine-sdk\data\candler_footprint.obj -p <your path to>\esri-cityengine-sdk\data\candler.rpk -a ruleFile:string=bin/candler.cgb -a startRule:string=Default$Footprint -a BuildingHeight:float=30 -e com.esri.prt.codecs.OBJEncoder -z baseName:string=theCandler
   ```
1. The obj file will now contain a model which is 30m high instead of the default 62m.


LICENSING
---------

Copyright (c) 2018 Esri

You may not use the content of this repository except in compliance with the following Licenses:
  1. All content of all directories **except "examples"** is licensed under the CityEngine EULA, see [EULA.pdf](../../EULA.pdf).
  2. All content in the "examples" directory tree is licensed under the APACHE 2.0 license. You may obtain a copy of this license at http://www.apache.org/licenses/LICENSE-2.0.
