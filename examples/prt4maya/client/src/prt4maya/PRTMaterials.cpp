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
#include "prt4mayaNode.h"
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
				M_CHECK(stat);
				for(unsigned int p = 0; p < plugs.length(); p++) {
					MFnDependencyNode node(plugs[p].node(), &stat);
					if(node.object().hasFn(MFn::kMesh)) {
						meshName  = node.name();
						mesh      = node.object();
						meshFound = true;
						break;
					}
					M_CHECK(stat);
					for(int pOut = (int)node.attributeCount(); --pOut >= 0 ;) {
						MObject attr = node.attribute(pOut, &stat);
						M_CHECK(stat);
						if(attr.apiType() == MFn::kGenericAttribute) {
							MPlug oPlug(node.object(), attr);
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
			if(1) {
				char  buf[256];
				for(unsigned int i = 0; i < shadingGroups.length(); i++) {
					sprintf(buf, "sets -forceElement %s %s.f[%d:%d]", shadingGroups[i].asChar(), meshName.asChar(), shadingRanges[i * 2], shadingRanges[i * 2 + 1]);
					M_CHECK(MGlobal::executeCommand(MString(buf)));
				}
			} else {
				MSelectionList selList;
				MObject        shadingGroup;
				MDagPath       dagPath;
				MObject        component;
				MPlug          pOutMesh(thisMObject(), outMesh);

				for(unsigned int i = 0; i < shadingGroups.length(); i++) {
					MGlobal::getSelectionListByName(shadingGroups[i], selList);
					selList.getDependNode(0, shadingGroup);

					MFnSet fSG(shadingGroup, &stat);
					M_CHECK(stat);

					if(fSG.restriction() != MFnSet::kRenderableOnly)
						return MS::kFailure;

					MSelectionList faces;

					MItMeshPolygon iFaces(mesh, &stat);
					M_CHECK(stat);

					int low  = shadingRanges[i * 2 + 0];
					int high = shadingRanges[i * 2 + 1];

					for(; !(iFaces.isDone()) && (int)iFaces.index() < high; iFaces.next() ) {
						if((int)iFaces.index() >= low)
							faces.add(iFaces.currentItem(&stat));
					}

					M_CHECK(fSG.addMembers(faces));
				}
			}
		}

		hasMaterials = true;

	} catch (MStatus& ms) {
		return ms;
	}
	return MS::kSuccess;
}

MStatus PRTMaterials::doIt(const MArgList& args) {
	MStatus stat;

	MString prtNodeName = args.asString(0, &stat);
	M_CHECK(stat);

	MSelectionList tempList;
	tempList.add(prtNodeName);
	MObject prtNode;
	M_CHECK(tempList.getDependNode(0, prtNode));
	MFnDependencyNode fNode(prtNode, &stat);
	M_CHECK(stat);

	if(fNode.typeId().id() != PRT_TYPE_ID)
		return MS::kFailure;

	return ((PRTNode*)fNode.userNode())->attachMaterials();
}

void* PRTMaterials::creator() {
	return new PRTMaterials;
}
