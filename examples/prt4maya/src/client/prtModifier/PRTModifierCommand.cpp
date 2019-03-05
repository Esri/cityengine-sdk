#include "PRTModifierCommand.h"
#include "PRTModifierNode.h"

#include <maya/MItSelectionList.h>
#include <maya/MGlobal.h>
#include <maya/MArgList.h>

PRTModifierCommand::PRTModifierCommand()
{
}

PRTModifierCommand::~PRTModifierCommand()
{}

void* PRTModifierCommand::creator()
{
	return new PRTModifierCommand();
}

bool PRTModifierCommand::isUndoable() const
{
	return true;
}

// implements the MEL PRT command, based on the Maya example splitUvCmd
MStatus PRTModifierCommand::doIt(const MArgList& argList)
{
	MStatus status;

	if (argList.length() == 1)
	{
		mRulePkg = argList.asString(0);
	}
	else
	{
		cerr << "Expecting one parameter: the rpk name path" << endl;
		displayError(" Expecting one parameter: the operation type.");
		return MS::kFailure;
	}

	// Parse the selection list for selected components of the right type.
	MSelectionList selList;
	MGlobal::getActiveSelectionList(selList);
	MItSelectionList selListIter(selList);
	selListIter.setFilter(MFn::kMesh);

	bool found = false;
	bool foundMultiple = false;

	for (; !selListIter.isDone(); selListIter.next())
	{
		MDagPath dagPath;
		MObject component;
		selListIter.getDagPath(dagPath, component);

		if (!found)
		{
			// Ensure that this DAG path will point to the shape 
			// of our object. Set the DAG path for the polyModifierCmd.
			if (dagPath.extendToShape() == MStatus::kSuccess) {
				setMeshNode(dagPath);
				found = true;
			}
			else if (dagPath.extendToShapeDirectlyBelow(0) == MStatus::kSuccess) {
				setMeshNode(dagPath);
				found = true;
			}
		}
		else
		{
			foundMultiple = true;
			break;
		}

	}
	if (foundMultiple)
	{
		displayWarning("Found more than one object with selected components.");
		displayWarning("Only operating on first found object.");
	}

	// Initialize the polyModifierCmd node type - mesh node already set
	//
	setModifierNodeType(PRTModifierNode::id);

	if (found)
	{
		// Now, pass control over to the polyModifierCmd::doModifyPoly() method
		// to handle the operation.
		status = doModifyPoly();

		if (status == MS::kSuccess)
		{
			setResult("PRT command succeeded!");
		}
		else
		{
			displayError("PRT command failed!");
		}
	}
	else
	{
		displayError(
			"PRT command failed: Unable to find selected components");
		status = MS::kFailure;
	}

	return status;
}

MStatus PRTModifierCommand::redoIt()
{
	MStatus status;
	status = redoModifyPoly();

	if (status == MS::kSuccess)
	{
		setResult("PRT command succeeded!");
	}
	else
	{
		displayError("PRT command failed!");
	}

	return status;
}

MStatus PRTModifierCommand::undoIt()
{
	MStatus status;
	status = undoModifyPoly();
	if (status == MS::kSuccess)
	{
		setResult("PRT undo succeeded!");
	}
	else
	{
		setResult("PRT undo failed!");
	}
	return status;
}

MStatus PRTModifierCommand::initModifierNode(MObject modifierNode)
{
	MStatus status;
	MFnDependencyNode depNodeFn(modifierNode);

	MObject attr = depNodeFn.attribute("Rule_Package");
	MPlug plug(modifierNode, attr);
	status = plug.setValue(mRulePkg);
	return status;
}

MStatus PRTModifierCommand::directModifier(MObject mesh)
{
	MStatus status;
	PRTModifierAction fPRTModifierAction;

	fPRTModifierAction.updateRuleFiles(MObject::kNullObj, mRulePkg);
	fPRTModifierAction.setMesh(mesh, mesh);

	// Now, perform the PRT action
	status = fPRTModifierAction.doIt();

	return status;
}


