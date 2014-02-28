# Esri CityEngine SDK

The Esri CityEngine 2013 is based on the procedural runtime, which is the underlying engine that supports two GP tools in ArcGIS 10.2 and drives procedural symbology in the release of ArcGIS Professional. The CityEngine SDK enables you as a 3rd party developer to extend CityEngine with additional import and export formats and storage backends beyond simple files. Moreover, you can integrate the procedural runtime in your own client applications taking full advantage of the procedural core without running CityEngine or ArcGIS.

This document explains how to install the CityEngine SDK and how to build the included examples. For in-depth documentation please see the doc directory.

## Features
* C++ SDK
* Documentation
* Example Code

## Documentation
* Whitepaper: [doc/esri_prt_whitepaper.pdf](doc/esri_prt_whitepaper.pdf)
* Architecture: [doc/esri_prt_architecture.pdf](doc/esri_prt_architecture.pdf)
* API Reference: [doc/html/index.html](doc/html/index.html)

## Requirements
* License for CityEngine 2013 (or later)
* Supported OS: Windows 7, Windows 8, MacOSX 10.7, MacOSX 10.8, RHEL 6.4
* C++ Compiler:
  * Windows: MSVC 10.0 or later
  * MacOSX: Clang 3.0 or later
  * Linux: GCC 4.1.2 or later
* (Optional) To compile PRTX extensions (e.g. custom encoders) you need to use these **exact** C++ compiler versions:
  * Windows: Microsoft C++ Compiler (cl.exe) 16.00.40219.01 for x64, included in either one of these products (i.e. choose one):
    * Visual Studio 2010 SP1 64bit
    * Windows SDK 7.1 64bit with Visual C++ 2010 Compiler SP1 (Attention: [known issue with SP1](https://connect.microsoft.com/VisualStudio/feedback/details/660584))
  * MacOSX: Apple GCC 4.2.1 included in:
    * XCode 4.2.1 and MacOSX SDK 10.7 (Based on Apple Inc. build 5658, LLVM build 2336.1.00)
  * Linux: GCC 4.1.2 64bit
* Linux/OSX: GNU Make 3.82 or later
* Windows: NMake 10.0 or later (part of Visual Studio [Express])
* CMake 2.8.10 or later (http://www.cmake.org)
* boost 1.34 or later (http://www.boost.org)

## Installation Instructions
TODO

## Examples
The SDK contains a (growing) number of examples with source code. Foremost, the prt4cmd example is a comprehensive example of the PRT API. It allows you to apply a rule package onto a initial shape and write the resulting geometry to disk. We encourage you to modify and play with this example source code in order to fully explore the possibilities of the CityEngine SDK.

### Generic Build Instructions
The following build instructions apply to all examples. See the individual example sections for any exceptions or special steps.
  1. cd into the examples/[example] directory
  2. Create an empty "build" directory in parallel to the src directory of the CLI example and cd into it
  3. Generate the make files:
    * on Linux/OSX: `cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release ../src`
    * on Windows: `cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release ../src`
  4. Build and install:
    * on Linux/OSX: `make install`
    * on Windows: `nmake install`

This will create a ready-to-run or ready-to-use installation of the example in the "install" directory in parallel to the src and build directories.

BOOST NOTE: if the example build script complains about "boost not found", please extend the cmake call in step 3 like this:
```
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release \
	-DBOOST_INCLUDEDIR=xxx \
	-DBOOST_LIBRARYDIR=XXX \
	-DBoost_USE_STATIC_LIBS=ON \
	../src 
```

### The prt4cmd CLI example
Note: all examples below assume that your current working directory is the "install" directory from above.

#### Displaying the help screen
Simply run the example without any arguments:
```
bin/prt4cmd
```

#### Specifying the license
The SDK will make use of your installed CityEngine license (node-locked or by license server).
The "-f" argument specifies the type of license:
- CityEngBas (basic network license)
- CityEngBasFx (basic node-locked license)
- CityEngAdv (advanced network license)
- CityEngAdvFx (advanced node-locked license)

The "-s" argument is only needed for the network license types. 

```
bin/prt4cmd -f CityEngAdv -s 27000@example.com
```

For simplicity, the license arguments have been omitted in the below examples.


#### Generate a building with the included example rule package
The following call will apply the rule package "data/candler.02.rpk" onto the initial shape geometry "data/candler_lot.obj".

*Windows Note: On the Windows command line, please use `Default$Lot` instead of `Default\$Lot` and omit the backslashes.*
```
bin/prt4cmd \
	-l 3 -p data/candler.02.rpk \
	-a ruleFile:string=bin/candler.01.cgb \
	-a startRule:string=Default\$Lot \
	-e com.esri.prt.codecs.OBJEncoder \
	-z baseName:string=myhouse \
	-g data/candler_lot.obj
```
Upon completion you should find an wavefront obj file inside the install/output folder with mtl and textures.

![myhouse62.png](doc/images/myhouse62.png "Building with height=62.0")

#### Generate the building with a different height
By adding the line "-a height:float=30" we change the height of the building from its default value of 62.0 to 30.0:

*Windows Note: On the Windows command line, please use `Default$Lot` instead of `Default\$Lot` and omit the backslashes.*
```
bin/prt4cmd \
	-l 3 \
	-p data/candler.02.rpk \
	-a ruleFile:string=bin/candler.01.cgb \
	-a startRule:string=Default\$Lot \
	-a height:float=30 \
	-e com.esri.prt.codecs.OBJEncoder \
	-z baseName:string=myhouse \
	-g data/candler_lot.obj
```

![myhouse30.png](doc/images/myhouse30.png "Building with height=30.0")

### The stlenc custom encoder example
The stlenc example demonstrates the use of the PRTX API to create custom encoders which are ready to be used in prt-based host applications.

Note: Libraries based on PRTX must be compiled with a specific compiler version, see requirements.

#### Use the STL encoder in the prt4cmd example
  1. Build the prt4cmd and stlenc examples using the generic build instructions above
  2. Copy stlenc/install/lib/libprt_stlenc.*  into prt4cmd/install/lib/
  3. The STL encoder can now be invoked with the id 'com.esri.prt.examples.STLEncoder'

## Resources
* Forum: http://forums.arcgis.com/forums/204-CityEngine-General-Discussion
* Facebook: https://www.facebook.com/CityEngine
* Twitter: https://twitter.com/CityEngine

## Issues

Find a bug or want to request a new feature?  Please let us know by submitting an issue.

## Contributing

Anyone and everyone is welcome to contribute and to extend and improve the examples.

## Licensing

Copyright 2014 Esri

You may not use the content of this repository except in compliance with the following Licenses:
  1. All content of all directories **except "examples"** is licensed under the CityEngine EULA, see [license/EULA.pdf](license/EULA.pdf).
  2. All content in the "examples" directory tree is licensed under the APACHE 2.0 license. You may obtain a copy of this license at http://www.apache.org/licenses/LICENSE-2.0. A copy of the license is also available in the repository at [license/APACHE-LICENSE-2.0.txt](license/APACHE-LICENSE-2.0.txt).

[](Esri Tags: CityEngine)
[](Esri Language: C++)
