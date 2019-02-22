#include "PRTModifierAction.h"

// General Includes
//
#include <maya/MGlobal.h>
#include <maya/MIOStream.h>
#include <maya/MFloatPointArray.h>

// Function Sets
//
#include <maya/MFnMesh.h>

// Iterators
//
#include <maya/MItMeshPolygon.h>
#include <maya/MItMeshEdge.h>

#define CHECK_STATUS(st) if ((st) != MS::kSuccess) { break; }

PRTModifierAction::PRTModifierAction()
//
//	Description:
//		PRTAction constructor
//
{
	fComponentIDs.clear();
}

PRTModifierAction::~PRTModifierAction()
//
//	Description:
//		PRTAction destructor
//
{}

void PRTModifierAction::setMesh( MObject& mesh )
//
//	Description:
//		Sets the mesh object for the factory to operate on
//
{
	fMesh = mesh;
}

void PRTModifierAction::setComponentList( MObject& componentList )
//
//	Description:
//		Sets the list of the components for the factory to operate on
//
{
	fComponentList = componentList;
}


void PRTModifierAction::setComponentIDs( MIntArray& componentIDs )
//
//	Description:
//		Sets the list of the components for the factory to operate on
//
{
	fComponentIDs = componentIDs;
}

void PRTModifierAction::setMeshOperation( MeshOperation operationType )
//
//	Description:
//		Sets the mesh operation for the factory to execute
//
{
	fOperationType = operationType;
}


MFn::Type PRTModifierAction::getExpectedComponentType( MeshOperation operationType )
{
	switch (operationType)
	{
	case kSubdivideEdges: return MFn::kMeshEdgeComponent;
	case kSubdivideFaces: return MFn::kMeshPolygonComponent;
	case kExtrudeEdges: return MFn::kMeshEdgeComponent;
	case kExtrudeFaces: return MFn::kMeshPolygonComponent;
	case kCollapseEdges: return MFn::kMeshEdgeComponent;
	case kCollapseFaces: return MFn::kMeshPolygonComponent;
	case kDuplicateFaces: return MFn::kMeshPolygonComponent;
	case kExtractFaces: return MFn::kMeshPolygonComponent;

	default: return MFn::kInvalid;
	}
}



MStatus PRTModifierAction::doIt()
//
//	Description:
//		Performs the operation on the selected mesh and components
//
{
	MStatus status;
	unsigned int i, j;

	// Get access to the mesh's function set
	//
	MFnMesh meshFn(fMesh);

	// The division count argument is used in many of the operations
	// to execute the operation multiple subsequent times. For example,
	// with a division count of 2 in subdivide face, the given faces will be
	// divide once and then the resulting inner faces will be divided again.
	//
	int divisionCount = 2;

	MFloatVector translation;
	if (fOperationType == kExtrudeEdges
		|| fOperationType == kExtrudeFaces
		|| fOperationType == kDuplicateFaces
		|| fOperationType == kExtractFaces)
	{
		// The translation vector is used for the extrude, extract and 
		// duplicate operations to move the result to a new position. For 
		// example, if you extrude an edge on a mesh without a subsequent 
		// translation, the extruded edge will be on at the position of the 
		// orignal edge and the created faces will have no area.
		// 
		// Here, we provide a translation that is in the same direction as the
		// average normal of the given components.
		//
		MFn::Type componentType = getExpectedComponentType(fOperationType);
		MIntArray adjacentVertexList;
		switch (componentType)
		{
		case MFn::kMeshEdgeComponent:
			for (i = 0; i < fComponentIDs.length(); ++i)
			{
				int2 vertices;
				meshFn.getEdgeVertices(fComponentIDs[i], vertices);
				adjacentVertexList.append(vertices[0]);
				adjacentVertexList.append(vertices[1]);
			}
			break;

		case MFn::kMeshPolygonComponent:
			for (i = 0; i < fComponentIDs.length(); ++i)
			{
				MIntArray vertices;
				meshFn.getPolygonVertices(fComponentIDs[i], vertices);
				for (j = 0; j < vertices.length(); ++j)
					adjacentVertexList.append(vertices[j]);
			}
			break;
		default:	
			break;
		}
		MVector averageNormal(0, 0, 0);
		for (i = 0; i < adjacentVertexList.length(); ++i)
		{
			MVector vertexNormal;
			meshFn.getVertexNormal(adjacentVertexList[i], vertexNormal,
				MSpace::kWorld);
			averageNormal += vertexNormal;
		}
		if (averageNormal.length() < 0.001)
			averageNormal = MVector(0.0, 1.0, 0.0);
		else averageNormal.normalize();
		translation = averageNormal;
	}

	// When doing an extrude operation, there is a choice of extrude the
	// faces/edges individually or together. If extrudeTogether is true and 
	// multiple adjacent components are selected, they will be extruded as if
	// it were one more complex component.
	//
	// The following variable sets that option.
	//
	bool extrudeTogether = true;

	// Execute the requested operation
	//
	switch (fOperationType)
	{
	case kSubdivideEdges: {
		status = meshFn.subdivideEdges(fComponentIDs, divisionCount);
		CHECK_STATUS(status);
		break; }

	case kSubdivideFaces: {
		status = meshFn.subdivideFaces(fComponentIDs, divisionCount);
		CHECK_STATUS(status);
		break; }

	case kExtrudeEdges: {
		status = meshFn.extrudeEdges(fComponentIDs, divisionCount,
			&translation, extrudeTogether);
		CHECK_STATUS(status);
		break; }

	case kExtrudeFaces: {
		status = meshFn.extrudeFaces(fComponentIDs, divisionCount,
			&translation, extrudeTogether);
		CHECK_STATUS(status);
		break; }

	case kCollapseEdges: {
		status = meshFn.collapseEdges(fComponentIDs);
		CHECK_STATUS(status);
		break; }

	case kCollapseFaces: {
		status = meshFn.collapseFaces(fComponentIDs);
		CHECK_STATUS(status);
		break; }

	case kDuplicateFaces: {
		status = meshFn.duplicateFaces(fComponentIDs, &translation);
		CHECK_STATUS(status);
		break; }

	case kExtractFaces: {
		status = meshFn.extractFaces(fComponentIDs, &translation);
		CHECK_STATUS(status);
		break; }

	default:
		status = MS::kFailure;
		break;
	}

	return status;
}
