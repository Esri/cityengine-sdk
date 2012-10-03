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


void MayaEncoder::encode(prtspi::IOutputStream* stream, const prtspi::InitialShape** initialShapes, size_t initialShapeCount, prtspi::AbstractResolveMapPtr am, prt::Attributable* options) {
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

	MayaData mdata;
	mdata.vertices = new MFloatPointArray();
	mdata.counts   = new MIntArray();
	mdata.connects = new MIntArray();
	mdata.destroy  = MayaEncoder::destroyMayaData;

	uint32_t base = 0;
	for (size_t gi = 0, size = geometries->size(); gi < size; ++gi) {
		prtspi::IGeometry* geo = (prtspi::IGeometry*)geometries->get(gi);

		const double* verts = geo->getVertices();
		const size_t vertsCount = geo->getVertexCount();

		for (size_t i = 0; i < vertsCount; ++i)
			mdata.vertices->append((float)verts[3*i], (float)verts[3*i+1], (float)verts[3*i+2]);

		for (size_t fi = 0; fi < geo->getFaceCount(); ++fi) {
			const prtspi::IFace* face = geo->getFace(fi);
			mdata.counts->append(face->getIndexCount());

			const uint32_t* indices = face->getVertexIndices();
			for (size_t vi = 0; vi < face->getIndexCount(); ++vi)
				mdata.connects->append(base + indices[vi]);
		}

		base = mdata.vertices->length();
	}

	prtspi::Log::trace("    mayaVertices.length = %d", mdata.vertices->length());
	prtspi::Log::trace("    mayaCounts.length   = %d", mdata.counts->length());
	prtspi::Log::trace("    mayaConnects.length = %d", mdata.connects->length());

	stream->write((uint8_t*)&mdata, sizeof(MayaData));
}

void MayaEncoder::destroyMayaData(struct MayaData* mdata) {
	delete mdata->vertices;
	delete mdata->counts;
	delete mdata->connects;
}
