# Esri CityEngine SDK Examples

The Esri CityEngine is based on the procedural runtime, which is the underlying engine that supports two geoprocessing tools in ArcGIS and drives procedural symbology in ArcGIS Pro. The CityEngine SDK enables you as a 3rd party developer to extend CityEngine with additional import and export formats. Moreover, you can integrate the procedural runtime in your own client applications taking full advantage of the procedural core without running CityEngine or ArcGIS.

## Preamble

This document explains how to install the CityEngine SDK and how to work with the source-code examples contained in this repository. The Esri CityEngine SDK is packaged into three parts:
- This git repository with the example sources.
- An archive with the SDK binaries and documentation (whitepaper and API reference).
- An archive with example data (e.g. rule packages and initial shapes).

The archives are available for download at the corresponding github release page. See installation instructions below for details.

## CityEngine SDK Releases
- [v1.4.2074 (2015-10-06, CityEngine 2015.2)](https://github.com/Esri/esri-cityengine-sdk/releases/tag/1.4.2074)
- [v1.3.1969 (2015-06-17, CityEngine 2015.1)](https://github.com/Esri/esri-cityengine-sdk/releases/tag/1.3.1969)
- [v1.3.1888 (2015-03-31, CityEngine 2015.0)](https://github.com/Esri/esri-cityengine-sdk/releases/tag/1.3.1888)
- [v1.2.1591 (2014-09-01, CityEngine 2014.1)](https://github.com/Esri/esri-cityengine-sdk/releases/tag/1.2.1591)
- [v1.1.1471 (2014-05-29, CityEngine 2014.0)](https://github.com/Esri/esri-cityengine-sdk/releases/tag/1.1.1471)
- v1.0.1209 (2014-01-15, CityEngine 2013.1)

## Installation Instructions
1. Clone or download this repository
2. Download the SDK binary archive from the release page
3. Download the SDK example data archive from the release page
4. Unzip the archives into the cloned repository into a "prt" and "data" directory.

The final directory layout should look like this:
```
/esri-cityengine-sdk/
    examples/...
    license/...
    data/...
    prt/
        bin/...
        cmake/...
        doc/...
        include/...
        lib/...
```

## Examples
This repository contains a number of source code examples. Each example contains a README with detailed instructions how to build and use it.

A quick overview:
- prt4cmd: a simple command line utility to apply rule packages onto initial shapes and generate models.
- prt4maya: wraps the SDK into a plugin for Autodesk Maya
- stlenc: demonstrates how to write a custom encoder, in this case for the STL geometry format.

## General Software Requirements
Please note that the individual example READMEs may include further requirements.

#### All Platforms
* To load custom encoders built with this SDK in CityEngine, CityEngine 2015.2 is required. For older versions of CityEngine an older version of the SDK might have to be used, see table above. 
* License for matching CityEngine
* CMake 3.0.0 or later (http://www.cmake.org)
* Optionally for custom SDK extension: boost 1.59 headers and binaries (using the same compiler as specified below)
    * Get the boost 1.59 headers from http://www.boost.org

#### Windows
* Windows 7 or later (32bit or 64bit)
* NMake 10.0 or later (part of Visual Studio)
* Recommended C++ Compilers for simple clients (prt4cmd example):
    * VC10 or later
* To compile SDK extensions (e.g. the stlenc example or a custom encoder) for **use in CityEngine 2014.0 up to 2015.2**, it is required to use this exact compiler:
    * Microsoft VC10 C++ Compiler (cl.exe) 16.00.40219.01, included in Visual Studio 2010 SP1
* Otherwise (e.g. the prt4maya example), also the VC11 compiler is supported:
    * Microsoft VC11 C++ Compiler (cl.exe) 17.00.61030.0, included in Visual Studio 2012 SP4
* Pre-compiled boost libraries for VC10 or VC11 can be obtained from http://boost.teeks99.com (e.g. boost_1_53_0-vc64-bin.exe or boost_1_53_0-vc32-bin.exe)

#### MacOSX
* To compile: MacOSX Yosemite (10.10)
* To run: MacOSX Yosemite (10.10) or later
* GNU Make 3.82 or later
* Recommended C++ Compilers for simple clients (prt4cmd example):
    * Apple CLANG 6.1 or later
* To compile SDK extensions (e.g. prt4maya example or the stlenc example for use in CityEngine) it is required to use this **exact** C++ compiler version:
    * Apple CLANG 6.1 included in XCode 6.4

#### Linux
* RedHat Enterprise Linux 6.x or compatible (64bit only)
* GNU Make 3.82 or later
* Recommended C++ Compilers for simple clients (prt4cmd example):
    * GCC 4.8.2 or later
* To compile SDK extensions (e.g. prt4maya example or the stlenc example for use in CityEngine) it is required to use this **exact** C++ compiler version:
    * GCC 4.8.2 included in RedHat Enterprise Linux 6 DevToolSet 2.1

## Further Resources
* Documentation:
    * [Changelog](changelog.md)
    * Online reference: https://esri.github.io/esri-cityengine-sdk
    * Offline reference: see /doc subdirectory inside the release archive
* Forum: 
    * General CityEngine: https://geonet.esri.com/community/gis/3d/cityengine
* Facebook: https://www.facebook.com/CityEngine
* Twitter: https://twitter.com/CityEngine

## Issues

Find a bug or want to request a new feature? Please let us know by submitting an issue.

## Contributing

Anyone and everyone is welcome to contribute and to extend and improve the examples by sending us pull requests.

## Licensing

Copyright 2015 Esri

You may not use the content of this repository except in compliance with the following Licenses:
  1. All content of all directories **except "examples"** is licensed under the CityEngine EULA, see [license/EULA.pdf](license/EULA.pdf).
  2. All content in the "examples" directory tree is licensed under the APACHE 2.0 license. You may obtain a copy of this license at http://www.apache.org/licenses/LICENSE-2.0. A copy of the license is also available in the repository at [license/APACHE-LICENSE-2.0.txt](license/APACHE-LICENSE-2.0.txt).

[](Esri Tags: CityEngine)
[](Esri Language: C++)
