# Esri CityEngine SDK Examples

The Esri CityEngine is based on the procedural runtime, which is the underlying engine that supports two GP tools in ArcGIS 10.x and drives procedural symbology in the release of ArcGIS Professional. The CityEngine SDK enables you as a 3rd party developer to extend CityEngine with additional import and export formats and storage backends beyond simple files. Moreover, you can integrate the procedural runtime in your own client applications taking full advantage of the procedural core without running CityEngine or ArcGIS.

## Preamble

This document explains how to install the CityEngine SDK and how to work with the source-code examples contained in this repository. The Esri CityEngine SDK is packaged into three parts:
- An archive (esri_ce_sdk_[xyz].zip) with the SDK binaries and documentation (whitepaper and API reference).
- This git repository with the example sources.
- An archive (esri_ce_sdk_example_data.zip) with example data (rule packages and initial shapes).

The archives are available for download at the corresponding github release page. See installation instructions below for details.

## CityEngine SDK Releases
- v1.1.1407 [TODO: put link to release page]
- v1.0.1209

## Installation Instructions
1. Clone or download this repository
2. Download the SDK binary archive from the release page (TODO: link)
3. Donwload the SDK example data archive from the release page (TODO: link)
4. Unzip the archives into the cloned repository (the examples will work out of the box in this way).

## Examples
This repository contains a number of source code examples. Each example contains a README with detailed instructions how to build and use it.

A quick overview:
- prt4cmd: a simple command line utility to apply rule packages onto initial shapes and generate models.
- prt4maya: wraps the SDK into a plugin for Autodesk Maya
- stlenc: demonstrates how to write a custom encoder, in this case for the STL geometry format.

## General Software Requirements
Please note that the individual example READMEs may include further requirements.

* License for CityEngine 2013 (or later)
* Supported OS: Windows XP and later, MacOSX 10.7 and later, RedHat Enterprise Linux 6.x and compatible
* C++ Compiler:
  * Windows: MSVC 10.0 or later
  * MacOSX: Clang 3.0 or later
  * Linux: GCC 4.4 or later
* (Optional) To compile SDK extensions (e.g. custom encoders) you need to use these **exact** C++ compiler versions:
  * Windows: Microsoft C++ Compiler (cl.exe) 16.00.40219.01 for x64, included in either one of these products (i.e. choose one):
    * Visual Studio 2010 SP1 64bit
    * Windows SDK 7.1 64bit with Visual C++ 2010 Compiler SP1 (Attention: [known issue with SP1](https://connect.microsoft.com/VisualStudio/feedback/details/660584))
  * MacOSX: Apple GCC 4.2.1 included in:
    * XCode 4.2.1 and MacOSX SDK 10.7 (Based on Apple Inc. build 5658, LLVM build 2336.1.00)
  * Linux: GCC 4.4.7 64bit
* Linux/OSX: GNU Make 3.82 or later
* Windows: NMake 10.0 or later (part of Visual Studio [Express])
* CMake 2.8.10 or later (http://www.cmake.org)
* boost 1.41 or later (http://www.boost.org)

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
