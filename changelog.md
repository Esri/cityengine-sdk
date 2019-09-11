ESRI CITYENGINE SDK 2.1.XXXX CHANGELOG
======================================

This section lists changes compared to CityEngine SDK 2.0.5403.


General Info
------------
* CityEngine SDK 2.X.XXXX is used in CityEngine 2019.1.YYYY


PRT API
-------
* Added `Callbacks::attrBoolArray`, `Callbacks::attrFloatArray`, `Callbacks::attrStringArray` for array attributes. [CE-6277]
* Fixed `createResolveMap()` (unpack to filesystem case): in rare cases there were filename clashes. [CE-6569]

PRTX API
--------
* Removed deprecated functions for logging wide strings: `log_wtrace`, `log_wdebug`, etc.


CGA
---
* New functions:
  * array initialization function (create arrays via initialization list).
* Changes to existing features:
  * `set` operation: Added set operations for array attributes.
* Bugfixes:
  * Maximum recursion level for CGA functions: Fixed crash if recursion level was chosen too high. The maximum recursion level is set via attribute `/cgaenv/maxFuncCallDepth`.
  * `roofShed(byHeight)` operation: Fixed wrong height in some situations.


Built-In Codecs Changes and Fixes
---------------------------------
* I3S/SLPK Encoder:
  * Update to I3S Version 1.7. [CE-6178, CE-6258, CE-6260, CE-6261]
  * Reduced memory consumption with many/large textures and improved texture atlas composition. [CE-6330]
  * Write PBR materials. [CE-6576]
  * Use Domains for boolean attributes. [CE-6060]
  * Removed texture quality option and replaced it with the "Max Tree Depth" option [CE-6750, CE-6654]
  * The handling of PNG textures has been fixed. This makes the resulting scene layers compatible with ArcGIS Portal 10.7. [CE-6668]
  * Improved screen size heuristic (for LOD switching in viewers). [CE-6751]
  * Improved texture size in atlas. [CE-6752]
  * Improved handling of degenerated texture coordinates. [CE-6969]
  * Fixed a bug which led to wrong materials in intermediate levels. [CE-6984]
  * Fixed a bug in "Features per leaf shape" - crashed in some cases if reports were emitted. [CE-6805] 
  * Fixed a bug where temporary files were not deleted from disk.
  * Fixed a bug which led to crashes if the resulting slpk file was too large to fit into memory.
  * Fixed bugs which led to errors in ArcGIS Pro. [CE-6897]
* Unreal Encoder:
  * Updated datasmith library for Unreal 4.23. [CE-6817]
  * Removed FBX dependency. [CE-5562]
  * Create Unreal PBR material instances in CE exporter. [CE-6600]
  * Improved Instance (HISMC) Caching Behaviour. [CE-6635]
  * Removed unnecessary heightmap scaling. [CE-6889]
  * Correctly handle absence of UV sets. [CE-6780]
  * Fixed precision issue with instances. [CE-6731]
  * Create Unreal Materials during export to remove dependency on predefined Master Materials. [CE-6600]
  * Improved automatic Material Blend Mode selection. [CE-6512]
  * Fixed filename legalizer bug which led to "_png" instead of ".png". [CE-6485]
* Alembic Encoder
  * Fix: Add the global export offset to pre-computed bounds.
* ShapeBuffer Encoder:
  * Multipatch: Use "triangles" part type for triangles. [CE-5913]


Misc Changes and Fixes
----------------------
* Updated Whitepaper / Architecture documents. [CE-6741]


ESRI CITYENGINE SDK 2.0.5403 CHANGELOG
======================================

This section lists changes compared to CityEngine SDK 1.10.4198.


General Info
------------
* CityEngine SDK 2.0.5403 is used in CityEngine 2019.0.5403


