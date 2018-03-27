ESRI CITYENGINE SDK 1.10.YYYY CHANGELOG
=======================================

This section lists changes compared to CityEngine SDK 1.9.3786.


General Info
------------
* CityEngine SDK 1.10.YYYY is used in CityEngine 2018.0 (2018.0.ZZZZ).


PRT API
-------
* Fixed the XML representation of prt::AttributeMapBuilder.
* Fixed missing enum items in API reference (e.g. prt::LogLevel). [CE-5326]


PRTX API
--------
* Fixed wrong prtx::Mesh::getUVSetsCount() return value.
* Fixed wrong prtx::MeshBuilder error message if uv coordinates and indices are out of sync.
* Updated prtx::NamePreparator to filter out invalid characters in URIs (e.g. for file name creation). [CE-5276]
* Reduced header inclusion and introduced a few forward-declarations. [CE-5382]


CGA
---
*  Behavior change in "random" evaluation: Fixed a bug in the evaluation of attr/const functions where the random generator could produce unexpected values if attrs/const functions are mutual dependent.
* New operation: "insertAlongUV"
* New function: "assetNamingInfo"
* Changes to existing functions:
  * acos, asin and sqrt: A warning is issued if the parameter is outside of the valid range.
* Bugfixes:
  * comp operation:
    * Re-evaluate random generator for each group (g) and material (m) component. Before this fix rand returned the same value for each component.
    * Group components (g): Fixed wrong per-face materials for meshes with holes.
  * trim and i operation: Trim planes now respect holes in assets.
  * trim, i and split operation: Trim planes and splits now respect per-face materials.
  * split operation: Fixed missing filling face when faces with near zero area were present at the split position.


Built-In Codecs Changes and Fixes
---------------------------------
* Collada Decoder
  * Fixed a bug where multiple UV coordinate sets could be wrongly imported. [CE-3389]
* Unreal Encoder
  * Updated to Epic Unreal Studio / Datasmith SDK 0.18 (corresponding to Unreal Engine 4.19).
  * Introduced two new encoder options to control mesh granularity and instancing behavior. [CE-5306]
  * Fixed a bug where the Datasmith NormalMap texture mode was not correctly set. [CE-4997]
  * Fixed a bug where the wrong output path was logged. [CE-5306]
  * Fixed a memory leak when resizing texture to power-of-two. [CE-5276]
  * Fixed a bug where the "maximum number of objects" limit of Unreal could be reached. [CE-5301]
* Texture Encoder
  * Added support for (32bit) GeoTiff metadata. [CE-1771]
* Vue, Alembic, Collada, OBJ Encoders
  * Added stricter validation on object/file names to support non-english locales. [CE-5276]
* I3S Encoder
  * Fixed a bug where encoding multiple objects with identical bounding boxes could lead to a crash. [CE-5057]
  * Fixed a bug where the I3S node level could be set to a wrong value.
* Shape Buffer Encoder
  * Added an encoder option to only export CGA report sums instead of all values. [CE-4673]
* Vue Encoder
  * Fixed a bug where not all encoded textures are correctly logged. [CE-5224]


Misc Changes and Fixes
----------------------
* OS and Compiler Ugrades
  * Linux: Switched to RHEL 7 and GCC 6.3 with C++14
  * Windows: Switched to Visual Studio 2017 (MSVC Toolchain 14.11)
  * macOS: Switched to macOS 10.12 and Apple Clang 8.1
* Added Unreal encoder libraries to generated CMake config. [CE-5394]



ESRI CITYENGINE SDK 1.9.3786 CHANGELOG
======================================

This section lists changes compared to CityEngine SDK 1.8.3501


General Info
------------
* CityEngine SDK 1.9.3786 is used in CityEngine 2017.1 (2017.1.3674).


PRT API
-------
* Documentation
  * Added missing namespaces (StringUtils.h).
  * Added link to featured annotations (RuleFileInfo.h). 


PRTX API
--------
* Added cut textures flag to EncodePreparator (EncodePreparator.h).
* New function returns pseudo id from initial shape (EncodePreparator.h).
* Added copy constructor for transformed mesh builder (Mesh.h).
* Removed obsolete includes (EncodePreparator.h).
* Documentation
  * Added missing namespaces (DataBackend.h, AnnotationBuilder.h).
  * Clarified ExtensionManager::addFactory (ExtensionManager.h).

