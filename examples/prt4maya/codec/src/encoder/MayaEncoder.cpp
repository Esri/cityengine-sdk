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

#include "prt/prtapi.h"

#include "prtx/base/SPIException.h"
#include "prtx/base/Log.h"
#include "prtx/base/IGeometry.h"
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
	std::vector<double> vertices;
	std::vector<double> normals;

	std::vector<int> counts;
	std::vector<int> connects;
	std::vector<int> normalConnects;

	std::vector<float> tcsU, tcsV;
	std::vector<int> uvCounts;
	std::vector<int> uvConnects;

	uint32_t base = 0;
	uint32_t tcBase = 0;
	for(size_t gi = 0, size = geometries->size(); gi < size; ++gi) {
		prtx::IGeometry* geo = (prtx::IGeometry*)geometries->get(gi);

		const double* verts = geo->getVertices();
		const size_t vertsCount = geo->getVertexCount();
		const double* norms = geo->getNormals();
		const size_t normsCount = geo->getNormalCount();

		for(size_t i = 0; i < vertsCount*3; ++i)
			vertices.push_back(verts[i]);

		for(size_t i = 0; i < normsCount*3; ++i)
			normals.push_back((norms[i]));

		const size_t tcsCount = geo->getUVCount();
		if(tcsCount > 0) {
			const double* tcs = geo->getUVs();
			for(size_t i=0; i<tcsCount; i++) {
				tcsU.push_back((float)tcs[i*2]);
				tcsV.push_back((float)tcs[i*2+1]);
			}
		}

		for(size_t fi = 0; fi < geo->getFaceCount(); ++fi) {
			const prtx::IFace* face = geo->getFace(fi);
			counts.push_back(face->getIndexCount());

			const uint32_t* indices = face->getVertexIndices();
			for(size_t vi = 0; vi < face->getIndexCount(); ++vi)
				connects.push_back(base + indices[vi]);

			const uint32_t* normalIndices = face->getNormalIndices();
			for(size_t ni = 0; ni < face->getIndexCount(); ++ni)
				normalConnects.push_back(base + normalIndices[ni]);

			uvCounts.push_back(face->getUVIndexCount());
			for(size_t vi = 0; vi < face->getUVIndexCount(); ++vi)
				uvConnects.push_back(tcBase + face->getUVIndices()[vi]);
		}

		base   = vertices.size() / 3;
		tcBase = tcsU.size();
	}

//	std::cout << "uvCounts: " << uvCounts << std::endl;
//	std::cout << "uvConnects: " << uvConnects << std::endl;
//	std::cout << "uvCounts sum:" << std::accumulate(uvCounts.begin(), uvCounts.end(), 0) << std::endl;
//	std::cout << "tcsU size = " << tcsU.size() << std::endl;

	mayaOutput->setVertices(&vertices[0], vertices.size());
	mayaOutput->setUVs(&tcsU[0], &tcsV[0], tcsU.size());
	mayaOutput->setNormals(&normals[0], normals.size());
	mayaOutput->setFaces(&counts[0], counts.size(), &connects[0], connects.size(), &normalConnects[0], normalConnects.size(), &uvCounts[0], uvCounts.size(), &uvConnects[0], uvConnects.size());
	mayaOutput->createMesh();

	int startFace = 0;
	for(size_t gi = 0, size = geometries->size(); gi < size; ++gi) {
		prtx::IGeometry* geo = (prtx::IGeometry*)geometries->get(gi);
		prtx::IMaterial* mat = geo->getMaterial();
		const int faceCount   = (int)geo->getFaceCount();

		mat->dump();

		std::wostringstream matName;
		matName << "material" << gi;

		std::wcout << L"creating material: '" << matName.str() << L"'" << std::endl;

		int mh = mayaOutput->matCreate(matName.str().c_str(), startFace, faceCount);

		std::wstring tex;
		if(mat->getTextureArray(L"diffuseMap")->size() == 1) {
			std::wstring uri(mat->getTextureArray(L"diffuseMap")->get(0)->getName());
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

