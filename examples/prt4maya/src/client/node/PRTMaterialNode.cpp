/**
 * Esri CityEngine SDK Maya Plugin Example
 *
 * This example demonstrates the main functionality of the Procedural Runtime API.
 * Esri R&D Center Zurich, Switzerland
 *
 * See http://github.com/ArcGIS/esri-cityengine-sdk for instructions.
 */

#include "node/PRTMaterialNode.h"
#include "node/Utilities.h"
#include "prt/StringUtils.h"

#include <maya/MFnTransform.h>
#include <maya/MFnSet.h>
#include <maya/MFnPhongShader.h>
#include <maya/MDGModifier.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MDagPath.h>
#include <maya/MItDependencyNodes.h>
#include <maya/adskDataStream.h>
#include <maya/adskDataChannel.h>
#include <maya/adskDataAssociations.h>
#include <maya/adskDataAccessorMaya.h>

#include <cstdio>
#include <sstream>

#ifdef _WIN32
#	include <Windows.h>
#else
#	include <dlfcn.h>
#endif

MTypeId PRTMaterialNode::id(PRT_MATERIAL_TYPE_ID);

MObject	PRTMaterialNode::aInMesh;
MObject PRTMaterialNode::aOutMesh;
MStringArray PRTMaterialNode::theShadingGroups;

void* PRTMaterialNode::creator()
{
	return new PRTMaterialNode();
}

const char* PRTMaterialNode::nodeName()
{
	return "PRTMaterialNode";
}

PRTMaterialNode::PRTMaterialNode()
{
}

PRTMaterialNode::~PRTMaterialNode() {
	theShadingGroups.clear();
}

MStatus PRTMaterialNode::initialize()
{
	MStatus status;

	MFnTypedAttribute tAttr;
	aInMesh = tAttr.create("inMesh", "im", MFnData::kMesh, MObject::kNullObj, &status);
	MCHECK(status);
	addAttribute(aInMesh);

	aOutMesh = tAttr.create("outMesh", "om", MFnData::kMesh, MObject::kNullObj, &status);
	MCHECK(status);
	tAttr.setWritable(false);
	tAttr.setStorable(false);
	addAttribute(aOutMesh);

	attributeAffects(aInMesh, aOutMesh);

	return MStatus::kSuccess;
}

MString getGroupName(const wchar_t* name) {
	MString matName(name);
	const unsigned int len = matName.numChars();
	MString result = "prtmat";
	result += prtu::toCleanId(matName.substringW(matName.rindexW('/') + 1, len));
	result += "SG";
	return result;
}


MString PRTMaterialNode::createShadingGroup(const wchar_t* name, MString* shadingCmd) {
	const MString groupName = getGroupName(name);

	bool createGroup = true;
	for (int i = PRTMaterialNode::theShadingGroups.length(); --i >= 0;)
		if (PRTMaterialNode::theShadingGroups[i] == groupName) {
			createGroup = false;
			break;
		}

	if (createGroup) {
		PRTMaterialNode::theShadingGroups.append(groupName);
		*shadingCmd += "sets -renderable true -noSurfaceShader true -empty -name (\"" + groupName + "\");\n";
	}

	return createGroup ? groupName : MString();
}

const MString OUTPUT_GEOMETRY = MString("og");

