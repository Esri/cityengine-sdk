/*
 * MayaEncoder.cpp
 *
 *  Created on: Sep 11, 2012
 *      Author: shaegler
 */

#include <iostream>
#include <sstream>
#include <vector>
#include <numeric>

#include "boost/filesystem/path.hpp"
#include "boost/foreach.hpp"

#include "util/StringUtils.h"
#include "util/Timer.h"
#include "util/Exception.h"

#include "prt/prt.h"

#include "prtx/Exception.h"
#include "prtx/Log.h"
#include "prtx/Geometry.h"
#include "prtx/Material.h"
#include "prtx/Shape.h"
#include "prtx/ShapeIterator.h"
#include "prtx/EncodePreparator.h"
#include "prtx/ExtensionManager.h"
#include "prtx/Utils.h"
#include "prtx/IGenerateContext.h"

#include "encoder/IMayaOutputHandler.h"
#include "encoder/MayaEncoder.h"

const std::wstring MayaEncoder::ID          = L"com.esri.prt.codecs.maya.MayaEncoder";
const std::wstring MayaEncoder::NAME        = L"Autodesk(tm) Maya(tm) Encoder";
const std::wstring MayaEncoder::DESCRIPTION	= L"Encodes geometry into Autodesk(tm) Maya(tm) format.";


MayaEncoder::MayaEncoder() {
}


MayaEncoder::~MayaEncoder() {
}


void MayaEncoder::init(prtx::IGenerateContext& /*context*/) {
	prt::Callbacks* cb = getCallbacks();
	log_trace("MayaEncoder::init: cb = %x") % (size_t)cb;
	IMayaOutputHandler* oh = dynamic_cast<IMayaOutputHandler*>(cb);
	log_trace("                   oh = %x") % (size_t)oh;
	if(oh == 0) throw(prtx::StatusException(prt::STATUS_ILLEGAL_CALLBACK_OBJECT));
}


void MayaEncoder::encode(prtx::IGenerateContext& context, size_t initialShapeIndex) {
	const prtx::InitialShape* ishape = context.getInitialShape(initialShapeIndex);

	prt::ResolveMap const* am = ishape->getResolveMap();

	IMayaOutputHandler* oh = dynamic_cast<IMayaOutputHandler*>(getCallbacks());

	util::Timer tim;

	prtx::EncodePreparatorPtr encPrep; // = prtx::EncodePreparator::create(true);
	prtx::LeafIteratorPtr li = prtx::LeafIterator::create(context, initialShapeIndex);
	for (prtx::ShapePtr shape = li->getNext(); shape != 0; shape = li->getNext()) {
		encPrep->add(context.getCache(), shape);
	}

	const float t1 = tim.stop();
	tim.start();

	prtx::GeometryPtrVector geometries;
	std::vector<prtx::DoubleVector> trafos;
	prtx::MeshPtrVector meshes;
	std::vector<prtx::MaterialPtrVector> materials;

	prtx::EncodePreparator::PreparationFlags prepFlags;
	prepFlags.instancing(false);
	prepFlags.mergeByMaterial(true);
	prepFlags.triangulate(false);
	prepFlags.mergeVertices(true);
	prepFlags.cleanupVertexNormals(true);
	prepFlags.cleanupUVs(true);
	prepFlags.processVertexNormals(prtx::VertexNormalProcessor::SET_MISSING_TO_FACE_NORMALS);

	std::vector<prtx::EncodePreparator::FinalizedInstance> finalizedInstances;
	encPrep->fetchFinalizedInstances(finalizedInstances, prepFlags);
	BOOST_FOREACH(prtx::EncodePreparator::FinalizedInstance& instance, finalizedInstances) {
		geometries.push_back(instance.getGeometry());
		trafos.push_back(instance.getTransformation());
		materials.push_back(instance.getMaterials());
	}

	size_t   start = 0;
	size_t   end   = 0;
	wchar_t* ruleFile = wcsdup(ishape->getRuleFile());
	for(size_t i = 0; ruleFile[i]; i++) {
		switch(ruleFile[i]) {
			case '\\':
			case '/':
				start = i + 1;
				break;
			case '.':
				ruleFile[i] = '_';
				end = i;
				break;
		}
	}
	ruleFile[end] = 0;
	std::wstring cgbName = std::wstring(&ruleFile[start]);
	free(ruleFile);
	convertGeometry(cgbName, geometries, materials, oh);

	const float t2 = tim.stop();
	log_info("MayaEncoder::encode() : preparator %f s, encoding %f s, total %f s") % t1 % t2 % (t1+t2);

	log_trace("MayaEncoder::encode done.");
}

