PRT4CMD - CityEngine SDK Command Line Utility
---------------------------------------------


DISCLAIMER
----------

This is an example for using the CityEngine SDK. There is no support for this application. 
If you find a bug or want to enhance functionality you have to do this yourself.


PREREQUISITES
-------------

Before you start working with this example, please make sure you follow
the main installation instructions for the CityEngine SDK in the
README.md file at the root of this example repository. This will 
explain how to get the sdk binaries and example data.


SOFTWARE REQUIREMENTS
---------------------

- See "General Software Requirements - Linux or MacOSX, for simple clients"
- Boost libraries 1.53 or later (binary build for your compiler / platform, http://www.boost.org)


BUILD INSTRUCTIONS
------------------

1. Open a terminal (e.g. bash)
1. Change into the example directory: `cd <your path to>/esri-cityengine-sdk/examples/prt4cmd`
1. Prepare build directory: `mkdir build && cd build`
1. Run cmake (trying to use boost from your system path): `cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DBoost_NO_BOOST_CMAKE=ON ../src`
1. Compile: `make install`
1. The build result will appear in a `install` directory in parallel to the `build` directory.


USING PRT4CMD
-------------

1. Open a bash shell and cd in to the above install directory: `cd ../install`
1. Type `bin/prt4cmd` or `bin/prt4cmd -h` to see the list of options.
1. Type the following to generate the "candler" building model in the wavefront obj format (replace `<your path to>` with the absolute path to the corresponding directory):
   ```
   bin/prt4cmd \
        -f CityEngAdvFx \
        -l 3 \
        -g <your path to>/esri-cityengine-sdk/data/candler_footprint.obj \
        -p <your path to>/esri-cityengine-sdk/data/candler.rpk \
        -a ruleFile:string=bin/candler.cgb \
        -a startRule:string=Default\$Footprint \
        -e com.esri.prt.codecs.OBJEncoder \
        -z baseName:string=theCandler
   ```
1. The result is placed in the output directory:
   - `theCandler.obj`
   - `theCandler.mtl`
   - Bunch of texture files

1. To regenerate the same model with a different height, use the following command (note the additional `-a BuildingHeight...` statement):
   ```
   bin/prt4cmd \
        -f CityEngAdvFx \
        -l 3 \
        -g <your path to>/esri-cityengine-sdk/data/candler_footprint.obj \
        -p <your path to>/esri-cityengine-sdk/data/candler.rpk \
        -a ruleFile:string=bin/candler.cgb \
        -a startRule:string=Default\$Footprint \
        -a BuildingHeight:float=30 \
        -e com.esri.prt.codecs.OBJEncoder \
        -z baseName:string=theCandler
   ```
1. The obj file will now contain a model which is 30m high instead of the default 62m.


SPECIFYING THE LICENSE TYPE
---------------------------

The SDK will make use of your installed CityEngine license (node-locked or by license server).
The `-f` argument specifies the type of license:
- CityEngBas (basic network license)
- CityEngBasFx (basic node-locked license)
- CityEngAdv (advanced network license)
- CityEngAdvFx (advanced node-locked license)

The `-s` argument is only needed for the network license types, for example `bin/prt4cmd -f CityEngAdv -s 27000@example.com`


LICENSING
---------

Copyright (c) 2016 Esri

You may not use the content of this repository except in compliance with the following Licenses:
  1. All content of all directories **except "examples"** is licensed under the CityEngine EULA, see [license/EULA.pdf](license/EULA.pdf).
  2. All content in the "examples" directory tree is licensed under the APACHE 2.0 license. You may obtain a copy of this license at http://www.apache.org/licenses/LICENSE-2.0. A copy of the license is also available in the repository at [license/APACHE-LICENSE-2.0.txt](license/APACHE-LICENSE-2.0.txt).
