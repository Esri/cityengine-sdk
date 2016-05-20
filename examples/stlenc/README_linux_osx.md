STLENC - CityEngine SDK Custom STL Encoder
------------------------------------------


PREREQUISITES
-------------

Before you start working with this example, please make sure you follow
the main installation instructions for the CityEngine SDK in the
README.md file at the root of this example repository. This will 
explain how to get the sdk binaries and example data.


SOFTWARE REQUIREMENTS
---------------------

- see "General Software Requirements - Linux or MacOSX"
- GCC 4.8.2 from RedHat Enterprise Linux 6 DevToolSet 2.1


BUILD INSTRUCTIONS
------------------

1. Open a `bash` shell
1. Change into the example directory: `cd <path to esri-cityengine-sdk git repository>/examples/stlenc`
1. Create a build directory and change into it: `mkdir build && cd build`
1. Run cmake: `cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release ../src`
1. Compile: `make install`
1. The build result will appear in the `install` directory in parallel to the `build` directory.


INSTALLATION INSTRUCTIONS FOR CITYENGINE
----------------------------------------

1. Locate the `stlenc` extension library in the `install` directory above, e.g. at:
   `<path to esri-cityengine-sdk git repository>/examples/stlenc/install/lib/libstlenc.so`
1. Copy `libstlenc.so` into `<CityEngine installation location>/plugins/com.esri.prt.clients.ce.gtk.linux.x86_64_1.0.0/lib/`
1. Start CityEngine and verify that the new `STL Encoder` appears in the model export format list.


LICENSING
---------

Copyright (c) 2016 Esri

You may not use the content of this repository except in compliance with the following Licenses:
  1. All content of all directories **except "examples"** is licensed under the CityEngine EULA, see [license/EULA.pdf](license/EULA.pdf).
  2. All content in the "examples" directory tree is licensed under the APACHE 2.0 license. You may obtain a copy of this license at http://www.apache.org/licenses/LICENSE-2.0. A copy of the license is also available in the repository at [license/APACHE-LICENSE-2.0.txt](license/APACHE-LICENSE-2.0.txt).
