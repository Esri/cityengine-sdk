# Esri CityEngine SDK Examples

Esri CityEngine is based on Esri Procedural Runtime, which is the underlying engine that supports two geoprocessing tools in ArcGIS and drives procedural symbology in ArcGIS Pro. The CityEngine SDK enables you as a 3rd party developer to extend CityEngine with additional import and export formats. Moreover, you can integrate the procedural runtime in your own client applications taking full advantage of the procedural core without running CityEngine or ArcGIS.

## Preamble

This document explains how to install the CityEngine SDK and how to work with the source-code examples contained in this repository. The Esri CityEngine SDK is packaged into three parts:
- This git repository with the example sources.
- An archive with the SDK binaries and documentation (whitepaper and API reference).
- An archive with example data (e.g. rule packages and initial shapes).

The archives are available for download at the corresponding github release page. See installation instructions below for details.

## CityEngine SDK Releases
- [v1.9.XXXX (2017-10-XX, CityEngine 2017.1)](https://github.com/Esri/esri-cityengine-sdk/releases/tag/1.9.XXXX)
- [v1.8.3501 (2017-06-29, CityEngine 2017.0)](https://github.com/Esri/esri-cityengine-sdk/releases/tag/1.8.3501)
- [v1.7.2915 (2016-10-03, CityEngine 2016.1)](https://github.com/Esri/esri-cityengine-sdk/releases/tag/1.7.2915)
- [v1.6.2663 (2016-06-21, CityEngine 2016.0)](https://github.com/Esri/esri-cityengine-sdk/releases/tag/1.6.2663)
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
- stldec: demonstrates how to write a custom decoder for the STL geometry format.

## General Software Requirements
Please note that the individual example READMEs may include further requirements.

#### All Platforms
* To load custom encoders built with this SDK in CityEngine, CityEngine 2017.0 is required. For older versions of CityEngine an older version of the SDK might have to be used, see list above.
* License for matching CityEngine
* CMake 3.3.0 or later (http://www.cmake.org)

#### Windows
* Windows 7, 8.1 or 10 (64bit)
* NMake 10.0 or later (NMake is part of Visual Studio)
* Recommended C++ Compilers for client code (e.g. `prt4cmd` example): `VC 14 C++ Compiler (Visual Studio 2015) or later`
* To compile SDK extensions (e.g. the `stlenc` example or a custom encoder) it is required to use this **exact** compiler:
    * Microsoft `VC14 C++ Compiler (cl.exe) 19.0.24215.1`, included in `Visual Studio 2015 Update 3`
    * Required flags: `/bigobj /GR /EHsc /MD`

#### MacOSX
* GNU Make 3.82 or later
* Recommended build environment for client code (e.g. the `prt4cmd` example):
    * MacOSX Yosemite (10.11) or later
    * Apple Clang 7.3 or later
* To compile SDK extensions (e.g. the `stlenc` example or a custom encoder) it is required to use this **exact** compiler:
    * MacOSX Yosemite (10.11)
    * Apple Clang 7.3 included in XCode 7.3
    * Required flags: `-std=c++14 -stdlib=libc++ -fvisibility=hidden -fvisibility-inlines-hidden -Wl,--exclude-libs,ALL`

#### Linux
* RedHat Enterprise Linux 6.x/7.x or compatible (64bit only)
* GNU Make 3.82 or later
* Recommended C++ Compilers for client code (e.g. the `prt4cmd` example):
    * GCC 4.8.2 or later
* To compile SDK extensions (e.g. the `stlenc` example or a custom encoder) it is required to use this **exact** compiler:
    * RedHat Enterprise Linux 6.x or compatible
    * GCC 4.8.2 included in RedHat Enterprise Linux 6 DevToolSet 2.1
    * Required flags: `-std=c++11 -march=nocona -fvisibility=hidden -fvisibility-inlines-hidden -Wl,--exclude-libs,ALL`

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

Copyright (c) 2017 Esri

You may not use the content of this repository except in compliance with the following Licenses:
  1. All content of all directories **except "examples"** is licensed under the CityEngine EULA, see [license/EULA.pdf](license/EULA.pdf).
  1. All content in the "examples" directory tree is licensed under the APACHE 2.0 license. You may obtain a copy of this license at http://www.apache.org/licenses/LICENSE-2.0. A copy of the license is also available in the repository at [license/APACHE-LICENSE-2.0.txt](license/APACHE-LICENSE-2.0.txt).
  1. The example data is licensed by the "ESRI Attribution and Licensing Agreement".
