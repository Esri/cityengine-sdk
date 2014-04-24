# Esri CityEngine SDK Examples

The Esri CityEngine is based on the procedural runtime, which is the underlying engine that supports two GP tools in ArcGIS 10.x and drives procedural symbology in the release of ArcGIS Professional. The CityEngine SDK enables you as a 3rd party developer to extend CityEngine with additional import and export formats and storage backends beyond simple files. Moreover, you can integrate the procedural runtime in your own client applications taking full advantage of the procedural core without running CityEngine or ArcGIS.

## Preamble

This document explains how to install the CityEngine SDK and how to work with the source-code examples contained in this repository. The Esri CityEngine SDK is packaged into three parts:
- This git repository with the example sources.
- An archive with the SDK binaries and documentation (whitepaper and API reference).
- An archive with example data (e.g. rule packages and initial shapes).

The archives are available for download at the corresponding github release page. See installation instructions below for details.

## CityEngine SDK Releases
- [v1.1.1407 (TBA)](https://github.com/Esri/esri-cityengine-sdk/releases/tag/1.1.1407)
- [v1.0.1209 (2014-01-15)](https://github.com/Esri/esri-cityengine-sdk/releases/tag/1.0.1209)

## Installation Instructions
1. Clone or download this repository
2. Download the SDK binary archive from the release page
3. Donwload the SDK example data archive from the release page
4. Unzip the archives into the cloned repository (the examples will work out of the box in this way).

## Examples
This repository contains a number of source code examples. Each example contains a README with detailed instructions how to build and use it.

A quick overview:
- prt4cmd: a simple command line utility to apply rule packages onto initial shapes and generate models.
- prt4maya: wraps the SDK into a plugin for Autodesk Maya
- stlenc: demonstrates how to write a custom encoder, in this case for the STL geometry format.

## General Software Requirements
Please note that the individual example READMEs may include further requirements.

#### Windows
* Windows XP SP1 or later (32bit and 64bit)
* License for CityEngine 2013 or later
* Recommended C++ Compilers
    * Microsoft VC 10.0
    * Microsoft VC 11.0
* (Optional) To compile SDK extensions (e.g. custom encoders) it is required to use these **exact** C++ compiler versions:
    * Microsoft C++ Compiler (cl.exe) 16.00.40219.01 for x64, included in either one of these products:
        * Visual Studio 2010 SP1 64bit
        * Windows SDK 7.1 64bit with Visual C++ 2010 Compiler SP1 (Attention: [known issue with SP1](https://connect.microsoft.com/VisualStudio/feedback/details/660584))
* NMake 10.0 or later (part of Visual Studio [Express])
* CMake 2.8.10 or later (http://www.cmake.org)
* boost 1.53 binaries 
  * boost libraries need to be compiled with your compiler (VC 10 or VC 11) for your architecture
  * download pre-built libraries from boost.teeks99.com (e.g. boost_1_53_0-vc64-bin.exe or boost_1_52_0-vc32-bin.exe)
  * or get just the source and build it  yourself: http://www.boost.org)

#### MacOSX
* MacOSX 10.7 or later (64bit only)
* License for CityEngine 2013 or later
* Recommended C++ Compilers
    * GCC 4.2.1 or later
    * Clang 3.0 or later
* (Optional) To compile SDK extensions (e.g. custom encoders) it is required to use these **exact** C++ compiler versions:
    * Apple GCC 4.2.1 included in:
        * XCode 4.2 and MacOSX SDK 10.7 (Based on Apple Inc. build 5658, LLVM build 2336.1.00)
* GNU Make 3.82 or later
* CMake 2.8.10 or later (http://www.cmake.org)
* boost 1.53 (http://www.boost.org)

#### Linux
* RedHat Enterprise Linux 6.x or compatible (64bit only)
* License for CityEngine 2013 or later
* Recommended C++ Compilers
    * GCC 4.4 or later
* (Optional) To compile SDK extensions (e.g. custom encoders) it is required to use these **exact** C++ compiler versions:
    * GCC 4.4.7 64bit included in:
        * RedHat Enterprise Linux 6.4 Package Repository
* GNU Make 3.82 or later
* CMake 2.8.10 or later (http://www.cmake.org)
* boost 1.53 (http://www.boost.org)

## Further Resources
* Forum: 
    * General CityEngine: http://forums.arcgis.com/forums/204-CityEngine-General-Discussion
    * CityEngine SDK: http://forums.arcgis.com/forums/270-CityEngine-SDK
* Facebook: https://www.facebook.com/CityEngine
* Twitter: https://twitter.com/CityEngine

## Issues

Find a bug or want to request a new feature?  Please let us know by submitting an issue.

## Contributing

Anyone and everyone is welcome to contribute and to extend and improve the examples by sending us pull requests.

## Licensing

Copyright 2014 Esri

You may not use the content of this repository except in compliance with the following Licenses:
  1. All content of all directories **except "examples"** is licensed under the CityEngine EULA, see [license/EULA.pdf](license/EULA.pdf).
  2. All content in the "examples" directory tree is licensed under the APACHE 2.0 license. You may obtain a copy of this license at http://www.apache.org/licenses/LICENSE-2.0. A copy of the license is also available in the repository at [license/APACHE-LICENSE-2.0.txt](license/APACHE-LICENSE-2.0.txt).

[](Esri Tags: CityEngine)
[](Esri Language: C++)
