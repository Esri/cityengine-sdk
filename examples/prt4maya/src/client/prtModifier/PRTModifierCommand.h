#ifndef _PRTCmd
#define _PRTCmd
//
// ***************************************************************************
//
// Overview:
//
//		The purpose of the PRT command is to execute a selected mesh 
//      operation, such as edge split or face subdivision on one or
//      more objects.
//
// How it works:
//
//		This command is based on the polyModifierCmd. It relies on the 
//		polyModifierCmd to manage "how" the effects of the PRTeration 
//		operation are applied (ie. directly on the mesh or through a modifier
//      node). See polyModifierCmd.h for more details
//
//		To understand the algorithm behind the PRT operation,
//      refer to PRTFty.h.
//
// Limitations:
//
//		(1) Can only operate on a single mesh at a given time. If there are 
//			more than one mesh with selected components, only the first mesh 
//			found in the selection list is operated on.
//

#include "polyModifierBase/polyModifierCmd.h"
#include "PRTModifierAction.h"

// Function Sets
//
#include <maya/MFnComponentListData.h>

// Forward Class Declarations
//

class PRTModifierCommand : public polyModifierCmd
{

public:
	////////////////////
	// Public Methods //
	////////////////////

    PRTModifierCommand();
	~PRTModifierCommand() override;

	static		void* creator();

	bool		isUndoable() const override;

	MStatus		doIt( const MArgList& ) override;
	MStatus		redoIt() override;
	MStatus		undoIt() override;

	/////////////////////////////
	// polyModifierCmd Methods //
	/////////////////////////////

	MStatus		initModifierNode( MObject modifierNode ) override;
	MStatus		directModifier( MObject mesh ) override;

private:
	//////////////////
	// Private Data //
	//////////////////

	// Selected Components
	// Selected Operation
	//
	MObject						fComponentList;
	MIntArray					fComponentIDs;
	MeshOperation				fOperation;

	// PRT Factory
	//
	PRTModifierAction				fPRTModifierAction;
};

#endif
