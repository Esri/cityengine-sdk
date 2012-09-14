/*
 * MayaEncoder.cpp
 *
 *  Created on: Sep 11, 2012
 *      Author: shaegler
 */

#ifdef USEMAYA

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

#define _BOOL // gcc hack
#include "maya/MFnMesh.h"
#include "maya/MFnMeshData.h"
#include "maya/MFloatPoint.h"
#include "maya/MFloatPointArray.h"
#include "maya/MDataBlock.h"
#include "maya/MDataHandle.h"
#include "maya/MIntArray.h"
#include "maya/MDoubleArray.h"
#include "maya/MLibrary.h"
#include "maya/MPlug.h"
#include "maya/MFnLambertShader.h"
#include "maya/MFnTransform.h"
#include "maya/MFnSet.h"
#include "maya/MDGModifier.h"
#include "maya/MSelectionList.h"

#include "encoder/MayaEncoder.h"


MayaEncoder::MayaEncoder() {
}


MayaEncoder::~MayaEncoder() {
}


void MayaEncoder::encode(prtspi::IOutputStream* stream, const prtspi::InitialShape** initialShapes, size_t initialShapeCount, prtapi::Attributable* options) {
	prtspi::Log::trace("MayaEncoder:encode: #initial shapes = %d", initialShapeCount);

	prtspi::EncodePreparator* encPrep = prtspi::EncodePreparator::create();
	for (size_t i = 0; i < initialShapeCount; ++i) {
		prtspi::IGeometry** occluders = 0;
		prtspi::ILeafIterator* li = prtspi::ILeafIterator::create(initialShapes[i], occluders, 0);
		for (const prtspi::IShape* shape = li->getNext(); shape != 0; shape = li->getNext()) {
			encPrep->add(initialShapes[i], shape);
			prtspi::Log::trace(L"encode leaf shape mat: %ls", shape->getMaterial()->getString(L"name"));
		}
	}

	prtspi::IContentArray* geometries = prtspi::IContentArray::create();
	encPrep->createEncodableGeometries(geometries);
	convertGeometry(stream, geometries);
	geometries->destroy();

	encPrep->destroy();

	prtspi::Log::trace("MayaEncoder::encode done.");
}


void MayaEncoder::convertGeometry(prtspi::IOutputStream* stream, prtspi::IContentArray* geometries) {
	prtspi::Log::trace("--- MayaEncoder::convertGeometry begin");

//	MStatus stat = MLibrary::initialize("MayaEncoder");

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

	MSelectionList selList;
	MString toMatch ("initialShadingGroup");
	MStatus status = selList.add(toMatch );

	MFnSet fnSet;
	MObject oSet = fnSet.create((const MSelectionList)selList, MFnSet::kNone, NULL);
	fnSet.addMember(outMesh);

	MFnLambertShader fnLambert;
	MObject oLambert = fnLambert.create();

	MPlug ss = fnSet.findPlug("ss");
	MPlug oc = fnLambert.findPlug("outColor");
	MDGModifier mod;
	mod.connect(oc,ss);
	mod.doIt();

	//	MPlug plug = outMesh.findPlug(outMesh.attribute("instObjGroups"), false);
//	prtspi::Log::trace("    found plug: %s", plug.name().asChar());

//	MObjectArray shaders;
//	MIntArray indices;
//	outMesh.getConnectedShaders(0, shaders, indices);



//	MDataHandle dataHandle;
//	dataHandle.
//	dataHandle.set(newOutputData);

	//MFileIO::

	//stream->write((const char*)dataHandle.asAddr(), sizeof(void*));

//	MLibrary::cleanup();

	prtspi::Log::trace("--- MayaEncoder::convertGeometry done");
}

#endif
