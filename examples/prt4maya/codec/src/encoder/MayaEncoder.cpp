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

#include "util/StringUtils.h"
#include "util/Timer.h"
#include "util/URIUtils.h"
#include "util/Exception.h"

#include "prt/prt.h"

#include "prtx/Exception.h"
#include "prtx/Log.h"
#include "prtx/Geometry.h"
#include "prtx/Material.h"
#include "prtx/IShape.h"
#include "prtx/ShapeIterator.h"
#include "prtx/EncodePreparator.h"
#include "prtx/ExtensionManager.h"

#include "encoder/MayaEncoder.h"
#include "prtx/IGenerateContext.h"


const std::wstring MayaEncoder::ID          = L"com.esri.prt.codecs.maya.MayaEncoder";
const std::wstring MayaEncoder::NAME        = L"Autodesk(tm) Maya(tm) Encoder";
const std::wstring MayaEncoder::DESCRIPTION	= L"Encodes geometry into Autodesk Maya format.";


MayaEncoder::MayaEncoder() {
}


MayaEncoder::~MayaEncoder() {
}


void MayaEncoder::encode(prtx::IGenerateContext& context, size_t initialShapeIndex) {
	prtx::AbstractResolveMapPtr am = context.getResolveMap()->toFileURIs();
	context.setResolveMap(am);		// !! changes context!!

	IMayaOutputHandler* oh = dynamic_cast<IMayaOutputHandler*>(getCallbacks());
	if(oh == 0) throw(prtx::StatusException(prt::STATUS_ILLEGAL_CALLBACK_OBJECT));

	util::Timer tim;

	prtx::EncodePreparatorPtr encPrep = prtx::EncodePreparator::create();
	prtx::LeafIteratorPtr li = prtx::LeafIterator::create(context, initialShapeIndex);
	for (prtx::IShapePtr shape = li->getNext(); shape != 0; shape = li->getNext()) {
		encPrep->add(/*initialShapes[i],*/ shape);
		//			log_trace(L"encode leaf shape mat: %ls", shape->getMaterial()->getString(L"name"));
	}

	const float t1 = tim.stop();
	tim.start();

	prtx::GeometryPtrVector geometries;
	prtx::MaterialPtrVector mat;
	encPrep->createEncodableGeometriesAndMaterialsAndReset(geometries, mat);
	convertGeometry(am, geometries, mat, oh);

	const float t2 = tim.stop();
	log_info("MayaEncoder::encode() : preparator %f s, encoding %f s, total %f s") % t1 % t2 % (t1+t2);

	log_trace("MayaEncoder::encode done.");
}


