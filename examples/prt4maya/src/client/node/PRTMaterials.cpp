/**
 * Esri CityEngine SDK Maya Plugin Example
 *
 * This example demonstrates the main functionality of the Procedural Runtime API.
 * Esri R&D Center Zurich, Switzerland
 *
 * See http://github.com/ArcGIS/esri-cityengine-sdk for instructions.
 */

#define MNoPluginEntry
#define MNoVersionString

#include "Utilities.h"
#include "PRTNode.h"
#include <limits>


const MString OUTPUT_GEOMETRY = MString("og");

MStatus PRTNode::attachMaterials() {
	try {
		MStatus stat;
		MString meshName;
		MObject mesh;
		bool    meshFound = false;

		for(MPlug plug(thisMObject(), outMesh); !(meshFound); ) {
			MPlugArray plugs;
			if(plug.connectedTo(plugs, false, true, &stat)) {
				MCHECK(stat);
				for(unsigned int p = 0; p < plugs.length(); p++) {
					MFnDependencyNode node(plugs[p].node(), &stat);
					if(node.object().hasFn(MFn::kMesh)) {
						meshName  = node.name();
						mesh      = node.object();
						meshFound = true;
						break;
					}
					MCHECK(stat);
					for(int pOut = static_cast<int>(node.attributeCount()); --pOut >= 0 ;) {
						const MObject attr = node.attribute(pOut, &stat);
						MCHECK(stat);
						if(attr.apiType() == MFn::kGenericAttribute) {
							const MPlug oPlug(node.object(), attr);
							if(oPlug.isSource() && OUTPUT_GEOMETRY == oPlug.partialName()) {
								plug = oPlug;
								p    = plugs.length();
								break;
							}
						}
					}
				}
			} else {
				return MS::kFailure;
			}
		}

		if(meshFound) {
			MString cmd;
			wchar_t* buf = new wchar_t[512];
			for(unsigned int i = 0; i < mShadingGroups.length(); i++) {
				swprintf(buf, 511, L"sets -e -forceElement %ls %ls.f[%d:%d];\n", mShadingGroups[i].asWChar(), meshName.asWChar(), mShadingRanges[i * 2], mShadingRanges[i * 2 + 1]);
				cmd += buf;
			}
			delete[] buf;
			MCHECK(MGlobal::executeCommand(cmd, DO_DBG, false));
		}

		mHasMaterials = true;

	} catch (const MStatus& ms) {
		return ms;
	}
	return MS::kSuccess;
}

MStatus PRTMaterials::doIt(const MArgList& args) {
	MStatus stat;

	const MString prtNodeName = args.asString(0, &stat);
	MCHECK(stat);

	MSelectionList tempList;
	tempList.add(prtNodeName);
	MObject prtNode;
	MCHECK(tempList.getDependNode(0, prtNode));
	const MFnDependencyNode fNode(prtNode, &stat);
	MCHECK(stat);

	if(fNode.typeId().id() != PRT_TYPE_ID)
		return MS::kFailure;

	return ((PRTNode*)fNode.userNode())->attachMaterials();
}

void* PRTMaterials::creator() {
	return new PRTMaterials;
}