MStatus PRTMaterialNode::compute(const MPlug& plug, MDataBlock& block)
{
	MStatus status = MS::kSuccess;


	MObject thisNode = thisMObject();

	MPlug inMeshPlug(thisNode, aInMesh);
	MDataHandle inMeshHandle = block.inputValue(inMeshPlug, &status);
	MObject inMeshObj = inMeshHandle.asMesh();
	MFnMesh inputMesh(inMeshObj);
	MCHECK(status);

	// Create a copy of the mesh object. Rely on the underlying geometry
	// object to minimize the amount of duplication that will happen.
	MPlug outMeshPlug(thisNode, aOutMesh);
	if (plug != outMeshPlug)
		return status;

	MDataHandle outMeshHandle = block.outputValue(outMeshPlug, &status);
	MCHECK(status);
	outMeshHandle.set(inMeshObj);
	MObject outMeshObj = outMeshHandle.asMesh();
	MFnMesh outputMesh(outMeshObj);

	const adsk::Data::Associations* inputAssociations = inputMesh.metadata();

	MStatus stat;
	MString meshName;
	MObject mesh;
	bool    meshFound = false;

	//find connected mesh node
	for (MPlug plug(thisNode, aOutMesh); !(meshFound); ) {
		MPlugArray plugs;
		if (plug.connectedTo(plugs, false, true, &stat)) {
			MCHECK(stat);
			for (unsigned int p = 0; p < plugs.length(); p++) {
				MFnDependencyNode node(plugs[p].node(), &stat);
				if (node.object().hasFn(MFn::kMesh)) {
					meshName = node.name();
					mesh = node.object();
					meshFound = true;
					break;
				}
				MCHECK(stat);
				for (int pOut = static_cast<int>(node.attributeCount()); --pOut >= 0;) {
					const MObject attr = node.attribute(pOut, &stat);
					MCHECK(stat);
					if (attr.apiType() == MFn::kGenericAttribute) {
						const MPlug oPlug(node.object(), attr);
						if (oPlug.isSource() && OUTPUT_GEOMETRY == oPlug.partialName()) {
							plug = oPlug;
							p = plugs.length();
							break;
						}
					}
				}
			}
		}
		else {
			return MS::kFailure;
		}
	}

	if (inputAssociations && meshFound)
	{
		adsk::Data::Associations outputAssociations(inputMesh.metadata(&status));
		MCHECK(status);


		adsk::Data::Channel* channel = outputAssociations.findChannel(gPRTMatChannel);
		if (channel) {
			adsk::Data::Stream*	stream = channel->findDataStream(gPRTMatStream);
			if (stream) {

				MString mShadingCmd;
				wchar_t* buf = new wchar_t[512];

				for (unsigned int i = 0; i < stream->elementCount(); ++i)
				{
					adsk::Data::Handle sHandle = stream->element(i);
					if (!sHandle.hasData()) continue;
					adsk::Data::Structure* fStructure = adsk::Data::Structure::structureByName(gPRTMatStructure.c_str());
					if (!sHandle.usesStructure(*fStructure)) continue;

					sHandle.setPositionByMemberName(gPRTMatMemberFaceStart.c_str());
					int faceStart = sHandle.asInt32()[0];

					sHandle.setPositionByMemberName(gPRTMatMemberFaceEnd.c_str());
					int faceEnd = sHandle.asInt32()[0];

					//workaround: transporting string as uint8 array, because using asString crashes maya
					char* tex = nullptr;
					if (sHandle.setPositionByMemberName(gPRTMatMemberTexture.c_str()))
						tex = (char*)sHandle.asUInt8();

					double* col = nullptr;
					if (sHandle.setPositionByMemberName(gPRTMatMemberColor.c_str()))
						col = sHandle.asDouble();

					MString matName;
					if (tex != nullptr && strlen(tex) > 0) {
						matName = createShadingGroup(MString(tex).asWChar(), &mShadingCmd);
						if (matName.numChars() == 0)
							continue;
						mShadingCmd += "prtSetDiffuseTexture(\"" + matName + "\",\"" + tex + "\",\"map1\");\n";
					}
					else if (col != nullptr) {
						wchar_t name[8];
						swprintf(name, 7, L"%02X%02X%02X", (int)(col[0] * 255.0), (int)(col[1] * 255.0), (int)(col[2] * 255.0));
						matName = createShadingGroup(name, &mShadingCmd);
						if (matName.numChars() == 0)
							continue;
						mShadingCmd += "prtSetColor(\"" + matName + "\"," + col[0] + "," + col[1] + "," + col[2] + ");\n";
					}

					swprintf(buf, 511, L"sets -e -forceElement %ls %ls.f[%d:%d];\n", matName.asWChar(), meshName.asWChar(), faceStart, faceEnd);
					mShadingCmd += buf;
				}

				delete[] buf;
				MCHECK(MGlobal::executeCommandOnIdle(mShadingCmd, DO_DBG));
			}
		}

		outputMesh.setMetadata(outputAssociations);
		outMeshHandle.setClean();
	}
	return status;
}

