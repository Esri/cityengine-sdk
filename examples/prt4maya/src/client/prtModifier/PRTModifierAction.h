#ifndef _PRTFty
#define _PRTFty


//
// ***************************************************************************
//
// Overview:
//
//		The PRT action implements the actual PRTAction operation. 
//		It takes in three parameters:
//
//			1) A polygonal mesh
//			2) An array of component IDs
//          3) A mesh operation identifier
//
// Please refer to MFnMeshOperations to get more information on the different 
// mesh operations.
//

#include "polyModifierBase/polyModifierFty.h"

// General Includes
//
#include <maya/MObject.h>
#include <maya/MIntArray.h>
#include <maya/MString.h>



enum MeshOperation
{
	kSubdivideEdges = 0,
	kSubdivideFaces = 1,
	kExtrudeEdges = 2,
	kExtrudeFaces = 3,
	kCollapseEdges = 4,
	kCollapseFaces = 5,
	kDuplicateFaces = 6,
	kExtractFaces = 7,
	
	// Number of valid operations
	kMeshOperationCount = 8
};

class PRTModifierAction : public polyModifierFty
{

public:
				PRTModifierAction();
			~PRTModifierAction() override;

	// Modifiers
	//
	void		setMesh( MObject& mesh );
	void		setComponentList( MObject& componentList );
	void		setComponentIDs( MIntArray& componentIDs );
	void		setMeshOperation( MeshOperation operationType );

	// Returns the type of component expected by a given mesh operation
	//
	static MFn::Type getExpectedComponentType( MeshOperation operationType );

	// polyModifierFty inherited methods
	//
	MStatus		doIt() override;

private:
	// Mesh Node - Note: We only make use of this MObject during a single 
	//					 call of the PRTeration plugin. It is never 
	//					 maintained and used between calls to the plugin as 
	//					 the MObject handle could be invalidated between 
	//                   calls to the plugin.
	//
	MObject		fMesh;

	// Selected Components and Operation to execute
	//
	MIntArray		fComponentIDs;
	MeshOperation	fOperationType;
	MObject			fComponentList;

};

#endif
