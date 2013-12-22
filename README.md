# Esri CityEngine SDK

The Esri CityEngine 2013 is based on the procedural runtime, which is the underlying engine that supports two GP tools in ArcGIS 10.2 and drives procedural symbology in the release of ArcGIS Professional. The CityEngine SDK enables you as a 3rd party developer to extend CityEngine with additional import and export formats and storage backends beyond simple files. Moreover, you can integrate the procedural runtime in your own client applications taking full advantage of the procedural core without running CityEngine or ArcGIS.

## Features
* C++ SDK
* Documentation
* Example Code

## Requirements
* CityEngine 2013 License
* C++ Compiler (MSVC 10.0 or later / GCC 4.1.2 or later / Clang 3.0 or later)
* Linux/OSX: GNU Make 3.82 or later
* Windows: NMake 10.0 or later (part of Visual Studio [Express])
* CMake 2.8.10 or later (http://www.cmake.org)
* boost 1.34 or later (http://www.boost.org)

## Installation Instructions
The SDK is installed by simply downloading and unzipping an archive from the Releases/Tags page. For more convenience and easy updates you may also want to directly clone this repository (there is one branch per platform).

## The prt4cmd CLI example
The bundled prt4cmd project is a comprehensive example of the PRT API. It allows you to apply a rule package onto a initial shape and write the resulting geometry to disk. We encourage you to modify and play with this example source code in order to fully explore the possibilities of the CityEngine SDK.

### Building the CLI example
1. cd into the examples/prt4cmd directory
2. Create an empty "build" directory in parallel to the src directory of the CLI example and cd into it
3. Generate the make files:
  * on Linux/OSX: `cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release ../src`
  * on Windows: `cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release ../src`
4. Build and install:
  * on Linux/OSX: `make install`
  * on Windows: `nmake install`

This will create a ready-to-run installation of prt4cmd in the "install" directory in parallel to the src and build directories.

BOOST NOTE: if you do not have boost installed on a system path, please extend the cmake call in step 3 like this:
```
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release \
	-DBOOST_INCLUDEDIR=xxx \
	-DBOOST_LIBRARYDIR=XXX \
	-DBoost_USE_STATIC_LIBS=ON \
	../src 
```


### Running the CLI example
Note: all examples below assume that your current working directory is the "install" directory from above.

#### Displaying the help screen
Simply run the example without any arguments:
```
bin/prt4cmd
```

#### Specifying the license
The SDK will make use of your installed CityEngine license (node-locked or by license server).
The "-f" argument specifies the type of license:
- CityEngBasFx (basic network license)
- CityEngBas (basic node-locked license)
- CityEngAdvFx (advanced network license)
- CityEngAdv (advanced node-locked license)

The "-s" argument is only needed for the network license types. 

```
bin/prt4cmd \
	-f CityEngAdvFx \
	-s 27000@example.com
```

For simplicity, the license arguments have been omitted in the below examples.


#### Generate a building with the included example rule package
The following call will apply the rule package "data/candler.02.rpk" onto the initial shape geometry "data/candler_lot.obj".

```
bin/prt4cmd \
	-l 3 \
	-p data/candler.02.rpk \
	-a ruleFile:string=bin/candler.01.cgb \
	-a startRule:string=Default\$Lot \
	-e com.esri.prt.codecs.OBJEncoder \
	-z baseName:string=myhouse \
	-g data/candler_lot.obj
```
Upon completion you should find a myhouse_0.obj file inside the install/output folder with corresponding mtl and texture files.

![myhouse62.png](images/myhouse62.png "Building with height=62.0")

#### Generate the building with a different height
By adding the line "-a height:float=30" we change the height of the building from its default value of 62.0 to 30.0:

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

![myhouse30.png](images/myhouse30.png "Building with height=30.0")

## Resources

* https://www.facebook.com/CityEngine
* https://twitter.com/CityEngine

## Issues

Find a bug or want to request a new feature?  Please let us know by submitting an issue.

## Contributing

Anyone and everyone is welcome to contribute. 

## Licensing

Copyright 2013 Esri
