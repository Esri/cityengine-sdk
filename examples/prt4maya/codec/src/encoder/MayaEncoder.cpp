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
#include "maya/MFloatArray.h"
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
//			prtspi::Log::trace(L"encode leaf shape mat: %ls", shape->getMaterial()->getString(L"name"));
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
	mdata.materialCount = geometries->size();
	mdata.materials     = new MayaMatData[mdata.materialCount];

	mdata.vertices = new MFloatPointArray();
	mdata.counts   = new MIntArray();
	mdata.connects = new MIntArray();

	mdata.tcsU       = new MFloatArray();
	mdata.tcsV       = new MFloatArray();
	mdata.tcConnects = new MIntArray();


	mdata.destroy  = MayaEncoder::destroyMayaData;


	uint32_t base = 0;
	uint32_t tcBase = 0;
	for(size_t gi = 0, size = geometries->size(); gi < size; ++gi) {
		prtspi::IGeometry* geo = (prtspi::IGeometry*)geometries->get(gi);

		const double* verts = geo->getVertices();
		const size_t vertsCount = geo->getVertexCount();

		for (size_t i = 0; i < vertsCount; ++i)
			mdata.vertices->append((float)verts[3*i], (float)verts[3*i+1], (float)verts[3*i+2]);


		const size_t  tcsCount = geo->getUVCount();
		if(tcsCount > 0) {
			const double* tcs      = geo->getUVs();
			for(size_t i=0; i<tcsCount; i++) {
				mdata.tcsU->append(tcs[i*2]);
				mdata.tcsV->append(tcs[i*2+1]);
			}
		}

		for (size_t fi = 0; fi < geo->getFaceCount(); ++fi) {
			const prtspi::IFace* face = geo->getFace(fi);
			mdata.counts->append(face->getIndexCount());

			const uint32_t* indices = face->getVertexIndices();
			for(size_t vi = 0; vi < face->getIndexCount(); ++vi)
				mdata.connects->append(base + indices[vi]);

			if(face->getUVIndexCount() > 0) {
				for(size_t vi = 0; vi < face->getIndexCount(); ++vi)
					mdata.tcConnects->append(tcBase + face->getUVIndices()[vi]);
			}
		}

		mdata.materials[gi].faceCount = geo->getFaceCount();
		mdata.materials[gi].hasUVs    = tcsCount > 0;

		prtspi::IMaterial* mat = geo->getMaterial();
		if(mat->getTextureArray(L"diffuseMap")->size() > 0) {
			mdata.materials[gi].texName = StringUtils::toOSNarrowFromOSWide(mat->getTextureArray(L"diffuseMap")->get(0)->getName());
		}

		base = mdata.vertices->length();
		tcBase = mdata.tcsU->length();
	}













/*



	for (size_t gi = 0, size = geometries->size(); gi < size; ++gi) {
		MayaMeshEntry& mEntry = mdata.meshEntries[gi];

		mEntry.vertices   = new MFloatPointArray();
		mEntry.counts     = new MIntArray();
		mEntry.connects   = new MIntArray();

		mEntry.tcsU       = new MFloatArray();
		mEntry.tcsV       = new MFloatArray();
		mEntry.tcConnects = new MIntArray();



		prtspi::IGeometry* geo = (prtspi::IGeometry*)geometries->get(gi);
//		prtspi::Log::trace("    working on geometry %s", geo->getName());


		const double* verts = geo->getVertices();
		const size_t vertsCount = geo->getVertexCount();

		for (size_t i = 0; i < vertsCount; ++i)
			mEntry.vertices->append(MFloatPoint(verts[3*i], verts[3*i+1], verts[3*i+2]));


		const double* tcs      = geo->getUVs();
		const size_t  tcsCount = geo->getUVCount();
		for(size_t i=0; i<tcsCount; i++) {
			mEntry.tcsU->append(tcs[i*2]);
			mEntry.tcsV->append(tcs[i*2+1]);
		}

		for (size_t fi = 0; fi < geo->getFaceCount(); ++fi) {
			const prtspi::IFace* face = geo->getFace(fi);
			mEntry.counts->append(face->getIndexCount());

			const uint32_t* indices = face->getVertexIndices();
			for (size_t vi = 0; vi < face->getIndexCount(); ++vi)
				mEntry.connects->append(indices[vi]);

			if(face->getUVIndexCount() > 0) {
				for (size_t vi = 0; vi < face->getIndexCount(); ++vi)
					mEntry.tcConnects->append(face->getUVIndices()[vi]);
			}
		}
	}

*/

//	prtspi::Log::trace("    mayaVertices.length = %d", mdata.vertices->length());
//	prtspi::Log::trace("    mayaCounts.length   = %d", mdata.counts->length());
//	prtspi::Log::trace("    mayaConnects.length = %d", mdata.connects->length());

	stream->write((uint8_t*)&mdata, sizeof(MayaData));
}

void MayaEncoder::destroyMayaData(struct MayaData* mdata) {
	delete mdata->vertices;
	delete mdata->counts;
	delete mdata->connects;
	delete mdata->tcsU;
	delete mdata->tcsV;
	delete mdata->tcConnects;

	delete[] mdata->materials;
}
