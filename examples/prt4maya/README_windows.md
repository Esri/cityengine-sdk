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

- See "General Software Requirements - Windows, to compile SDK extensions"
- Visual Studio 2012 (in addition to Visual Studio 2013)
- Autodesk Maya 2016 **including the development kit** (devkit)


BUILD INSTRUCTIONS
------------------

1. Open the standard windows shell (**not** the Visual Studio Shell)
1. `cd <your path to>\esri-cityengine-sdk\examples\prt4maya`
1. `build.bat ..\..\prt\cmake C:\Autodesk\Maya2016`
1. The build result will appear in the `install` directory in parallel to the `build` directory. We will use this as the plugin directory below.


INSTALLATION INSTRUCTIONS
-------------------------

1. Locate the directory where you copied the plugin (or keep `<your path to>\esri-cityengine-sdk\examples\prt4maya\install`), let's call it `PLUGINDIR`
1. Locate the Maya.env file in your home, usually its in "<home directory>\Documents\maya\2016"
1. Edit Maya.env as follows:
   ```
   :: use CityEngAdv for floating license
   ESRI_CE_SDK_LIC_FEATURE=CityEngAdvFx
   
   :: uncomment for floating license
   :: ESRI_CE_SDK_LIC_HOST=27000@my.flexnet.host

   :: replace <PLUGINDIR> with the actual path
   PATH=<PLUGINDIR>\plug-ins;%PATH%
   MAYA_PLUG_IN_PATH=<PLUGINDIR>\plug-ins
   MAYA_SCRIPT_PATH=<PLUGINDIR>\scripts
   ```
1. Start maya
1. Open the plugin managaer: Windows -> Settings/Preferences -> Plug-in Manager
1. Enable `prt4maya.mll`
1. The plugin should load and a new menu item `PRT` should appear in Maya.


LICENSING
---------

Copyright (c) 2016 Esri

You may not use the content of this repository except in compliance with the following Licenses:
  1. All content of all directories **except "examples"** is licensed under the CityEngine EULA, see [license/EULA.pdf](license/EULA.pdf).
  2. All content in the "examples" directory tree is licensed under the APACHE 2.0 license. You may obtain a copy of this license at http://www.apache.org/licenses/LICENSE-2.0. A copy of the license is also available in the repository at [license/APACHE-LICENSE-2.0.txt](license/APACHE-LICENSE-2.0.txt).
