PRT4MAYA - CityEngine SDK for Autodesk Maya
-------------------------------------------


DISCLAIMER
----------

This is an example for using the CityEngine SDK, not an official product with support.
If you find a bug or are missing a feature, let us know by submitting an issue. Or - even better - submit a pull request.


PREREQUISITES
-------------

Before you start working with this example, please make sure you follow
the main installation instructions for the CityEngine SDK in the
README.md file at the root of this example repository. This will 
explain how to get the sdk binaries and example data.


SOFTWARE REQUIREMENTS
---------------------

- See "General Software Requirements" (Linux or macOS)
- Autodesk Maya 2016 with development kit (devkit)


BUILD INSTRUCTIONS
------------------

1. Open a terminal (e.g. bash)
1. Change into the example directory: `cd <your path to>/esri-cityengine-sdk/examples/prt4maya`
1. Create a build directory and change into it: `mkdir build && cd build`
1. Run cmake (please adjust the maya path): `cmake -Dmaya_DIR=/usr/autodesk/maya2016 ../src`
1. Compile: `make install`
1. The build result will appear in the `install` directory in parallel to the `build` directory. We will use this as the plugin directory below.


INSTALLATION INSTRUCTIONS
-------------------------

1. Locate the absolute path to the `install` directory created above (or keep `<your path to>/esri-cityengine-sdk/examples/prt4maya/install`), let's call it `PLUGINDIR`
1. Locate the Maya.env file in your home: `~/maya/2016/Maya.env`
1. Edit Maya.env as follows:
   ```
   ESRI_CE_SDK_LIC_FEATURE=CityEngAdvFx            # use CityEngAdv for floating license
   # ESRI_CE_SDK_LIC_HOST=27000@my.flexnet.host    # uncomment for floating license
   
   PLUGINDIR=<PLUGINDIR> # replace <PLUGINDIR> with the actual path
   MAYA_PLUG_IN_PATH=$PLUGINDIR/plug-ins
   MAYA_SCRIPT_PATH=$PLUGINDIR/scripts
   ```
1. Start maya (note: you may want to start maya from a shell to see the prt4maya log output)
1. Open the plugin manager: Windows -> Settings/Preferences -> Plug-in Manager
1. Enable `libprt4maya.so`
1. The plugin should load and a new menu item `PRT` should appear in Maya.


LICENSING
---------

Copyright (c) 2017 Esri

You may not use the content of this repository except in compliance with the following Licenses:
  1. All content of all directories **except "examples"** is licensed under the CityEngine EULA, see [license/EULA.pdf](license/EULA.pdf).
  2. All content in the "examples" directory tree is licensed under the APACHE 2.0 license. You may obtain a copy of this license at http://www.apache.org/licenses/LICENSE-2.0. A copy of the license is also available in the repository at [license/APACHE-LICENSE-2.0.txt](license/APACHE-LICENSE-2.0.txt).