Licensing
---------
* The CityEngine SDK is now free for non-commercial use and we removed the technical requirement to have a CityEngine license, i.e. the CityEngine SDK runs without a license now. All content of all directories is licensed under the [Esri Terms of Use](http://www.esri.com/legal/licensing-translations).


PRT API
-------
* Removed FlexLicParams and LicParams structs and LicParams parameter in init() function. [CE-5608]
* Renamed STATUS_ADAPTOR_NOT_FOUND to STATUS_STREAM_ADAPTOR_NOT_FOUND, added STATUS_RESOLVEMAP_PROVIDER_NOT_FOUND to Status. [CE-6016]
* New CT_STRING in ContentType. [CE-6112]
* Added AAT_BOOL_ARRAY, AAT_FLOAT_ARRAY, AAT_STR_ARRAY to AnnotationArgumentType. [CE-6116]
* New InitialShapeBuilder member functions createInitialShape() and resetGeometry(). [CE-5407]
* InitialShapeBuilder::setAttributes(): multiple calls are now possible per instance. [CE-5407]


PRTX API
--------
* New function DataBackend::resolveBinaryData() and deprecated readURI(). [CE-6017]
* New URIUtils::addFragment(), URIUtils::removeFragment(), URIUtils::addQuery() functions. [CE-6016]
* New URI::hasFragment() and URI::getFragment() functions. [CE-6016]
* Removed Adaptor class and introduced ResolveMapProvider class. [CE-6016]
* Removed ET_ADAPTOR from ExtensionType and added ET_STREAM_ADAPTOR and ET_RESOLVEMAP_PROVIDER. [CE-6016]
* New ExtensionManager::createResolveMapProvider() function. [CE-6016]
* New CTString and StringDecoder classes, new DataBackend::resolveText() function. [CE-6112]
* New types PRTUtils::AttributeMapUPtr, PRTUtils::AttributeMapBuilderUPtr, PRTUtils::ResolveMapUPtr, PRTUtils::ResolveMapBuilderUPtr. [CE-5407]
* New URI::isFilePath() function. [CE-6402]
* Shape: BuiltinShapeAttributes::xxx_map getters: return resolve map key instead of resolved URI (if you want the resolved URI, use the xxxMap getters of the Material class which return a Texture). [CE-6336]
* Expose type definition for `prtx::ReportDomains` to allow custom implementations of `prtx::ReportsAccumulator` without violating ODR.
 

CGA
---
* New operations:
  * `rectify` operation.
  * `resetGeometry` operation.
  * `setbackPerEdge` operation.
  * `setbackToArea` operation.
* New functions:
  * `readTextFile` function.
  * `geometry.boundaryLength` function.
  * `splitString` function.
  * `comp` function.
  * Array util functions: `index`, `item` and `size`.
  * Edge attribute functions: `edgeAttr.getFloat`, `edgeAttr.getString`, `edgeAttr.getBool`.
* New attributes:
  * Material attributes:
    * `material.emissive.{r|g|b}`, `material.metallic`, `material.roughness`.
    * `material.{emissive|occlusion|roughness|metallic}map` and corresponding `.{su|sv|tu|tv|rw}`.
    * `material.opacitymap.mode`.
* Changes to existing features:
  * `alignScopeToAxes` operation: New axis selectors for alignment in the object coordinate system were added.
  * `print` operation and `print` function: Support for arrays.
  * `comp` operation: New component selector `fe` for face edges.
  * `setback` operation: Supports individual distances per edge.
* Bugfixes:
  * `convexify` operation: Fixed incorrect results on shapes with holes.
  * `softenNormals`, `setNormals(auto)` operation: Compute correct soft normals for vertices bordering a hole. This bug only appeared when the threshold angle was set to a value less than 180 degrees.
  * `comp(v)`, `comp(e)` operation: Compute correct orientation of the scope z axis for vertex/edge components bordering a hole.
  * `comp(v)`, `comp(e)`, `alignScopeToGeometry` operation: Fixed resulting scopes when applied on edges or vertices.
  * `roofHip`, `roofGable` operation: Corrected the resulting order and first edge of roof faces if the initial shape contained collinear vertices.
  * `roofGable` operation:
    * Fixed a crash in subsequent operations or functions if the initial shape contained near-identical vertices.
    * Corrected trim plane generation if the initial shape contained several faces.
  * `cleanupGeometry` operation: Fixed a crash in vertex merge. This happened when hole vertices were within tolerance to vertices that belonged to both another hole and an encircling face.
  * `setback` operation: Enhanced result at concave vertices.


Built-In Codecs Changes and Fixes
---------------------------------
* New Decoders:
  * Added GLTF Decoder. [CE-5946]
* New Encoders:
  * Added GLTF Encoder. [CE-5947]
* All Encoders:
  * Speedup of vertex/normals/texture coordinates merging in exotic cases. [CE-5936]
* Unreal Encoder:
  * Correctly handle textures with same name. [CE-5886]
  * Improve actor translation values. [CE-5875]
  * Added native metadata export. [CE-5884]
  * Added native terrain export API. [CE-6147]
  * Added default material selection if not set by CGA. [CE-6030]
  * Fixed wrong materials for instanced meshes. [CE-6378]
  * Support for PBR material properties. [CE-6120]
  * Updated datasmith library for Unreal 4.22. [CE-6424]
  * Fixed hanger on crash. [CE-6011]
* OBJ/MTL Decoder:
  * Speedup for large material counts.
* I3S/SLPK Encoder:
  * Update to I3S Version 1.6. [CE-5950]
  * Fixed faulty texture mime types. [CE-5578]
  * Removed DDS Texture option (always writes them). [CE-6184]
  * Improved speed for DDS compression. [CE-6034]
  * Reduced memory consumption. [CE-6035]
  * Aligned MBS center and OBB center. [CE-6290]
  * Fixed crash with non-square textures. [CE-6475]
* FBX Encoder:
  * Only add unification extension to material name when needed. [CE-6335]
* FBX Decoder:
  * Fixed crash due to invalid materials. [CE-6036]
* Alembic Encoder
  * Moved to separate extension library and linking Alembic library statically. [CE-6226]


Misc Changes and Fixes
----------------------
* Removed FlexLM library. [CE-5608]
* Fixed crash in ResolveMap::getString() if status argument is omitted. [CE-6187]
* PRTUtils::ObjectDestroyer: fixed crash on nullptr.
* Fixed random-based CGA attr initialization on 32bit (behaves the same as on 64bit now). [CE-6124]
* Removed an invalid key which was returned in prtx::Shape::getKeys(). [CE-6336] 
* Fixed crash in prtx::Geometry access via Attributable interface. [CE-6336]
* Improved the search functionality of the included reference documentation. [CE-5808]


ESRI CITYENGINE SDK 1.10.4198 CHANGELOG
=======================================

This section lists changes compared to CityEngine SDK 1.10.4051.


General Info
------------
* CityEngine SDK 1.10.4198 is used in CityEngine 2018.1 (2018.1.4065).


PRT API
-------
* Corrected cgaenv default values in API reference.


CGA
---
* `setback` Operation
  * The first edge of setback faces is set at the outline of the initial face. For remainder faces the first edge is the closest to the first edge of the initial shape. This fix potentially changes your existing models. [CE-5592]
  * Avoid creation of slivers, especially at rounded corners with short edges. [CE-4463]
  * Fixed a possible crash for invalid uv-sets. [CE-4123]
* `alignScopeToGeometry` Operation
  * Fixed a numerical instability when `world.lowest` is used. In case multiple faces or edges are candidates for the lowest component, the one with the lowest index is chosen. This fix potentially changes your existing models. [CE-4828]
* `innerRectangle` Operation
  * Fixed a numerical instability. In case several rectangles are candidates for the largest, the one with the lowest distance to the shape's centroid is chosen. This fix potentially changes your existing models. [CE-4788]


Built-In Codecs Changes and Fixes
---------------------------------
* New Encoders
  * Added LERC encoder. [CE-5288]
* OBJ/MTL Decoder:
  * If Ks is zero and map_Ks is present, assume a value of 1 for Ks. [CE-5384]
* I3S/SLPK Encoder:
  * Additionally store textures in DXT/DDS formats. [CE-2294, CE-5667]
  * Use minimum oriented bounding boxes for nodes instead of bounding spheres. [CE-5577]
* Unreal Encoder:
  * Added simple terrain support. [CE-4579]
  * Store shape/model offsets in "Unreal Actor Locators" instead of adding them to the geometry. [CE-5729]
  * Ensure that the `foliage` flag is only set on instances with identical materials.


Misc Changes and Fixes
----------------------
* Added new shared library `glutess` to the CE SDK: The OpenGL tesselator is now included, we do not rely anymore on the one provided by the OpenGL drivers.



ESRI CITYENGINE SDK 1.10.4051 CHANGELOG
=======================================

This section lists changes compared to CityEngine SDK 1.9.3786.


General Info
------------
* CityEngine SDK 1.10.4051 is used in CityEngine 2018.0 (2018.0.3929).


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
* Behavior change in "random" evaluation: Fixed a bug in the evaluation of attr/const functions where the random generator could produce unexpected values if attrs/const functions are mutual dependent.
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
