# CityEngine SDK

This is the official site for the SDK of [CityEngine](https://www.esri.com/en-us/arcgis/products/esri-cityengine/overview), a 3D city modeling software for urban design, visual effects, and VR/AR production.

The CityEngine SDK can be used for the development of:

1. *Custom importers and exporters for CityEngine.*

   This means the SDK enables you to develop CityEngine plugins to read or write additional 3D and image formats or your own proprietary 3D data format. A simple use case example is 3D printing where the STL geometry format is often needed. STL support is not provided out-of-the-box in CityEngine, but you can develop your own STL exporter as described below.

2. *3D apps which need a procedural geometry engine.*

   The core of CityEngine is its unique geometry generation engine, called Procedural Runtime (PRT). PRT takes as input an initial geometry and then applies a given rule package (= CGA rules authored in CityEngine) to generate more detailed 3D geometry as output. For example, PRT can generate - based on given rules - a 3D model of a building out of a parcel polygon. With the SDK you can integrate PRT in your own 3D applications taking full advantage of the procedural geometry generation without running CityEngine. An interesting use case example is [Palladio](https://github.com/esri/palladio), a plugin for SideFX’s Houdini software. Palladio includes PRT and therefore extends Houdini with the procedural geometry engine of CityEngine. Another use case example could be a specific cultural heritage 3D application which automatically generates detailed 3D models of temples based on input attributes.

## Preamble

This document explains how to install the CityEngine SDK and how to work with the source-code examples contained in this repository. The CityEngine SDK is packaged into three parts:

* This git repository with the example sources.
* An archive per platform with the SDK binaries and documentation (whitepaper and API reference). The archives are attached to the repo release tags, see links below.
* An archive with example data (e.g. rule packages and initial shapes).

The archives are available for download at the corresponding github release page. See installation instructions below for details.

## CityEngine SDK Releases

* [v2.4.7316 (2021-05-27, CityEngine 2021.0)](https://github.com/Esri/cityengine-sdk/releases/tag/2.4.7316)
* [v2.3.6821 (2020-10-22, CityEngine 2020.1)](https://github.com/Esri/cityengine-sdk/releases/tag/2.3.6821)
* [v2.2.6332 (2020-06-09, CityEngine 2020.0)](https://github.com/Esri/cityengine-sdk/releases/tag/2.2.6332)
* [v2.1.5705 (2019-12-11, CityEngine 2019.1)](https://github.com/Esri/cityengine-sdk/releases/tag/2.1.5705)
* v2.1.5704 (2019-09-25, CityEngine 2019.1, replaced by v2.1.5705 with minor documentation fix)
* [v2.0.5403 (2019-05-08, CityEngine 2019.0)](https://github.com/Esri/cityengine-sdk/releases/tag/2.0.5403)
* [v1.10.4198 (2018-09-17, CityEngine 2018.1)](https://github.com/Esri/cityengine-sdk/releases/tag/1.10.4198)
* [v1.10.4051 (2018-05-11, CityEngine 2018.0)](https://github.com/Esri/cityengine-sdk/releases/tag/1.10.4051)
* [v1.9.3786 (2017-11-06, CityEngine 2017.1)](https://github.com/Esri/cityengine-sdk/releases/tag/1.9.3786)
* [v1.8.3501 (2017-06-29, CityEngine 2017.0)](https://github.com/Esri/cityengine-sdk/releases/tag/1.8.3501)
* [v1.7.2915 (2016-10-03, CityEngine 2016.1)](https://github.com/Esri/cityengine-sdk/releases/tag/1.7.2915)
* [v1.6.2663 (2016-06-21, CityEngine 2016.0)](https://github.com/Esri/cityengine-sdk/releases/tag/1.6.2663)
* [v1.4.2074 (2015-10-06, CityEngine 2015.2)](https://github.com/Esri/cityengine-sdk/releases/tag/1.4.2074)
* [v1.3.1969 (2015-06-17, CityEngine 2015.1)](https://github.com/Esri/cityengine-sdk/releases/tag/1.3.1969)
* [v1.3.1888 (2015-03-31, CityEngine 2015.0)](https://github.com/Esri/cityengine-sdk/releases/tag/1.3.1888)
* [v1.2.1591 (2014-09-01, CityEngine 2014.1)](https://github.com/Esri/cityengine-sdk/releases/tag/1.2.1591)
* [v1.1.1471 (2014-05-29, CityEngine 2014.0)](https://github.com/Esri/cityengine-sdk/releases/tag/1.1.1471)
* v1.0.1209 (2014-01-15, CityEngine 2013.1)

## Installation Instructions

1. Clone or download this repository
1. For the prt4cmd example download the SDK example data archive from the release page and unzip into the cloned repository into a "data" directory.

The final directory layout should look like this:

```text
/cityengine-sdk/
    data/...
    examples/...
```

## Examples

This repository contains a number of source code examples in the "examples" directory. Each example contains a README with detailed instructions how to build and use it:

* prt4cmd: a simple command line utility to apply rule packages onto initial shapes and generate models.
* stlenc: demonstrates how to write a custom encoder, in this case for the STL geometry format.
* stldec: demonstrates how to write a custom decoder for the STL geometry format.

## Related Projects

* [PyPRT](https://esri.github.io/cityengine/pyprt): Python language bindings for the Procedural Runtime (PRT)
* [Palladio](https://esri.github.io/cityengine/palladio): CityEngine plugin for SideFX Houdini
* [Serlio](https://esri.github.io/cityengine/serlio): CityEngine plugin for Autodesk Maya
* [Vitruvio](https://esri.github.io/cityengine/vitruvio): CityEngine plugin for Epic Unreal Engine

## General Software Requirements

Please note that the individual example READMEs may include further requirements.

### All Platforms

* To load custom encoders built with the latest SDK, CityEngine 2021.1 is required. For older versions of CityEngine an older version of the SDK might have to be used, see list above.
* License for matching CityEngine, e.g. to author Rule Packages.
* CMake 3.3 or later (<https://www.cmake.org>)

### Windows

* Windows 8.1 or 10 (64bit)
* Required C++ compiler: Visual Studio 2019 with Toolset MSVC 14.27 or later
* Required flags for extension libraries release mode: `/std:c++17 /bigobj /GR /EHsc /MD` (use `/MDd` to debug)

### Linux

* RedHat Enterprise Linux 7.x or compatible
* Required C++ compiler: GCC 9.3 or later (RedHat Enterprise Linux DevToolSet 9)
* Required flags for extension libraries: `-std=c++17 -D_GLIBCXX_USE_CXX11_ABI=0 -march=nocona -fvisibility=hidden -fvisibility-inlines-hidden -Wl,--exclude-libs,ALL`

## Further Resources

* Documentation
  * [Changelog](changelog.md)
  * Online reference: <https://esri.github.io/cityengine-sdk>
  * Offline reference: see /doc subdirectory inside the release archive
* Community
  * Forum: <https://geonet.esri.com/community/gis/3d/cityengine>
  * Youtube: <https://www.youtube.com/c/CityEngineTV>
  * Facebook: <https://www.facebook.com/CityEngine>
  * Twitter: <https://twitter.com/CityEngine>

## Issues

Did you find a bug or do you want to request a new feature? Please let us know by submitting an issue.

## Contributing

Anyone and everyone is welcome to contribute and to extend and improve the examples by sending us pull requests.

## Licensing

The CityEngine SDK is free for personal, educational, and non-commercial use. Commercial use requires at least one commercial license of the latest CityEngine version installed in the organization. Redistribution or web service offerings are not allowed unless expressly permitted.

The CityEngine SDK is licensed under the Esri Terms of Use:

* <https://www.esri.com/en-us/legal/terms/full-master-agreement>
* <https://www.esri.com/en-us/legal/terms/product-specific-scope-of-use>
* All content in the "Examples" directory/section is licensed under the APACHE 2.0 license. You may obtain a copy of this license at <https://www.apache.org/licenses/LICENSE-2.0>.
* For questions or enquiries regarding licensing, please contact the CityEngine team at cityengine-info@esri.com.