CGA
---
(Please also refer to the CGA ChangeLog for 2017.1 in the [CityEngine Online Help](http://cehelp.esri.com/help/index.jsp?topic=/com.procedural.cityengine.help/html/cgareference/cga_changelog.html) for new operations and bug-fixes.)
* Increased CGA language version to `2017.1`
* New annotations:
  * @Enum annotation.
* New functions:
  * geometry.height function.
* Changes to existing features:
  * cleanupGeometry operation: Merge vertices: Vertices of holes are now merged within tolerance. As a consequence, neighboring holes could be merged to a single hole and small holes could be removed.
  * Prior usage of @Range annotation is deprecated. Instead use new parameterized version of @Range annotation and new @Enum annotation.
  * @Angle, @Distance and @Percent annotations are supported in CityEngine.
* Bugfixes:
  * Occlusion queries: Improved accuracy in cases when the query geometry lies on the surface of an occluder shape.
  * All intra context/occlusion queries: Fixed a bug where queries did not respect occluder/context shapes and therefore returned a wrong value. This happened for labels (including the empty label) which were not encountered in the first evaluation pass (two-pass generation process).
* cleanupGeometry operation:
  * Merge vertices: Prevent creation of small holes and inverted faces.
  * Remove duplicate faces: Correct consideration of holes.
  * Remove shared edges between coplanar faces: Enhanced overall robustness and results for holes.
  * Remove degenerate faces with small area: Applied at the end in order to reduce impact.
* scatter operation: Gaussian standard deviation can be specified in relative coordinates.
* tileUV operation: Fixed creation of invalid UVs in case of near zero area geometry.


Built-In Codecs Changes and Fixes
---------------------------------
* New Encoders
  * Added Unreal encoder.
* I3S/SLPK Encoder
  * Added an option to enable backface culling ("backfaceCulling"). [CE-3656]
  * Added attribute statistics and labels. [CE-4529]
  * Fixed inconsistent feature IDs. [CE-4105, CE-4672]
  * Fixed handling of CGA generation errors. [CE-4398]
* ShapeBuffer Encoder
  * Textures are cropped to used regions. [CE-2602]
* FBX Decoder
  * Set asset URI in FBX decoder (fixes instancing support).


Misc Changes and Fixes
----------------------
* Licensing sub-system has been updated to FNP 11.14. [CE-4676]



ESRI CITYENGINE SDK 1.8.3501 CHANGELOG
======================================

This section lists changes compared to CityEngine SDK 1.7.2915


General Info
------------
* CityEngine SDK 1.8.3501 is used in CityEngine 2017.0 (2017.0.3406).


PRT API
-------
* Added a prt::generate overload with support for multiple occlusion sets. [CE-3791, CE-3994]


PRTX API
--------
* Added support for multiple occlusion sets (GenerateContext.h). [CE-3791]
* Small change to improve const correctness (ExtensionManager.h, GenerateContext.h).


CGA
---
(Please also refer to the CGA ChangeLog for 2017.0 in the [CityEngine Online Help](http://cehelp.esri.com/help/index.jsp?topic=/com.procedural.cityengine.help/html/cgareference/cga_changelog.html) for new operations and bug-fixes.)
* Increased CGA language version to `2017.0`
*  New operations:
  * label operation.
* New functions:
  * New basic functions: min, max and clamp.
  * New functions for color conversion: colorHexToB, colorHexToG, colorHexToH, colorHexToO, colorHexToR, colorHexToS, colorHexToV, colorHSVToHex, colorHSVOToHex, colorRGBToHex and colorRGBOToHex.
  * New functions for context queries to labeled shapes: minimumDistance, contextCompare and contextCount.
* Changes to existing features:
  * primitiveQuad, primitiveDisk, primitiveCube operation: Removed vertex normals because they do not influence shading. Vertex normals can still be created using setNormals or softenNormals operation.
  * comp operation: New component selectors g for groups, m for materials and h for holes.
  * inside, overlaps and touches functions:
    * New optional label identifier for performing occlusion queries on labeled occluder shapes.
    * Geometries only slightly lying outside an occluder shape are considered inside (adjusted threshold behavior due to floating point limitations).
    * Surfaces touch also when normals point in same direction.
  * Support shapes consisting of vertices or edges (coming from comp or scatter operation):
    * geometry.isClosedSurface, geometry.isPlanar, geometry.isRectangular functions: Return false.
    * geometry.nEdges function: Counts edges.      
    * geometry.isOriented function: Consider vertex/edge normal.
* Bugfixes:
  * setback operation: In some cases self-intersecting faces on concave shapes were created. That could lead to a failure of subsequent operations.
  * comp operation:
    * Combined (=) face component split now respects per-face materials.
    * Fixed wrong edge component split for edges with opposing adjacent faces.
    * Fixed wrong result of combined (=) vertex and edge component split.
    * Fixed (deprecated) noStreetSide comp split selector handling.
  * geometry.area and geometry.isOriented function: Corrected scope selectors for negative scope sizes (e.g. front becomes back if scope.sz is < 0).
  * alignScopeToGeometry operation: zUp mode: Fixed wrong pivot for meshes that only consist of vertices (for example from vertex splits or scatter).
  * inside, overlaps and touches functions:
    * Fixed a bug where a closed shape were not touching and not overlapping another geometry that lied inside.
    * Intra-occlusion: Fixed a bug where component split shapes which are children of the current shape were considered for occlusion.
    * Inter-occlusion: Fixed a bug where occlusion-queries were not considered when defined in consts or functions.
    * Fixed a bug that mistakenly returned true in rare situations due to near zero-area triangles.
    * Inter-ccclusion: Fixed a bug that created an occluder shape by a component split or offset operation during the intra ghost tree generation which was then mistakenly considered also for inter occlusion.
    * Fixed behavior in exports.
  * set operation: fixed a bug: trying to set the initialShape.pz attribute resulted in inconsistent behaviour (mismatch with initialShape.pa).
  * alignScopeToAxes, rotateScope, mirrorScope, setPivot, cleanupGeometry, reduceGeometry, convexify, split operations: Correctly consider a non-adjusted scope coming from a split operation.
  * offset operation: Now keeps asset materials and groups.
  * scatter operation: Issue warning for negative or zero number of points.
  * inside, overlaps, touches, geometry.angle functions, alignScopeToGeometry, alignScopeToAxes operations: Fixed for shapes consisting of vertices or edges (coming from comp or scatter operation).
  * assetApproxRatio, assetApproxSize, assetBestRatio, assetBestSize, assetFitSize, fileRandom, imageApproxRatio, imageBestRatio, fileSearch functions: Fixed a bug which led to wrong asset lookups if used in RPKs.


Built-In Codecs Changes and Fixes
---------------------------------
* Multiple Encoders/Decoders
  * Fixed import/export of vertex normals (FBX/Collada). [CE-3407]
  * Fixed matrix inversion for mirror-matrices (fixes several FBX/Collada import issues). [CE-3970]
  * Correctly import material names (FBX/Collada). [CE-2352]
* I3S/SLPK Encoder
  * Fixed a crash when no geometry was generated. [CE-4325]
  * Fixed a problem where nodes could end up with too much geometry. [CE-4075]
  * Fixed projection of vertex normals into the target coordinate system.
  * Added support for "popupInfo".
  * Changed file extension from spk to slpk. [CE-3201]
  * Fixed the header of the compressed SLPK archive - resolves import problems in ArcGIS Earth and Pro [CE-3888]
* FBX Encoder/Decoder
  * Updated to FBX SDK 2017.0.1 (internal file format 7.5.0). [CE-3313]
  * Correctly apply the FBX scene units on import. [CE-2259]
  * Correctly convert geometry into current scene coordinate system on import. [CE-2978]
  * Correctly read material of instanced geometries.
  * Instances were not correctly exported if the "create shape groups" option was enabled. [CE-2540]
* Collada Encoder
  * Fixed a problem with non-consequtive uv sets. [CE-2839]
  * Avoid unreferenced texture coordinates in exported Collada files. [CE-1359]
  * By default, "Global Scaling Factor" has a hidden annotation. [CE-1353]
  * Enable Collada encoder in basic license feature set. [CE-3771]
* Alembic Encoder
  * Updated to Alembic library 1.7.0
* Renderman Encoder
  * Added support for renderman txmake tool. [CE-1830]


Misc Changes and Fixes
----------------------
* Fixed a threading-related crash in roof operations and skeleton subdivision. [CE-3706]
* Fixed a Windows handle leak when looking up the path to the core library. [CE-2143]
* Licensing sub-system has been updated to FNP 11.13. [CE-3053]
* Compiler toolchain updates (see README.md)
  * Windows: Visual Studio 2015 Update 3 (C++14)
  * macOS: Apple Clang 7.3 (Xcode 7.3), C++14, new min macOS 10.11
  * Linux: GCC 4.8.2 (C++11) (no change)


ESRI CITYENGINE SDK 1.7.2915 CHANGELOG
======================================

This section lists changes compared to CityEngine SDK 1.6.2663


General Info
------------
* CityEngine SDK 1.7.2915 is used in CityEngine 2016.1 (2016.1.2875).


PRT API
-------
* Clarified the documentation of prt::Callbacks::cgaGetCoord regarding projection support.
* Also fixed the status return value in some cases of prt::Callbacks::cgaGetCoord.


PRTX API
--------
* Fixed a bug in the encode preparator where the bounding box was not invalidated after fetching instances.
* Improved the consistency of the encode preparator regarding mesh order. [CE-3003]
* Fixed a bug where invalid PRT extension libraries were registered. This fixes potential crashes at exit.


CGA
---
* Improved performance of inner rectangle operation. [CE-2623]
* Improved robustness of remainder computation in inner rectangle operation. [CE-3073, CE-3113]
* Fixed a static initialization issue on Linux and MacOS which led to wrong results on first model generation. [CE-3270]
* Fixed a bug with collinear trim planes during the roof operation. [CE-2915]
* Fixed a bug where polygons where not correctly merged after a trim operation. [CE-2911]
* Fixed a bug where rule attributes where not correctly overwritten by lower-level CGA imports. [CE-2870]
* Fixed two issues regarding trim planes and scope in the roofGable operation. [CE-2912]
* Fixed a bug regarding roof generation on perfectly square polygons. [CE-2972]
* Fixed a bug where edge comp split would return the wrong edge on non-planar polygons. [CE-2260]
* Fixed a crash in the envelope operation. [CE-1568]
* Fixed a crash on setNormals, cleanupGeometry operations. [CE-2997]
* Fixed a number of bugs in trim plane generation in the roof operations. [CE-1991]
* Fixed a bug in reduceGeometry and cleanUpGeometry operations regarding wrong resulting scope. [CE-2532]


Built-In Codecs Changes and Fixes
---------------------------------
* OBJ Decoder
  * Fixed support for spaces in texture and mtl file names. [CE-1548]
* OBJ Encoder
  * Fixed a number of bugs in the export log generation.
  * Fixed a small memory leak.
* Collada Decoder
  * Added support for common texture/sampler schema dialect/shortcut. [CE-2382]
  * Avoid triggering empty CGA asset warnings.
* Texture Encoders
  * Fixed a threading issue when encoding JPEG textures in parallel. [CE-3131]
* ShapeBuffer Encoder
  * Fixed support for polygons with holes. [CE-3267]
* Alembic Encoder
  * Added support for the "Alembic Preview Material Schema". [CE-3192]
  * Improved Alembic node hierarchy with an additional grid hierarchy layer. [CE-3114]
  * Updated to Alembic SDK 1.6.1. [CE-3071]
  * Fixed support for UV sets > 0 and avoid writing empty uv set 0.
  * Added support for self/child bounds.
  * Added support for OID initial shape attributes. [CE-2939]
  * Added support for the material.shader property. [CE-2851]
  * Fixed a number of minor issues and improved robustness.
* I3S Encoder (Scene Layer Packages)
  * Fix crash with empty shapes. [CE-3157]
  * Added white diffuse color if vertexColor is on. [CE-3139]
  * Fixed compression method used to store SPKs. This fixes support for ArcGIS Earth. [CE-3175]
  * Fixed a bug in converting vertex normals. [CE-3144]
  * Fixed bounding spheres computation for scenes with units in feet. [CE-3046]
  * Fixed wrong z-value in scenes with units in feet. [CE-3046]
  * Added atlas flag to better support ArcGIS Earth. [CE-3062]
  * Added support for vertex normals in global scenes. [CE-3045]
  * Added support for textureData attribute to better support ArcGIS Pro. [CE-3044]
  * Added support for export log generation. [CE-2600]
  * Added support for CGA reports. [CE-2520]
* FBX Decoder
  * Fixed a crash in the FBX decoder while handling empty nodes. [CE-3057]


Misc Changes and Fixes
----------------------
* Linux: Added a prefix to the internal Flexnet Publisher symbols to avoid potential
  license collisions with flexnet-based host applications. [CE-2844]



ESRI CITYENGINE SDK 1.6.2663 CHANGELOG
======================================

This section lists changes compared to CityEngine SDK 1.4.2074


General Info
------------
* CityEngine SDK 1.6.2663 is used in CityEngine 2016.0 (2016.0.2642).
* CityEngine SDK 1.5 has been an internal release only.
* Increased license level to "2016.0", licenses for 2015.2 and older are not valid for CityEngine SDK 1.6.


PRT API
-------
* Added new function `prt::setLogLevel` to specify the minimally active logging level.


PRTX API
--------
* Required compilers have been upgraded and C++11 has been enabled:
  * Windows: Visual Studio 2013 Update 5
  * OSX: Apple Clang 6.1 (Xcode 6.4) (on OSX, we require C++14)
  * RHEL: GCC 4.8.2 (DevToolSet 2.1)
* Removed dependency on the Boost C++ library.
* Added support for texture regions to `prtx::Mesh`. [CE-2509]
* Improved texture atlas generator in `prtx::EncodePreparator`:
  * Added flag `atlasRepeatingTextures` to support repeating textures. [CE-2510]
  * Added flag `maxTexSize` to resize incoming textures. [CE-2251]
  * Added flag `forceAtlasing` to force atlas even for a single textures. [CE-2471]
* cleaned up logger interface
* small interface cleanups

CGA
---
* Increased CGA language version to `2016.0`
* New CGA Operations
  * Added new operation `splitArea`. [CE-2106]
  * Added new operation `softenNormals`. [CE-1848]
  * Added new operation `innerRectangle` with alignment and remainder options. [CE-2566]
  * Added new primitive generation operations `primitiveQuad`, `primitiveDisk`, `primitiveCube`, `primitiveSphere`, `primitiveCylinder` and `primitiveCone`. [CE-2418, CE-2419]
* Changes to existing CGA operations
  * Updated `insert` operation [CE-2271]:
    * New parameter `upAxisOfGeometry` that specifies the up axis of the geometry.
    * New parameter `insertMode` that specifies the alignment and positioning of the geometry in the scope.
    * Also see deprecation list below.
  * `setNormals` operation:
    * New mode `auto` that uses softenNormals functionality. [CE-1848]
    * New mode `conform` that computes consistent normals. [CE-1849]
  * `cleanupGeometry`: Mode `all`/`faces`: All duplicate/inverse faces are removed except one (independent from orientation).
  * `extrude` operation
    * The parameter `axisWorld` is deprecated and changed to a parameter `extrusionType`
    * New types `face.normal`, `vertex.normal`, `world.up` and `world.up.flatTop`
    * The axes `x`, `y`, `z`, `world.x`, `world.y` and `world.z` are deprecated. Use type `world.up` instead of axis `world.y`.
  * `roofGable`, `roofHip`, `roofPyramid` and `roofShed`
    * Roofs can now be generated with a given height.
    * New parameter `valueType` to switch beetween a roof generation `byAngle` or `byHeight`.
* Deprecated Operations and Arguments
  * `innerRect` (replaced by `innerRectangle`)
  * Deprecations related to `insert`:
    * `builtin:cube` and `builtin:cube:notex` arguments for `insert` are replaced by new `primitiveCube` operation.
    * The usage of `s(0,0,0)` is deprecated in relation to `insert`. Use `keepSizeAndPosition` as `insertMode` instead.
* Bugfixes
  * `cleanup`, roofs, `extrude`: fixed vertex normal computation for rare cases [CE-2531]
  * `roofShed` operation: For shed roofs on shapes with one face no horizontal trim planes, hip, valley and ridge edges are now generated. This was already the case for shapes with more than one face.
  * `scatter`: fixes precision problem and fixed a crash
  * `split`, `trim` and `insert` operation: Fixed a bug that led to wrong results when vertices or edges lie in the respective split/trim plane.
  * `geometry.isInstanced`: Fixed for builtin primitives ("builtin:cube" and "builtin:cube:notex"). [CE-1566]
  * `roofHip`, `roofGable`: Fixed a bug that occured on rectangular shapes and led to a wrong shading due to double vertices.
  * `roofShed`: Fixed a bug that generated wrong shed roofs for shapes with more than one face, at least one hole and a non-zero edge index set. [CE-1562]
  * `cleanupGeometry`: Fixed an issue that prevented vertices to merge when they were the first vertex in a face. [CE-2531]
  * `geometry.area` and normal calculation became more accurate; floating point precision got improved in general.


Built-In Codecs Changes and Fixes
---------------------------------
* New Encoders
  * Added Alembic encoder.
  * Added I3S (SPK) encoder.
  * Added TIFF texture encoder.
  * Added raw memory texture encoder. [CE-2471]
* Changes/Fixes to existing Codecs:
  * MTL Encoder: fixed wrong line feed character
  * `ShapeAttributeEncoder`: optimized performance of attribute handling. [CE-1837]


Misc Changes and Fixes
----------------------
* Fixed a memory leak in `prt::generateOccluders`. [CE-2100]
* Fixed a memory leak in the `comp` split. [CE-2100]
* Fixed a memory leak in the `prtx::EncodePreparator`. [CE-1355]
* Fixed multiple memory leaks in prtx::Mesh.
* Fixed compiler firewall on RHEL.
* Fixed a bug where the logger reported events in the wrong log level.
* Fixed a crash on exit related to threading on OSX.



ESRI CITYENGINE SDK 1.4.2074 CHANGELOG
======================================

This section lists all changes compared to CityEngine SDK 1.3.1969


General Info
------------
* CityEngine SDK 1.4.2074 is used in CityEngine 2015.2


PRT API
-------
* Added a new method releaseTransientBlob() to prt::Cache. This method
  must be called to release a pointer which was previously obtained with
  getTransientBlob() or insertAndGetTransientBlob().
* Fixed a race condition when releasing a cache pointer and flushing
  the cache at the same time.
* Added a new method logStats() to prt::SimpleOutputCallbacks which provides
  metadata about the written entities.
* Fixed version info of PRT dlls (windows only).
* Emit resolve warnings also for already cached assets.


PRTX API
--------
* Added a texture atlas generator feature to prtx::EncodePreparator.
* Added support for a 'name' query to prtx::URI. This is also used to
  control the name of generated texture atlases.
* EncodePreparator
  * Fixed a bug where materials of instances could be mixed up.
  * Removed deprecated method EncodePreparator::add(const GeometryPtr&)
* Fixed value of the initialShape.startRule attribute
* Fixed a crash with style-prefixed shape attributes.


CGA
---
* No changes.


Built-In Codecs Changes and Fixes
---------------------------------
* All Encoders
  * Added an option to enable output of export stats via the new logStats() callback.
  * Fixed validation of various encoder options (e.g. fixed missing min/max clamping).
  * Removed functionality to emit reports by default. Users should use the CGA Report
    Encoder (com.esri.prt.core.CGAReportEncoder) to emit reports.
* FBX decoder
  * Fixed reading of multiple uv sets.
  * Improved performance of FBX decoder.
* ShapeBuffer encoder
  * Fixed edge order. The first edge is now maintained.
* CityEngine WebScene encoder
  * Hardened validator against missing "layerGroup" option.
  * Fixed "enable materials" option.
  * Correctly filter out and report unsupported polygons.
* Renderman encoder
  * Replaced deprecated ObjectBegin/ObjectInstance with ObjectBeginV/ObjectInstanceV calls
  * Fixed numbering suffixes of exported RIB files.
  * Added support for bumpValue material attribute.
  * Fixed object mode: surface statement needs to be inside object
  * Updated built-in shader to support both archive and object mode.
  * Fixed material su,sv / tu,tv mix-up.
  * Do not emit texture extensions into RIBs.



ESRI CITYENGINE SDK 1.3.1969 CHANGELOG
======================================

This section lists all changes compared to CityEngine SDK 1.3.1888


General Info
------------
* CityEngine SDK 1.3.1969 is used in CityEngine 2015.1
* The API (prt/prtx) remains unchanged - therefore the minor version remains unchanged.


PRT API
-------
* No changes.


PRTX API
--------
* No changes.


CGA
---
* Fixed a numerical issue which led to misaligned faces after comp(f)


Built-In Codecs Changes and Fixes
---------------------------------
* New decoder: added support for reading Autodesk FBX files
* Esri ShapeBuffer encoder
  * added support for writing the "polygon" buffer type
* CityEngine 3WS WebScene Encoder
  * Now includes layer names to uniquify object names, e.g. it is now possible
    to have identically named objects in different layers (e.g. House1.Wall and House2.Wall)
* All encoders:
  * Fixed a bug in the EncodePreparator where the final mesh and material lists could get out of sync.
  * Fixed a bug in object name processing: it now recognizes existing number suffices in object names.



ESRI CITYENGINE SDK 1.3.1888 CHANGELOG
======================================

This section lists all changes compared to CityEngine SDK 1.2.1591.


General Info
------------
* CE SDK 1.3.1888 is used in CityEngine 2015.0


PRT API
-------
* fixed InitialShape::toXML()
* fixed error code in createDecoderInfo()
* fixed a crash in ResolveMapBuilder::toXML()


PRTX API
--------
* texture: renamed the attribute "depth" to "bytesPerPixel"
* renamed Extension::getMinimalVersion to getVersion
  (major and minor number of an extension must exactly match)
* cleaned up builder interfaces in prtx:
  - replaced Builder, PRTBuilder, PRTXBuilder with Builder
    and SharedPtrBuilder
  - removed create() and createAndReset() methods for prtx builders


CGA
---
(Please also check the CGA change log in the CityEngine 2015.0
manual for details.)

* Changes:
  * New features:
    * getTreeKey function.
    * isNull function.
  * split operations:
    * Missing sizes are not allowed any longer
    * Functions in size expression do not need extra
      parantheses anymore.
  * @Hidden annotation: changed propagation across imports.A hidden
    import hides all its imports recursively.
  * "uid" shape attribute is now deprecated. Use the getTreeKey()
    function instead.

* Bug fixes:
  * Fixed a bug in the cgb decoder which failed to read compiled
    cga files with a large number of attributes/rules/splits.
  * Vertexmerger: fixed a bug which prevented hole vertices to be
    merged with vertices of other faces.
  * float() function: made string-to-float conversion independent
    of locale.
  * split operations: intensified internal mesh cleanup to reduce
    memory load, fixed a bug which led to undesired vertex-merges.
  * roofHip operation: avoid duplicate vertices.
  * offset, roofGable and roofHip operations: made offset / roof
    construction more stable on polygons with co-linear vertices and
    fixed a memory explosion bug.
  * cleanupGeometry operation: fixed bug which led to illegal material
    assignments ("filled holes rendering bug") on edge cleanup for
    geometries with per-face-materials.
  * Fixed undefined behaviour if the same name was used for a scalar
    attribute and a map attribute.
  * CGA compiler: Parameteric rules and functions with a large number
    of parameters do not hang the CGA compiler anymore.
  * offset, roofGable, roofHip and roofShed operations: made the handling
    of polygons with holes more stable for illegal holes (hole vertices
    identical to vertices of enclosing polygon resulted in hangers - fixed).


Built-In Codecs Changes and Fixes
---------------------------------
* Fixed CGA report encoding in OBJ, FBX, DAE, VUE and Renderman encoders
* Shapebuffer encoder:
  * fixed handling of textures with an alpha channel
  * introduced "generateModels" and "encodeInitialShapes" encoder options
  * removed "errorFallback" encoder option
* prtx::NamePreparator : sped up name preparator method legalizeAndUniquify()
* Image encode: fixed buffer allocation for resizing + float-byte conversion
* GDAL Texture Decoder: don't turn 8bit gray into rgb
* GDAL Texture Encoder: don't turn 8/16bit gray into rgb
* Added a TIFFEncoder


Misc Changes and Fixes
----------------------
* Licensing: disable debugger detection (allow process startup with
  debugger already attached)
* Performance optimizations to speed up prt::generate()
* Reworked and optimized internal multi-threading ("numberWorkerThreads" entry
  in generateOptions argument of prt::generate())



ESRI CITYENGINE SDK 1.2.1591 CHANGELOG
======================================

This section lists all changes compared to CityEngine SDK 1.1.1471.


General Info
------------
  * CE SDK 1.2.1591 is used in CityEngine 2014.1


PRT API
-------

New Features
  * New method "prt::InitialShapeBuilder::resolveAttributes" to read
    attributes from assets (if the decoder supports it).

Changes to existing Features
  * Version structure: now contains CGA Compiler (CGAC) version and architecture.


PRTX API
--------
  * EncodePreparator: Added support for collecting CGA reports.
  * Added customizable support for collecting CGA reports from the shape tree.


CGA
---
(Please also check the CGA change log in the CityEngine 2014.1 manual for details.)

* Changes:
  * new Features:
    * deleteHoles() operation
    * geometry.nHoles function
    * geometry.isClosedSurface function
    * listTerminate function
  * Hole support: all operations and Collada reader support polygons
    with holes now and many bugfixes.
  * color operation: support for setting opacity.
  * Better support for nonplanar polygons (interpolate instead of planarize).
  * Better support for texture coordinates and vertex normals
    (interpolate instead of delete).
  * Increased defaults for recursion detection.
  * Made float-to-string conversion independent of locale.

* Bug fixes:
  * Made convexify operation more consistant.
  * Fixed points/edge support in alignScopeToGeometry operation.
  * Fixed some cases which led to open volumes in envelope operation.
  * Fixed memory leak in setback and shape{L|U|O} operations.
  * Fixed nan handling in a number of operations.
  * Fixed non-';'-terminated string lists handling in listRange function.
  * AssetErrors: report warnings from geometry conversion of inserted assets


Built-In Codecs Changes and Fixes
---------------------------------

All Encoders
  * Added support for lines and points to EncodePreparator.
  * EncodePreparator: decreased default texture coordinate merge tolerance
    to 1e-4 to avoid UV precision issues on large objects.
  * Renamed encoder option "exportContent" (enum) to "errorFallback" (boolean)
    and hide it per default. The option "errorFallback" is now a requirement for
    CityEngine-compatible encoders.
  * Correctly pass on exceptions if generation error fall-back is turned off.

All Decoders
  * Fixed resolve map key construction for secondary resources
    (e.g. for  OBJ -> MTL -> texture files).

ShapeBuffer Encoder
  * Renamed from "Multipatch Encoder".
  * Added support for PolylineZ, Point and MultiPoint shape buffer types.
  * Added support for encoding leaf shapes (new per-layer option "featureGranularity").
  * Added support for per-leaf-shape reporting.
  * Added new boolean option "emitReports" to control output of CGA reports.

Collada Decoder
  * Fixed texture lookup regression if image path is prefixed with "./".
  * Avoid second parsing pass if first pass encounters fatal errors.

Collada Encoder
  * Switched to double precision to avoid geometry precision errors.

WebScene Encoder
  * Fixed precision and rounding issues for initial shape metadata
    attributes and CGA reports.
  * Added new options for vertex/normal/texture coordinate
    merge tolerances.
  * Do not create webscenes with no objects (illegal),
    but throw a STATUS_ILLEGAL_GEOMETRY StatusException.
  * Do not create webscenes with INF extends (illegal),
    but throw a STATUS_ILLEGAL_GEOMETRY StatusException.


Misc Changes and Fixes
----------------------
  * Fixed a race condition in unknown texture / geometry handling.
  * Fixed empty mesh handling (bad_alloc).
  * Fixed illegal memory access in internal mesh data structure.



ESRI CITYENGINE SDK 1.1.1471 CHANGELOG
======================================

This section lists all changes compared to CE SDK release 1.0.1209.


General Info
------------
  * CE SDK 1.1.1471 is used in CityEngine 2014.0
  * New officially supported Linux platform: Red Hat Enterprise Linux 6.4 (and compatible)


PRT API
-------

New Features
  * Added support for multiple texture coordinate sets in prt::InitialShapeBuilder.
  * Added prt::StringUtils namespace with methods to help with string encoding.
  * Added support for generic initial shape attributes:
    * Introduced the "/enc/metaData" namespace in prt::InitialShapeBuilder
      to store generic shape attributes.
    * Added support for client-specified ordering of generic initial shape attributes.

Changes to existing Features
  * createResolveMap(): unpack to filesystem feature: unpacking directory
    must be explicitly specified.
  * Switched vertex order of holes: holes are opposite to encircling face now.
  * Changed signature of listEncoders() / listDecoders() (analogous to toXML()).

Bug Fixes
  * Fixed multiple threading issues, e.g. in prt::init().
  * Fixed a bug where the API was in an undefined state after license handle destruction.
  * createResolveMap(): unpack to filesystem feature:
      * URIs in the resolve map were not percent-encoded.
      * Fixed handling of RPK entries with non-ansi/ascii characters.
  * Fixed error/status handling on all toXML() implementations.
  * Fixed a bug in prt::encodeTexture() where width/height/format metadata was lost.

Misc
  * Improved const correctness.


PRTX API
--------

New Features
  * Added support for "data:" URIs in prtx::URI and prtx::URIUtils.

Changes to existing Features
  * prtx::ShapeUtils::getRuleParameters() and ShapeUtils::getRuleArguments() swapped.
  * Switched vertex order of holes: holes are opposite to encircling face now.

Bug Fixes
  * prtx::ShapeUtils::getGeometryToWorldTrafo(): fixed precision issues.
  * Fixed various issues in prtx::URI related to path handling.
  * Fixed a bug in prtx::TextureBuilder where width/height/format metadata was lost.

Misc
  * Improved const correctness.
  * Improved prtx::MeshBuilder consistency checks.


CGA Operations
--------------
(Please also check the CGA change log in the CityEngine 2014.0 manual for details.)

New Features
  * Improved hole support in:
    * offset() operation
    * roof operations
  * setback() operation: new syntax and selectors for selecting edges
    based on their uv coordinates.
  * tileUV() operation: textureWidth or the textureHeight parameter protection.

Changes to existing Features
  * rule file "import" command: attr value propagation logic simplified

Bug Fixes
  * General import issues
  * Imported 'attribute protection' issues
  * General attribute sampling issues
  * Asset import path issues
  * Asset lookup function issues
  * fileSearch():
      * returning relative instead of absolute paths
      * spaces in quotes broken
      * crash if fileSearch() due to illegal regexp
  * Initial shape attribute values wrong in const / attr functions
  * comp() selector issues
  * imageInfo() crashes
  * imagesSortRatio() crashes
  * geometry.angle() broken


Built-In Codecs Changes and Fixes
---------------------------------

WebScene Encoder
  * Fixed object offset bugs (increased precision of transformations and geo-reference values).
  * Fixed a bug where JSON floating point values were written with the wrong decimal separation
    character depending on the current system locale.
  * Fixed validation of required initial shape attributes.

Multipatch Encoder/Decoder
  * Added option to triangulate meshes.
  * Fixed conversion of vertex normals from/to y-up/z-up.
  * Corrected material transparency and shininess ranges to 0..100 (was 0..255).
  * Performance improvement (internal caching of re-used textures).

Collada Encoder
  * Added optional encoding of CGA reports into Collada <extra> tags.
  * Improved encoding performance.

OBJ Encoder
  * Fixed mesh size estimation for memory budget export.
  * Performance improvements (increased buffer size for write callbacks).

OBJ Decoder
  * Fixed whitespace handling (lines with tabs have not been read correctly).
  * Fixed an error if referenced MTL file paths start with "./"

Texture Codecs
  * Fixed a crash when handling non-8bit textures
  * Fixed a bug where texture metadata was not properly carried through the system.
  * Fixed incorrect creation of JPEG images from texture data with alpha channel.
  * Improved texture scaling/resampling.

Various Codecs
  * Streamlined encoder option naming.
  * Fixed storage of encoder UI icons.
  * Increased robustness and fault tolerance.


Misc Changes and Fixes
----------------------
  * Fixed various memory leaks and static initialization issues.
  * Improved licensing error handling.
  * Fixed misleading warnings when trying to load a potential prt extension shared library.
  * Added version checks to avoid loading of inconsistent plugins.
  * Improved reference documentation for classes and built-in codecs.
  * Added various application notes to documentation.
  * Added CGA reference documentation based on CityEngine.
  * Windows only: removed dependency on "dbghelp.dll" to simplify client code debugging.
  * Factored out version of released SDK into version.properties file to simplify integration
    into external build systems.
  * Added CGA compiler version checks to CGBDecoder and improved handling of CGB illegal files
  * Increased Linux GCC compiler requirement to GCC 4.4.7 (RHEL 6.4) for extension development.