#define USE_NORMALS 0

void MayaEncoder::convertGeometry(const std::wstring& cgbName, const prtx::GeometryPtrVector& geometries, const std::vector<prtx::MaterialPtrVector>& mats, IMayaOutputHandler* mayaOutput) {
	log_trace("MayaEncoder::convertGeometry: begin");
	std::vector<double> vertices;
	std::vector<int>    counts;
	std::vector<int>    connects;
	int base    = 0;

#if USE_NORMALS == 1
	std::vector<double> normals;
	std::vector<int>    normalCounts;
	std::vector<int>    normalConnects;
	int nrmBase = 0;
#endif

	std::vector<float>  tcsU, tcsV;
	std::vector<int>    uvCounts;
	std::vector<int>    uvConnects;
	int uvBase  = 0;

	for(size_t gi = 0, geoCount = geometries.size(); gi < geoCount; ++gi) {
		log_trace("    geometry: %d") % gi;
		prtx::Geometry* geo = geometries[gi].get();

		const prtx::MeshPtrVector& meshes = geo->getMeshes();
		for(size_t mi = 0, meshCount = meshes.size(); mi < meshCount; mi++) {
			log_trace("      mesh %d") % mi;
			prtx::MeshPtr mesh = meshes[mi];

			const prtx::DoubleVector&  verts    = mesh->getVertexCoords();
#if USE_NORMALS == 1
			const prtx::DoubleVector&  norms    = mesh->getVertexNormalsCoords();
#endif
			bool                       hasUVs   = mesh->getUVSetsCount() > 0;
			size_t                     uvsCount = 0;

			vertices.reserve(    vertices.size()     + verts.size());
#if USE_NORMALS == 1
			normals.reserve(     normals.size()      + norms.size());
			normalCounts.reserve(normalCounts.size() + mesh->getFaceCount());
#endif
			counts.reserve(      counts.size()       + mesh->getFaceCount());
			uvCounts.reserve(    uvCounts.size()     + mesh->getFaceCount());

			for(size_t i = 0, size = verts.size(); i < size; ++i)
				vertices.push_back(verts[i]);

#if USE_NORMALS == 1
			for(size_t i = 0, size = norms.size(); i < size; ++i)
				normals.push_back(norms[i]);
#endif

			if(hasUVs) {
				const prtx::DoubleVector& uvs = mesh->getUVCoords(0);
				uvsCount                      = uvs.size();

				tcsU.reserve(tcsU.size() + uvsCount / 2);
				tcsV.reserve(tcsV.size() + uvsCount / 2);

				for(size_t i = 0, size = uvsCount; i < size; i += 2) {
					tcsU.push_back((float)uvs[i+0]);
					tcsV.push_back((float)uvs[i+1]);
//					log_debug("uvs[%d] = (%f,%f)") % i % uvs[i] % uvs[i+1];
				}
			}
			log_trace("      copied vertex attributes");

			for(size_t fi = 0, faceCount = mesh->getFaceCount(); fi < faceCount; ++fi) {
//				log_debug("    -- face %d") % fi;
//				log_debug("       vtx index count: %d") % mesh->getFaceVertexCount(fi);
//				log_debug("       nrm index count: %d") % mesh->getFaceVertexNormalCount(fi);
//				if (mesh->getUVSetsCount() > 0)
//					log_debug("       uvs index count: %d") % mesh->getFaceUVCount(fi, 0);

				const uint32_t* vtxIdx = mesh->getFaceVertexIndices(fi);
				prtx::IndexVector vidxs(vtxIdx, vtxIdx + mesh->getFaceVertexCount(fi));

				counts.push_back((int)vidxs.size());
				for(size_t vi = 0, size = vidxs.size(); vi < size; ++vi)
					connects.push_back(base + vidxs[vi]);

#if USE_NORMALS == 1
				const uint32_t*	nidxs = mesh->getFaceVertexNormalIndices(fi);
				uint32_t faceVertexNormalCount = mesh->getFaceVertexNormalCount(fi);
				normalCounts.push_back((int)faceVertexNormalCount);
				for(size_t ni = 0, size = faceVertexNormalCount; ni < size; ++ni)
					normalConnects.push_back(nrmBase + nidxs[ni]);
#endif

				if(hasUVs && mesh->getFaceUVCount(fi, 0) > 0) {
						//log_debug("    faceuvcount(%d, 0) = %d") % fi % mesh->getFaceUVCount(fi, 0);
						//log_debug("    getFaceVertexCount(%d) = %d") % fi % mesh->getFaceVertexCount(fi);
						assert(mesh->getFaceUVCount(fi, 0) == mesh->getFaceVertexCount(fi));

						const uint32_t* uv0Idx = mesh->getFaceUVIndices(fi, 0);
						uvCounts.push_back((int)mesh->getFaceUVCount(fi, 0));
						for(size_t vi = 0, size = mesh->getFaceUVCount(fi, 0); vi < size; ++vi) {
							//log_debug("       vi = %d, uvBase = %d, uv0Idx[vi] = %d") % vi % uvBase % uv0Idx[vi];
							uvConnects.push_back(uvBase + uv0Idx[vi]);
						}
				}
				else
					uvCounts.push_back(0);
			}

			base	  += (int)verts.size() / 3;
#if USE_NORMALS == 1
			nrmBase	+= (int)norms.size() / 3;
#endif
			uvBase  += (int)uvsCount     / 2;

			log_trace("copied face attributes");
		}
	}

	bool hasUVS     = tcsU.size() > 0;
#if USE_NORMALS == 1
	bool hasNormals = normals.size() > 0;
#endif

	mayaOutput->setVertices(&vertices[0], vertices.size());
	log_trace("    set vertices");
	mayaOutput->setUVs(hasUVS ? &tcsU[0] : 0, hasUVS ? &tcsV[0] : 0, tcsU.size());
	log_trace("    set uvs");

	#if USE_NORMALS == 1
	mayaOutput->setNormals(hasNormals ? &normals[0] : 0, normals.size());
	mayaOutput->setFaces(
			&counts[0], counts.size(),
			&connects[0], connects.size(),
			hasNormals ? &normalCounts[0] : 0, normalCounts.size(),
			hasNormals ? &normalConnects[0] : 0, normalConnects.size(),
			hasUVS ? &uvCounts[0] : 0, uvCounts.size(),
			hasUVS ? &uvConnects[0] : 0, uvConnects.size()
	);
#else
	log_trace("uvCounts.size() = %d, uvConnects.size() = %d") % uvCounts.size() % uvConnects.size();
	mayaOutput->setFaces(
			&counts[0], counts.size(),
			&connects[0], connects.size(),
			0, 0,
			0, 0,
			hasUVS ? &uvCounts[0] : 0, uvCounts.size(),
			hasUVS ? &uvConnects[0] : 0, uvConnects.size()
	);
#endif
	log_trace("set faces");

	mayaOutput->createMesh();
	log_trace("    maya output: created mesh");

	int startFace = 0;
	for(size_t gi = 0, geoCount = geometries.size(); gi < geoCount; ++gi) {
		prtx::Geometry* geo = geometries[gi].get();

		prtx::MaterialPtr mat = mats[gi].front(); // FIXME

		std::wostringstream matName;
		matName << "m" << cgbName << gi;

		log_wtrace(L"creating material: '%s'") % matName.str();

		int faceCount = 0;
		const prtx::MeshPtrVector& meshes = geo->getMeshes();
		for(size_t mi = 0, meshCount = meshes.size(); mi < meshCount; mi++)
			faceCount += (int)meshes[mi]->getFaceCount();

		int mh = mayaOutput->matCreate(matName.str().c_str(), startFace, faceCount);

		std::wstring tex;
		if(mat->diffuseMap().size() > 0 && mat->diffuseMap()[0]->isValid()) {
			prtx::URIPtr texURI = mat->diffuseMap()[0]->getURI();
			log_wtrace(L"trying to set texture uri: %s") % texURI->wstring();
			std::wstring texPath = texURI->getPath();
			mayaOutput->matSetDiffuseTexture(mh, texPath.c_str());
		}

		startFace += faceCount;
	}
	mayaOutput->finishMesh();

	log_trace("MayaEncoder::convertGeometry: end");
}


void MayaEncoder::finish(prtx::IGenerateContext& /*context*/) {
}

