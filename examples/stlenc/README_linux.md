# STLENC - CityEngine SDK Custom STL Encoder

## PREREQUISITES

Before you start working with this example, please make sure you follow the main installation instructions for the CityEngine SDK in the README.md file at the root of this example repository. This will explain how to get the sdk binaries and example data.

## SOFTWARE REQUIREMENTS

* See "General Software Requirements" (Linux)
* Make sure you use the **exact** compiler for PRT extensions

## BUILD INSTRUCTIONS

1. Open a `bash` shell
1. Change into the example directory: `cd <your path to>/cityengine-sdk/examples/stlenc`
1. Create a build directory and change into it: `mkdir build && cd build`
1. Run cmake: `cmake ../src`
1. Compile: `make install`
1. The build result will appear in the `install` directory in parallel to the `build` directory.

## INSTALLATION INSTRUCTIONS FOR CITYENGINE

1. Locate the `stlenc` extension library in the `install` directory above, e.g. at:
   `<your path to>/cityengine-sdk/examples/stlenc/install/lib/libprt_stlenc.so`
1. Copy `libprt_stlenc.so` into `<CityEngine installation location>/plugins/com.esri.prt.clients.ce.gtk.linux.x86_64_1.0.0/lib/`
1. Start CityEngine and verify that the new `STL Encoder` appears in the model export format list.

## LICENSING

The CityEngine SDK is free for personal, educational, and non-commercial use. Commercial use requires at least one commercial license of the latest CityEngine version installed in the organization. Redistribution or web service offerings are not allowed unless expressly permitted.

The CityEngine SDK is licensed under the Esri Terms of Use:

* <https://www.esri.com/en-us/legal/terms/full-master-agreement>
* <https://www.esri.com/en-us/legal/terms/product-specific-scope-of-use>
* All content in the "Examples" directory/section is licensed under the APACHE 2.0 license. You may obtain a copy of this license at <https://www.apache.org/licenses/LICENSE-2.0>.
* For questions or enquiries regarding licensing, please contact the CityEngine team at cityengine-info@esri.com.