void MayaEncoder::convertGeometry(const prtx::AbstractResolveMapPtr am, const prtx::GeometryPtrVector& geometries, const prtx::MaterialPtrVector& mats, IMayaOutputHandler* mayaOutput) {
	log_trace("MayaEncoder::convertGeometry: begin");
	std::vector<double> vertices;
	std::vector<int>    counts;
	std::vector<int>    connects;

	std::vector<double> normals;
	std::vector<int>    normalCounts;
	std::vector<int>    normalConnects;

	std::vector<float>  tcsU, tcsV;
	std::vector<int>    uvCounts;
	std::vector<int>    uvConnects;

	int base    = 0;
	int nrmBase = 0;
	int uvBase  = 0;
	for(size_t gi = 0, geoCount = geometries.size(); gi < geoCount; ++gi) {
		prtx::Geometry* geo = geometries[gi].get();

		const prtx::MeshPtrVector& meshes = geo->getMeshes();
		for(size_t mi = 0, meshCount = meshes.size(); mi < meshCount; mi++) {
			prtx::Mesh* mesh = meshes[mi].get();

			const prtx::FacePtrVector& faces = mesh->getFaces();
			const prtx::DoubleVector&  verts = mesh->getVertexCoords();
			const prtx::DoubleVector&  norms = mesh->getVertexNormalsCoords();
			const prtx::DoubleVector&  uvs   = mesh->getUVCoords(0);

			vertices.reserve(    vertices.size()     + verts.size() * 3);
			normals.reserve(     normals.size()      + norms.size() * 3);
			tcsU.reserve(        tcsU.size()         + uvs.size());
			tcsV.reserve(        tcsV.size()         + uvs.size());
			counts.reserve(      counts.size()       + faces.size());
			normalCounts.reserve(normalCounts.size() + faces.size());
			uvCounts.reserve(    uvCounts.size()     + faces.size());

			for(size_t i = 0, size = verts.size() * 3; i < size; ++i)
				vertices.push_back(verts[i]);

			for(size_t i = 0, size = normals.size() * 3; i < size; ++i)
				normals.push_back(norms[i]);

			for(size_t i = 0, size = uvs.size() * 3; i < size; ++i) {
				tcsU.push_back((float)uvs[i*2]);
				tcsV.push_back((float)uvs[i*2+1]);
			}

			for(size_t fi = 0, faceCount = faces.size(); fi < faceCount; ++fi) {
				const prtx::FacePtr face = faces[fi];

				log_trace("    -- face %d") % fi;
				log_trace("       vtx index count: %d") % face->getVertexIndices().size();
				log_trace("       nrm index count: %d") % face->getVertexNormalsIndices().size();

				const prtx::IndexVector&	vidxs = face->getVertexIndices();
				counts.push_back((int)vidxs.size());
				for(size_t vi = 0, size = vidxs.size(); vi < size; ++vi)
					connects.push_back(base + vidxs[vi]);

				const prtx::IndexVector&	nidxs = face->getVertexNormalsIndices();
				normalCounts.push_back((int)nidxs.size());
				for(size_t ni = 0, size = nidxs.size(); ni < size; ++ni)
					normalConnects.push_back(nrmBase + nidxs[ni]);

				const prtx::IndexVector&	uvidxs = face->getUVIndices(0);
				uvCounts.push_back((int)uvidxs.size());
				for(size_t vi = 0, size = uvidxs.size(); vi < size; ++vi)
					uvConnects.push_back(uvBase + uvidxs[vi]);
			}

			base	+= (int)verts.size();
			nrmBase	+= (int)norms.size();
			uvBase	+= (int)uvs.size();
		}
	}

	mayaOutput->setVertices(&vertices[0], vertices.size());
	mayaOutput->setUVs(&tcsU[0], &tcsV[0], tcsU.size());
	mayaOutput->setNormals(&normals[0], normals.size());
	mayaOutput->setFaces(&counts[0], counts.size(), &connects[0], connects.size(), &normalCounts[0], normalCounts.size(), &normalConnects[0], normalConnects.size(), &uvCounts[0], uvCounts.size(), &uvConnects[0], uvConnects.size());
	mayaOutput->createMesh();

	int startFace = 0;
	for(size_t gi = 0, geoCount = geometries.size(); gi < geoCount; ++gi) {
		prtx::Geometry* geo = geometries[gi].get();

		prtx::MaterialPtr mat = mats[gi];

		std::wostringstream matName;
		matName << "material" << gi;

		log_wtrace(L"creating material: '%ls'") % matName.str().c_str();

		int faceCount = 0;
		const prtx::MeshPtrVector& meshes = geo->getMeshes();
		for(size_t mi = 0, meshCount = meshes.size(); mi < meshCount; mi++)
			faceCount += (int)meshes[mi]->getFaces().size();

		int mh = mayaOutput->matCreate(matName.str().c_str(), startFace, faceCount);

		std::wstring tex;
		if(mat->getTextureArray(L"diffuseMap").size() == 1) {
			std::wstring uri(mat->getTextureArray(L"colorMap").front()->getName());
			log_trace("trying to set texture uri: %ls") % uri.c_str();
			tex = uri.substr(util::URIUtils::SCHEME_FILE.size());
			mayaOutput->matSetDiffuseTexture(mh, tex.c_str());
		}

		startFace += faceCount;
	}

	mayaOutput->finishMesh();
}

void MayaEncoder::init(prtx::IGenerateContext& /*context*/) {
}

void MayaEncoder::finish(prtx::IGenerateContext& /*context*/) {
}


