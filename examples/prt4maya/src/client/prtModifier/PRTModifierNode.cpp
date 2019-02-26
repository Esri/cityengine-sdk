#include "PRTModifierNode.h"

#include "node/Utilities.h"

#include <maya/MFnTypedAttribute.h>
#include <maya/MFnMeshData.h>
#include <maya/MFnComponentListData.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnStringData.h>

#include <maya/MGlobal.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MIOStream.h>


#define MCheckStatus(status,message)	\
	if( MStatus::kSuccess != status ) {	\
		cerr << message << "\n";		\
		return status;					\
	}


namespace {
    const MString		NAME_RULE_PKG = "Rule_Package";
}

// Unique Node TypeId
MTypeId     PRTModifierNode::id( 0x00085000 );
MObject     PRTModifierNode::rulePkg;


PRTModifierNode::PRTModifierNode()
{}

PRTModifierNode::~PRTModifierNode()
{}

//make sure the dynamically added plugs affect the outMesh
MStatus PRTModifierNode::setDependentsDirty(const MPlug& /*plugBeingDirtied*/, MPlugArray& affectedPlugs) {
    const MPlug pOutMesh(thisMObject(), outMesh);
    affectedPlugs.append(pOutMesh);
    return MS::kSuccess;
}

//		This method computes the value of the given output plug based
//		on the values of the input attributes. Based on the Maya example splitUvCmd
MStatus PRTModifierNode::compute( const MPlug& plug, MDataBlock& data )
{
	MStatus status = MS::kSuccess;
 
	MDataHandle stateData = data.outputValue( state, &status );
    MCheckStatus( status, "ERROR getting state" );

	// Check for the HasNoEffect/PassThrough flag on the node.
	// (stateData is an enumeration standard in all depend nodes)
	// 
	// (0 = Normal)
	// (1 = HasNoEffect/PassThrough)
	// (2 = Blocking)
	if( stateData.asShort() == 1 )
	{
		MDataHandle inputData = data.inputValue( inMesh, &status );
		MCheckStatus(status,"ERROR getting inMesh");

		MDataHandle outputData = data.outputValue( outMesh, &status );
		MCheckStatus(status,"ERROR getting outMesh");

		// Simply redirect the inMesh to the outMesh for the PassThrough effect
		outputData.set(inputData.asMesh());
	}
	else
	{
		// Check which output attribute we have been asked to 
		// compute. If this node doesn't know how to compute it, 
		// we must return MS::kUnknownParameter
		if (plug == outMesh)
		{
			MDataHandle inputData = data.inputValue( inMesh, &status );
			MCheckStatus(status,"ERROR getting inMesh");

			MDataHandle outputData = data.outputValue( outMesh, &status );
			MCheckStatus(status,"ERROR getting outMesh"); 

            MDataHandle rulePkgData = data.inputValue(rulePkg, &status);
            MCheckStatus(status, "ERROR getting rulePkg");
            
			// Copy the inMesh to the outMesh, so you can
			// perform operations directly on outMesh
			//
			outputData.set(inputData.asMesh());
			MObject iMesh = outputData.asMesh();
            MObject oMesh = outputData.asMesh();

			// Set the mesh object and component List on the factory
			
            if (rulePkgData.asString() != currentRulePkg) {
                fPRTModifierAction.updateRuleFiles(thisMObject(), rulePkgData.asString());
            }
            fPRTModifierAction.fillAttributesFromNode(thisMObject());
            fPRTModifierAction.setMesh(iMesh, oMesh);

			// Now, perform the PRT
			//
			status = fPRTModifierAction.doIt();

            currentRulePkg = rulePkgData.asString();

			// Mark the output mesh as clean
			//
			outputData.setClean();
		}
		else
		{
			status = MS::kUnknownParameter;
		}
	}
    
	return status;
}

void* PRTModifierNode::creator()
{
	return new PRTModifierNode();
}

MStatus PRTModifierNode::initialize()
//
//	Description:
//		This method is called to create and initialize all of the attributes
//      and attribute dependencies for this node type.  This is only called 
//		once when the node type is registered with Maya.
//
//	Return Values:
//		MS::kSuccess
//		MS::kFailure
//		
{
	MStatus				status;

	MFnTypedAttribute attrFn;
	MFnEnumAttribute enumFn;

	
	inMesh = attrFn.create("inMesh", "im", MFnMeshData::kMesh);
	attrFn.setStorable(true);	// To be stored during file-save

	// Attribute is read-only because it is an output attribute
	//
	outMesh = attrFn.create("outMesh", "om", MFnMeshData::kMesh);
	attrFn.setStorable(false);
	attrFn.setWritable(false);

	// Add the attributes we have created to the node
	//

	status = addAttribute( inMesh );
		if (!status)
		{
			status.perror("addAttribute");
			return status;
		}
	status = addAttribute( outMesh);
		if (!status)
		{
			status.perror("addAttribute");
			return status;
		}

    MStatus				stat2;
    MStatus				stat;
    MFnStringData        stringData;
    MFnTypedAttribute fAttr;


    rulePkg = fAttr.create(NAME_RULE_PKG, "rulePkg", MFnData::kString, stringData.create(&stat2), &stat);
    MCHECK(stat2);
    MCHECK(stat);
    MCHECK(fAttr.setUsedAsFilename(true));
    MCHECK(fAttr.setCached(true));
    MCHECK(fAttr.setStorable(true));
    MCHECK(fAttr.setNiceNameOverride(MString("Rule Package(*.rpk)")));
    MCHECK(addAttribute(rulePkg));
    MCHECK(attributeAffects(rulePkg, outMesh));


	// Set up a dependency between the input and the output.  This will cause
	// the output to be marked dirty when the input changes.  The output will
	// then be recomputed the next time the value of the output is requested.
	//
	status = attributeAffects( inMesh, outMesh );
		if (!status)
		{
			status.perror("attributeAffects");
			return status;
		}

	return MS::kSuccess;

}
