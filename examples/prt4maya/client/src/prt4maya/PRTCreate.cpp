#include "Utilities.h"
#include "prt4maya/PRTNode.h"


void* PRTCreate::creator() {
	return new PRTCreate;
}

MStatus PRTCreate::doIt(const MArgList& args) {
	MString result = MGlobal::executeCommandStringResult(MString("createNode prt"));

	MStatus stat;

	MSelectionList tempList;
	tempList.add(result);
	MObject prtNode;
	MCHECK(tempList.getDependNode(0, prtNode));
	MFnDependencyNode fNode(prtNode, &stat);
	MCHECK(stat);

	if(fNode.typeId().id() != PRT_TYPE_ID)
		return MS::kFailure;

	((PRTNode*)fNode.userNode())->mCreatedInteractively = true;

	setResult(result);
	return MS::kSuccess;
}

