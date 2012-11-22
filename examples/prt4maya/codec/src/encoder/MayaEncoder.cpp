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

#include <boost/filesystem/path.hpp>

#include "util/StringUtils.h"
#include "util/Timer.h"
#include "util/URIUtils.h"
#include "util/Exception.h"

#include "prt/prt.h"

#include "prtx/Exception.h"
#include "prtx/Log.h"
#include "prtx/IGeometry.h"
#include "prtx/Material.h"
#include "prtx/IShape.h"
#include "prtx/ShapeIterator.h"
#include "prtx/EncodePreparator.h"
#include "prtx/ExtensionManager.h"

#include "encoder/MayaEncoder.h"
#include "prtx/IGenerateContext.h"


MayaEncoder::MayaEncoder() {
}


MayaEncoder::~MayaEncoder() {
}


void MayaEncoder::encode(prtx::IGenerateContext& context, size_t initialShapeIndex) {
	prtx::AbstractResolveMapPtr am = context.getResolveMap()->toFileURIs();
	context.setResolveMap(am);		// !! changes context!!

	IMayaOutputHandler* oh = dynamic_cast<IMayaOutputHandler*>(&context.getOutputHandler());
	if(oh == 0) throw(prtx::StatusException(prt::STATUS_ILLEGAL_OUTPUT_HANDLER));

	util::Timer tim;

	prtx::EncodePreparatorPtr encPrep = prtx::EncodePreparator::create();
	prtx::LeafIteratorPtr li = prtx::LeafIterator::create(context, initialShapeIndex);
	for (const prtx::IShape* shape = li->getNext(); shape != 0; shape = li->getNext()) {
		encPrep->add(/*initialShapes[i],*/ shape);
		//			log_trace(L"encode leaf shape mat: %ls", shape->getMaterial()->getString(L"name"));
	}

	const float t1 = tim.stop();
	tim.start();

	prtx::GeometryPtrVector geometries;
	encPrep->createEncodableGeometries(geometries);
	convertGeometry(am, geometries, oh);

	const float t2 = tim.stop();
	log_info("MayaEncoder::encode() : preparator %f s, encoding %f s, total %f s", t1, t2, t1+t2);

	log_trace("MayaEncoder::encode done.");
}


void MayaEncoder::convertGeometry(prtx::AbstractResolveMapPtr am, prtx::GeometryPtrVector& geometries, IMayaOutputHandler* mayaOutput) {
	log_trace("MayaEncoder::convertGeometry: begin");

	std::vector<double> vertices;
	std::vector<int> counts;
	std::vector<int> connects;

	std::vector<double> normals;
	std::vector<int> normalCounts;
	std::vector<int> normalConnects;

	std::vector<float> tcsU, tcsV;
	std::vector<int> uvCounts;
	std::vector<int> uvConnects;

	uint32_t base = 0;
	uint32_t nrmBase = 0;
	uint32_t uvBase = 0;
	for(size_t gi = 0, size = geometries.size(); gi < size; ++gi) {
		prtx::IGeometry* geo = (prtx::IGeometry*)geometries[gi].get();

		const size_t& faceCount = geo->getFaceCount();
		const double* verts = geo->getVertices();
		const size_t vertsCount = geo->getVertexCount();
		const double* norms = geo->getNormals();
		const size_t normsCount = geo->getNormalCount();
		const double* uvs = geo->getUVs();
		const size_t uvCount = geo->getUVCount();

		vertices.reserve(vertices.size() + vertsCount*3);
		normals.reserve(normals.size() + normsCount*3);
		tcsU.reserve(tcsU.size() + uvCount);
		tcsV.reserve(tcsV.size() + uvCount);
		counts.reserve(counts.size() + faceCount);
		normalCounts.reserve(normalCounts.size() + faceCount);
		uvCounts.reserve(uvCounts.size() + faceCount);

		for(size_t i = 0; i < vertsCount*3; ++i)
			vertices.push_back(verts[i]);

		for(size_t i = 0; i < normsCount*3; ++i)
			normals.push_back(norms[i]);

		for(size_t i = 0; i < uvCount; ++i) {
			tcsU.push_back((float)uvs[i*2]);
			tcsV.push_back((float)uvs[i*2+1]);
		}

		for(size_t fi = 0; fi < faceCount; ++fi) {
			const prtx::Face* face = geo->getFace(fi).get();

			log_trace("    -- face %d", fi);
			log_trace("       vtx index count: %d", face->getIndexCount());
			log_trace("       nrm index count: %d", face->getNormalIndexCount());

			const uint32_t vertexIndexCount = face->getIndexCount();
			counts.push_back(vertexIndexCount);
			const uint32_t* indices = face->getVertexIndices();
			for(size_t vi = 0; vi < vertexIndexCount; ++vi)
				connects.push_back(base + indices[vi]);

			const uint32_t normalIndexCount = face->getNormalIndexCount();
			normalCounts.push_back(normalIndexCount);
			const uint32_t* normalIndices = face->getNormalIndices();
			for(size_t ni = 0; ni < normalIndexCount; ++ni) {
				normalConnects.push_back(nrmBase + normalIndices[ni]);
			}

			uvCounts.push_back(face->getUVIndexCount());
			const uint32_t* uvIndices = face->getUVIndices();
			for(size_t vi = 0; vi < face->getUVIndexCount(); ++vi)
				uvConnects.push_back(uvBase + uvIndices[vi]);
		}

		base	+= vertsCount;
		nrmBase	+= normsCount;
		uvBase	+= uvCount;
	}

	mayaOutput->setVertices(&vertices[0], vertices.size());
	mayaOutput->setUVs(&tcsU[0], &tcsV[0], tcsU.size());
	mayaOutput->setNormals(&normals[0], normals.size());
	mayaOutput->setFaces(&counts[0], counts.size(), &connects[0], connects.size(), &normalCounts[0], normalCounts.size(), &normalConnects[0], normalConnects.size(), &uvCounts[0], uvCounts.size(), &uvConnects[0], uvConnects.size());
	mayaOutput->createMesh();

	int startFace = 0;
	for(size_t gi = 0, size = geometries.size(); gi < size; ++gi) {
		prtx::IGeometry* geo = (prtx::IGeometry*)geometries[gi].get();
		prtx::Material* mat = geo->getMaterial();
		const int faceCount  = (int)geo->getFaceCount();

		mat->dump();

		std::wostringstream matName;
		matName << "material" << gi;

		log_trace(L"creating material: '%ls'", matName.str().c_str());

		int mh = mayaOutput->matCreate(matName.str().c_str(), startFace, faceCount);

		std::wstring tex;
		if(mat->getTextureArray(L"diffuseMap").size() == 1) {
			std::wstring uri(mat->getTextureArray(L"colorMap").front()->getName());
			log_trace("trying to set texture uri: %ls", uri.c_str());
			tex = uri.substr(wcslen(util::URIUtils::SCHEME_FILE));
			mayaOutput->matSetDiffuseTexture(mh, tex.c_str());
		}

		startFace += faceCount;
	}

	mayaOutput->finishMesh();
}


void MayaEncoder::unpackRPK(std::wstring rpkPath) {

}

