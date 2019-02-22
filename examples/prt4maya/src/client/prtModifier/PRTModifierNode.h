#ifndef _PRTNode
#define _PRTNode


#include "polyModifierBase/polyModifierNode.h"
#include "PRTModifierAction.h"

// General Includes
//
#include <maya/MTypeId.h>
 
class PRTModifierNode : public polyModifierNode
{
public:
						PRTModifierNode();
					~PRTModifierNode() override; 

	MStatus		compute( const MPlug& plug, MDataBlock& data ) override;

	static  void*		creator();
	static  MStatus		initialize();

public:

	// There needs to be a MObject handle declared for each attribute that
	// the node will have.  These handles are needed for getting and setting
	// the values later.
	//
	// The polyModifierNode class has predefined the standard inMesh 
	// and outMesh attributes. We define an input attribute for the 
	// component list and the operation type
	//
	static  MObject		cpList;
	static  MObject		opType;

	// The typeid is a unique 32bit indentifier that describes this node.
	// It is used to save and retrieve nodes of this type from the binary
	// file format.  If it is not unique, it will cause file IO problems.
	//
	static	MTypeId		id;

	PRTModifierAction			fPRTModifierAction;
};

#endif
