/*
 * MayaEncoder.cpp
 *
 *  Created on: Sep 11, 2012
 *      Author: shaegler
 */

#include <iostream>
#include <sstream>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include "api/prtapi.h"
#include "spi/base/Log.h"
#include "spi/base/IGeometry.h"
#include "spi/base/IShape.h"
#include "spi/base/ILeafIterator.h"
#include "spi/codec/EncodePreparator.h"
#include "spi/extension/ExtensionManager.h"

#include "util/StringUtils.h"

#include "maya/MDataHandle.h"
#include "maya/MStatus.h"
#include "maya/MObject.h"
#include "maya/MDoubleArray.h"
#include "maya/MPointArray.h"
#include "maya/MPoint.h"
#include "maya/MString.h"
#include "maya/MFileIO.h"
#include "maya/MLibrary.h"
#include "maya/MIOStream.h"
#include "maya/MGlobal.h"
#include "maya/MStringArray.h"
#include "maya/MFloatPoint.h"
#include "maya/MFloatPointArray.h"
#include "maya/MDataBlock.h"
#include "maya/MDataHandle.h"
#include "maya/MIntArray.h"
#include "maya/MDoubleArray.h"
#include "maya/MLibrary.h"
#include "maya/MPlug.h"
#include "maya/MDGModifier.h"
#include "maya/MSelectionList.h"
#include "maya/MDagPath.h"
#include "maya/MFileObject.h"

#include "maya/MFnNurbsSurface.h"
#include "maya/MFnMesh.h"
#include "maya/MFnMeshData.h"
#include "maya/MFnLambertShader.h"
#include "maya/MFnTransform.h"
#include "maya/MFnSet.h"
#include "maya/MFnPartition.h"

#include "encoder/MayaEncoder.h"


#include "util/Timer.h"

MayaEncoder::MayaEncoder() {
}


MayaEncoder::~MayaEncoder() {
}


void MayaEncoder::encode(prtspi::IOutputStream* stream, const prtspi::InitialShape** initialShapes, size_t initialShapeCount, prtspi::AbstractResolveMapPtr am, prtapi::Attributable* options) {
	Timer tim;
	prtspi::Log::trace("MayaEncoder:encode: #initial shapes = %d", initialShapeCount);

	prtspi::EncodePreparator* encPrep = prtspi::EncodePreparator::create();
	for (size_t i = 0; i < initialShapeCount; ++i) {
		prtspi::IGeometry** occluders = 0;
		prtspi::ILeafIterator* li = prtspi::ILeafIterator::create(initialShapes[i], am, occluders, 0);
		for (const prtspi::IShape* shape = li->getNext(); shape != 0; shape = li->getNext()) {
			encPrep->add(initialShapes[i], shape);
			prtspi::Log::trace(L"encode leaf shape mat: %ls", shape->getMaterial()->getString(L"name"));
		}
	}

	const float t1 = tim.stop();
	tim.start();

	prtspi::IContentArray* geometries = prtspi::IContentArray::create();
	encPrep->createEncodableGeometries(geometries);
	convertGeometry(stream, geometries);
	geometries->destroy();

	encPrep->destroy();

	const float t2 = tim.stop();
	prtspi::Log::info("MayaEncoder::encode() : preparator %f s, encoding %f s, total %f s", t1, t2, t1+t2);

	prtspi::Log::trace("MayaEncoder::encode done.");
}


void MayaEncoder::convertGeometry(prtspi::IOutputStream* stream, prtspi::IContentArray* geometries) {
	prtspi::Log::trace("--- MayaEncoder::convertGeometry begin");

	// maya api tutorial: http://ewertb.soundlinker.com/maya.php

	MFloatPointArray mayaVertices;
	MIntArray mayaCounts;
	MIntArray mayaConnects;

	uint32_t base = 0;
	for (size_t gi = 0, size = geometries->size(); gi < size; ++gi) {
		prtspi::IGeometry* geo = (prtspi::IGeometry*)geometries->get(gi);
		prtspi::Log::trace("    working on geometry %s", geo->getName());

		const double* verts = geo->getVertices();
		const size_t vertsCount = geo->getVertexCount();

		for (size_t i = 0; i < vertsCount; ++i) {
			mayaVertices.append(MFloatPoint(verts[3*i], verts[3*i+1], verts[3*i+2]));
		}

		for (size_t fi = 0; fi < geo->getFaceCount(); ++fi) {
			const prtspi::IFace* face = geo->getFace(fi);
			mayaCounts.append(face->getIndexCount());

			const uint32_t* indices = face->getVertexIndices();
			for (size_t vi = 0; vi < face->getIndexCount(); ++vi)
				mayaConnects.append(base + indices[vi]);
		}

		base = mayaVertices.length();
	}

	prtspi::Log::trace("    mayaVertices.length = %d", mayaVertices.length());
	prtspi::Log::trace("    mayaCounts.length = %d", mayaCounts.length());
	prtspi::Log::trace("    mayaConnects.length = %d", mayaConnects.length());

	MStatus returnStatus;

//	MFnTransform fnTransform;
//	MObject oParent = fnTransform.create();

	MFnMesh fnMesh;
	MObject outMesh = fnMesh.create(mayaVertices.length(), mayaCounts.length(), mayaVertices, mayaCounts, mayaConnects, MObject::kNullObj, &returnStatus);
	prtspi::Log::trace("    created maya output mesh object, error message = %s", returnStatus.errorString().asChar());

	/*
	MSelectionList selList;
	MGlobal::getSelectionListByName( MString( "initialShadingGroup" ), selList );
	MObject initialSG;
	selList.getDependNode( 0, initialSG );

	MDagPath meshPath;
	MDagPath::getAPathTo(outMesh, meshPath);
	meshPath.extendToShape();

	MFnSet fnSG(initialSG);
	fnSG.addMember(meshPath);

	MFnLambertShader fnLambert;
	MObject oLambert = fnLambert.create();
	MFnLambertShader lambertRed(oLambert);
	lambertRed.setColor(MColor(1.0, 0.0, 0.0));

	// FIXME: does not yet correctly attach the red material to the poly shape
	MPlug ss = fnSG.findPlug("ss");
	MPlug oc = lambertRed.findPlug("outColor");
	MDGModifier mod;
	mod.connect(oc,ss);
	mod.doIt();

	MString fullPath = meshPath.fullPathName();
	stream->write(fullPath.asChar(), fullPath.length());

	prtspi::Log::trace("--- MayaEncoder::convertGeometry done");
	*/
	MObject* result = new MObject(outMesh);
	stream->write((uint8_t*)&result, sizeof(result));
}
