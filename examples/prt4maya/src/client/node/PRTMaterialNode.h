/**
 * Esri CityEngine SDK Maya Plugin Example
 *
 * This example demonstrates the main functionality of the Procedural Runtime API.
 * Esri R&D Center Zurich, Switzerland
 *
 * See http://github.com/ArcGIS/esri-cityengine-sdk for instructions.
 */

#pragma once

#include "node/MayaCallbacks.h"

#include <maya/MPxNode.h>
#include <maya/MString.h>
#include <maya/MTypeId.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MAngle.h>
#include <maya/MEvaluationNode.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MFnStringData.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFloatPoint.h>
#include <maya/MFloatPointArray.h>
#include <maya/MIntArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MItMeshVertex.h>
#include <maya/MPxCommand.h>
#include <maya/MArgList.h>
#include <maya/MIOStream.h>
#include <maya/MStringArray.h>
#include <maya/MPlugArray.h>
#include <maya/MGlobal.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MSelectionList.h>
#include <maya/MDagPath.h>
#include <maya/MFnSet.h>
#include <maya/MItMeshPolygon.h>

#include <cmath>
#include <cstdlib>
#include <vector>
#include <algorithm>


#define PRT_MATERIAL_TYPE_ID 0x8667b

const std::string gPRTMatStructure = "prtMaterialStructure";
const std::string gPRTMatChannel = "prtMaterialChannel";
const std::string gPRTMatStream = "prtMaterialStream";
const std::string gPRTMatMemberTexture = "texture";
const std::string gPRTMatMemberColor = "color";
const std::string gPRTMatMemberFaceStart = "faceIndexStart";
const std::string gPRTMatMemberFaceEnd = "faceIndexEnd";

class PRTMaterialNode : public MPxNode {

public:
    PRTMaterialNode();
    virtual ~PRTMaterialNode();

    static  void*		creator();
    static  MStatus		initialize();
    static  const char*	nodeName();

    virtual MStatus					compute(const MPlug& plug, MDataBlock& data) override;

    static MTypeId					id;
    static  MObject	aInMesh;
    static  MObject	aOutMesh;

private:
    static MStringArray				theShadingGroups;
    MString createShadingGroup(const wchar_t* name, MString* shadingCmd);
}; // class PRTMaterialNode


