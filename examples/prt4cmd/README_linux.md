# prt4cmd - CityEngine SDK Command Line Utility

## Disclaimer

This is an example for using the CityEngine SDK. There is no support for this application. If you find a bug or want to enhance functionality you have to do this yourself.

## Prerequisites

Before you start working with this example, please make sure you follow the main installation instructions for the CityEngine SDK in the README.md file at the root of this example repository. This will explain how to get the sdk binaries and example data.

## Software Requirements

* See "General Software Requirements" (Linux)

## Build Instructions

1. Open a terminal (e.g. bash)
1. Change into the example directory: `cd <your path to>/cityengine-sdk/examples/prt4cmd`
1. Prepare build directory: `mkdir build && cd build`
1. Run cmake: `cmake ../src`
1. Compile: `make install`
1. The build result will appear in a `install` directory in parallel to the `build` directory.

## Using prt4cmd

1. Open a bash shell and `cd` in to the above `install` directory
1. Type `bin/prt4cmd -h` to see the list of options.
1. Type the following to generate the "candler" building model in the wavefront obj format:

   ```text
   bin/prt4cmd -l 3 -g ../../../data/candler_footprint.obj -p ../../../data/candler.rpk -e com.esri.prt.codecs.OBJEncoder -z baseName:string=theCandler
   ```

1. The result is placed in a new `output` directory inside the `install` directory:
    * `theCandler_0.obj`
    * `theCandler.mtl`
    * Bunch of texture files
1. To regenerate the same model with a different height, use the following command (note the additional `-a BuildingHeight...` statement):

   ```text
   bin/prt4cmd -l 3 -g ../../../data/candler_footprint.obj -p ../../../data/candler.rpk -a BuildingHeight:float=30 -e com.esri.prt.codecs.OBJEncoder -z baseName:string=theCandler
   ```

1. The obj file will now contain a model which is 30m high instead of the default 62m.

## Licensing

The CityEngine SDK is free for personal, educational, and non-commercial use. Commercial use requires at least one commercial license of the latest CityEngine version installed in the organization. Redistribution or web service offerings are not allowed unless expressly permitted.

The CityEngine SDK is licensed under the Esri Terms of Use:

* <https://www.esri.com/en-us/legal/terms/full-master-agreement>
* <https://www.esri.com/en-us/legal/terms/product-specific-scope-of-use>
* All content in the "Examples" directory/section is licensed under the APACHE 2.0 license. You may obtain a copy of this license at <https://www.apache.org/licenses/LICENSE-2.0>.
* For questions or enquiries regarding licensing, please contact the CityEngine team at cityengine-info@esri.com.
