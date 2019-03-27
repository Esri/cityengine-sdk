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
#include <maya/adskDataHandle.h>

#include <cmath>
#include <cstdlib>
#include <vector>
#include <algorithm>


#define PRT_MATERIAL_TYPE_ID 0x8667b

const std::string gPRTMatStructure = "prtMaterialStructure";
const std::string gPRTMatChannel = "prtMaterialChannel";
const std::string gPRTMatStream = "prtMaterialStream";
const std::string gPRTMatMemberFaceStart = "faceIndexStart";
const std::string gPRTMatMemberFaceEnd = "faceIndexEnd";

class MaterialInfo {
public:
	MaterialInfo(adsk::Data::Handle sHandle);

	std::string bumpMap;
	std::string colormap;
	std::string dirtmap;
	std::string emissiveMap;
	std::string metallicMap;
	std::string normalMap;
	std::string occlusionMap;
	std::string opacityMap;
	std::string roughnessMap;
	std::string specularMap;

	double opacity;
	double metallic;
	double roughness;
	
	std::vector<double> ambientColor;
	std::vector<double> diffuseColor;
	std::vector<double> emissiveColor;
	std::vector<double> specularColor;

	std::vector<double> specularmapTrafo;
	std::vector<double> bumpmapTrafo;
	std::vector<double> colormapTrafo;
	std::vector<double> dirtmapTrafo;	
	std::vector<double> emissivemapTrafo;
	std::vector<double> metallicmapTrafo;
	std::vector<double> normalmapTrafo;
	std::vector<double> occlusionmapTrafo;
	std::vector<double> opacitymapTrafo;
	std::vector<double> roughnessmapTrafo;

	bool equals(const MaterialInfo& o) const;
	static MString toMString(const std::vector<double> &d, size_t size, size_t offset);

private:
	std::string getTexture(adsk::Data::Handle sHandle, const std::string& texName);
	std::vector<double>  getDoubleVector(adsk::Data::Handle sHandle,const std::string& name, size_t numElements);
	double getDouble(adsk::Data::Handle sHandle, const std::string& name);
};

class PRTMaterialNode : public MPxNode {

public:
	PRTMaterialNode() = default;

	static  void*       creator();
	static  MStatus     initialize();
	static  const char* nodeName();

	virtual MStatus compute(const MPlug& plug, MDataBlock& data) override;

	static MTypeId  id;
	static  MObject aInMesh;
	static  MObject aOutMesh;

private:
	void setAttribute(MString &mShadingCmd, std::vector<double> vec, size_t elements, std::string target);
	void setAttribute(MString &mShadingCmd, std::vector<double> vec, size_t elements, size_t offset, std::string target);
	void setTexture(MString &mShadingCmd, std::string tex, std::string target);
	void setAttribute(MString &mShadingCmd, double vec, std::string target);
	static MString sfxFile;
}; // class PRTMaterialNode


