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

#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include "prt/prt.h"

#include "prtx/base/Exception.h"
#include "prtx/base/Log.h"
#include "prtx/base/IGeometry.h"
#include "prtx/base/Material.h"
#include "prtx/base/IShape.h"
#include "prtx/base/ILeafIterator.h"
#include "prtx/codec/EncodePreparator.h"
#include "prtx/extension/ExtensionManager.h"

#include "util/StringUtils.h"
#include "util/Timer.h"
#include "util/URIUtils.h"
#include "util/Exception.h"

#include "encoder/MayaEncoder.h"


MayaEncoder::MayaEncoder() {
}


MayaEncoder::~MayaEncoder() {
}


void MayaEncoder::encode(const prt::InitialShape** initialShapes, size_t initialShapeCount, prtx::AbstractResolveMapPtr am, const prt::AttributeMap* options, prt::OutputHandler* const outputHandler) {
	am = am->toFileURIs();

	IMayaOutputHandler* oh = dynamic_cast<IMayaOutputHandler*>(outputHandler);
	if(oh == 0) throw(prtx::StatusException(prt::STATUS_ILLEGAL_OUTPUT_HANDLER));

	util::Timer tim;
	log_trace("MayaEncoder:encode: #initial shapes = %d", initialShapeCount);

	prtx::EncodePreparator* encPrep = prtx::EncodePreparator::create();
	for (size_t i = 0; i < initialShapeCount; ++i) {
		prtx::IGeometry** occluders = 0;
		prtx::ILeafIterator* li = prtx::ILeafIterator::create(initialShapes[i], am, occluders, 0);
		for (const prtx::IShape* shape = li->getNext(); shape != 0; shape = li->getNext()) {
			encPrep->add(/*initialShapes[i],*/ shape);
			//			log_trace(L"encode leaf shape mat: %ls", shape->getMaterial()->getString(L"name"));
		}
	}

	const float t1 = tim.stop();
	tim.start();

	prtx::IContentArray* geometries = prtx::IContentArray::create();
	encPrep->createEncodableGeometries(geometries);
	convertGeometry(am, geometries, oh);
	geometries->destroy();

	encPrep->destroy();

	const float t2 = tim.stop();
	log_info("MayaEncoder::encode() : preparator %f s, encoding %f s, total %f s", t1, t2, t1+t2);

	log_trace("MayaEncoder::encode done.");
}


void MayaEncoder::convertGeometry(prtx::AbstractResolveMapPtr am, prtx::IContentArray* geometries, IMayaOutputHandler* mayaOutput) {
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
	for(size_t gi = 0, size = geometries->size(); gi < size; ++gi) {
		prtx::IGeometry* geo = (prtx::IGeometry*)geometries->get(gi);

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
			const prtx::IFace* face = geo->getFace(fi);

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
	for(size_t gi = 0, size = geometries->size(); gi < size; ++gi) {
		prtx::IGeometry* geo = (prtx::IGeometry*)geometries->get(gi);
		prtx::Material* mat = geo->getMaterial();
		const int faceCount  = (int)geo->getFaceCount();

		mat->dump();

		std::wostringstream matName;
		matName << "material" << gi;

		log_trace(L"creating material: '%ls'", matName.str().c_str());

		int mh = mayaOutput->matCreate(matName.str().c_str(), startFace, faceCount);

		std::wstring tex;
		if(mat->getTextureArray(L"diffuseMap")->size() == 1) {
			std::wstring uri(mat->getTextureArray(L"colorMap")->get(0)->getName());
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

