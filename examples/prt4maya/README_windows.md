# PRT4MAYA - CityEngine SDK for Autodesk Maya


## DISCLAIMER
This is an example for using the CityEngine SDK, not an official product with support. If you find a bug or are missing a feature, let us know by submitting an issue. Or - even better - submit a pull request.


## PREREQUISITES
Before you start working with this example, please make sure you follow
the main installation instructions for the CityEngine SDK in the
README.md file at the root of this example repository. This will 
explain how to get the sdk binaries and example data.


## SOFTWARE REQUIREMENTS
- See "General Software Requirements" (Windows)
- To build, Autodesk Maya 2018 **or** the development kit is needed (please refer to [Maya documentation](http://help.autodesk.com/view/MAYAUL/2018/ENU/?guid=__files_Setting_up_your_build_environment_htm)).
- To run, Autodesk Maya 2018 is needed.


## BUILD INSTRUCTIONS

### Building with Visual Studio
1. Open a Windows Command Shell in the `prt4maya` root directory, i.e. at `<your path to>\esri-cityengine-sdk\examples\prt4maya`.
2. Create a build directory with `mkdir build` and change into it with `cd build`.
3. Run `cmake` to generate a Visual Studio solution:
   ```
   cmake -G "Visual Studio 15 2017 Win64" ../src
   ```
   Use options `-Dprt_DIR=<ce sdk root>\cmake` and `-Dmaya_DIR=<maya installation root>` to override the default locations of CityEngine SDK and Maya.
1. Open the generated `prt4maya_parent.sln` in Visual Studio.
2. Switch the solution configuration to "Release" or "RelWithDebInfo" ("Debug" is not supported with release CE SDK).
3. Call `build` on the `INSTALL` project.
1. Proceed with the Installation Instructions below.

### Building on the Command Line
1. Open a Visual Studio 2017 x64 Command Shell in the `prt4maya` root directory, i.e. at `<your path to>\esri-cityengine-sdk\examples\prt4maya`.
2. Create a build directory with `mkdir build` and change into it with `cd build`.
3. Run `cmake` to generate the Makefiles:
   ```
   cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=RelWithDebInfo ../src
   ```
   Use options `-Dprt_DIR=<ce sdk root>\cmake` and `-Dmaya_DIR=<maya installation root>` to override the default locations of CityEngine SDK and Maya.
4. Run make to build the desired target, e.g.
   ```
   make install
   ```
5. The build result will appear in the `install` directory in parallel to the `build` directory. We will use this as the plugin directory below.
1. Proceed with the Installation Instructions below.


## INSTALLATION INSTRUCTIONS
1. Locate the directory where you copied the plugin (or keep `<your path to>\esri-cityengine-sdk\examples\prt4maya\install`), let's call it `PLUGINDIR`
1. Locate the Maya.env file in your home, usually its in `<home directory>\Documents\maya\2018`
1. Edit Maya.env as follows:
   ```
   :: replace <PLUGINDIR> with the actual path
   PATH=<PLUGINDIR>\plug-ins;%PATH%
   MAYA_PLUG_IN_PATH=<PLUGINDIR>\plug-ins
   MAYA_SCRIPT_PATH=<PLUGINDIR>\scripts
   ```
1. Start maya
1. Open the plugin manager: Windows -> Settings/Preferences -> Plug-in Manager
1. Enable `prt4maya.mll`
1. The plugin should load and a new menu item `PRT` should appear in Maya.


## USAGE INSTRUCTIONS
1. Use CityEngine to export a Rule Package (RPK).
1. In Maya, select a mesh and use the PRT menu to assign the RPK. This will run the rules for each face in the mesh.
2. Use the Hypergraph to navigate to the PRT node where you can edit the rule parameter.

## LICENSING

Copyright (c) 2019 Esri

You may not use the content of this repository except in compliance with the following Licenses:
  1. All content of all directories **except "examples"** is licensed under the CityEngine EULA, see [EULA.pdf](EULA.pdf).
  2. All content in the "examples" directory tree is licensed under the APACHE 2.0 license. You may obtain a copy of this license at http://www.apache.org/licenses/LICENSE-2.0.
