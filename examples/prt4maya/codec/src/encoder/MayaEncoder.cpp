/**
 * Esri CityEngine SDK Maya Plugin Example
 *
 * This example demonstrates the main functionality of the Procedural Runtime API.
 * Esri R&D Center Zurich, Switzerland
 *
 * See http://github.com/ArcGIS/esri-cityengine-sdk for instructions.
 */

#include <iostream>
#include <sstream>
#include <vector>
#include <numeric>

#include "prt/prt.h"

#include "prtx/Exception.h"
#include "prtx/Log.h"
#include "prtx/Geometry.h"
#include "prtx/Material.h"
#include "prtx/Shape.h"
#include "prtx/ShapeIterator.h"
#include "prtx/EncodePreparator.h"
#include "prtx/ExtensionManager.h"
#include "prtx/GenerateContext.h"

#include "encoder/IMayaOutputHandler.h"
#include "encoder/MayaEncoder.h"

const std::wstring MayaEncoder::ID          	= L"MayaEncoder"; // todo: define in common header
const std::wstring MayaEncoder::NAME        	= L"Autodesk(tm) Maya(tm) Encoder";
const std::wstring MayaEncoder::DESCRIPTION	= L"Encodes geometry into Autodesk(tm) Maya(tm) format.";


MayaEncoder::MayaEncoder(const std::wstring& id, const prt::AttributeMap* options, prt::Callbacks* callbacks)
: prtx::GeometryEncoder(id, options, callbacks)
{
}


MayaEncoder::~MayaEncoder() {
}


void MayaEncoder::init(prtx::GenerateContext& /*context*/) {
	prt::Callbacks* cb = getCallbacks();
	log_debug("MayaEncoder::init: cb = %x") % (size_t)cb;
	IMayaOutputHandler* oh = dynamic_cast<IMayaOutputHandler*>(cb);
	log_debug("                   oh = %x") % (size_t)oh;
	if(oh == 0) throw(prtx::StatusException(prt::STATUS_ILLEGAL_CALLBACK_OBJECT));
}


void MayaEncoder::encode(prtx::GenerateContext& context, size_t initialShapeIndex) {
	prtx::InitialShape const* initialShape = context.getInitialShape(initialShapeIndex);

	IMayaOutputHandler* oh = dynamic_cast<IMayaOutputHandler*>(getCallbacks());

	prtx::DefaultNamePreparator        namePrep;
	prtx::NamePreparator::NamespacePtr nsMesh     = namePrep.newNamespace();
	prtx::NamePreparator::NamespacePtr nsMaterial = namePrep.newNamespace();

	// create the preparator
	prtx::EncodePreparatorPtr encPrep = prtx::EncodePreparator::create(true, namePrep, nsMesh, nsMaterial);

	prtx::LeafIteratorPtr li = prtx::LeafIterator::create(context, initialShapeIndex);
	for (prtx::ShapePtr shape = li->getNext(); shape != 0; shape = li->getNext())
		encPrep->add(context.getCache(), shape, initialShape->getAttributeMap());

	prtx::GeometryPtrVector geometries;
	std::vector<prtx::DoubleVector> trafos;
	prtx::MeshPtrVector meshes;
	std::vector<prtx::MaterialPtrVector> materials;

	prtx::EncodePreparator::PreparationFlags prepFlags;
	prepFlags.instancing(false);
	prepFlags.mergeByMaterial(true);
	prepFlags.triangulate(false);
	prepFlags.mergeVertices(false);
	prepFlags.cleanupVertexNormals(false);
	prepFlags.cleanupUVs(false);
	prepFlags.processVertexNormals(prtx::VertexNormalProcessor::SET_MISSING_TO_FACE_NORMALS);
	prepFlags.indexSharing(prtx::EncodePreparator::PreparationFlags::INDICES_SAME_FOR_VERTICES_AND_NORMALS);

	prtx::EncodePreparator::InstanceVector finalizedInstances;
	encPrep->fetchFinalizedInstances(finalizedInstances, prepFlags);
	for (prtx::EncodePreparator::InstanceVector::const_iterator instIt = finalizedInstances.begin(); instIt != finalizedInstances.end(); ++instIt) {
		geometries.push_back(instIt->getGeometry());
		trafos.push_back(instIt->getTransformation());
		materials.push_back(instIt->getMaterials());
	}

	size_t   start = 0;
	size_t   end   = 0;
	wchar_t* ruleFile = wcsdup(initialShape->getRuleFile()); // TODO: cleanup
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
}

void MayaEncoder::convertGeometry(const std::wstring& cgbName, const prtx::GeometryPtrVector& geometries, const std::vector<prtx::MaterialPtrVector>& mats, IMayaOutputHandler* mayaOutput) {
	std::vector<double> vertices;
	std::vector<int>    counts;
	std::vector<int>    connects;
	int base    = 0;

	std::vector<double> normals;

	std::vector<float>  tcsU, tcsV;
	std::vector<int>    uvCounts;
	std::vector<int>    uvConnects;
	int uvBase  = 0;

	for(size_t gi = 0, geoCount = geometries.size(); gi < geoCount; ++gi) {
		prtx::Geometry* geo = geometries[gi].get();

		const prtx::MeshPtrVector& meshes = geo->getMeshes();
		for(size_t mi = 0, meshCount = meshes.size(); mi < meshCount; mi++) {
			prtx::MeshPtr mesh = meshes[mi];

			const prtx::DoubleVector&  verts    = mesh->getVertexCoords();
			const prtx::DoubleVector&  norms    = mesh->getVertexNormalsCoords();
			bool                       hasUVs   = mesh->getUVSetsCount() > 0;
			size_t                     uvsCount = 0;

			vertices.reserve(    vertices.size()     + verts.size());
			normals.reserve(     normals.size()      + norms.size());
			counts.reserve(      counts.size()       + mesh->getFaceCount());
			uvCounts.reserve(    uvCounts.size()     + mesh->getFaceCount());

			for(size_t i = 0, size = verts.size(); i < size; ++i)
				vertices.push_back(verts[i]);

			for(size_t i = 0, size = norms.size(); i < size; ++i)
				normals.push_back(norms[i]);

			if(hasUVs) {
				const prtx::DoubleVector& uvs = mesh->getUVCoords(0);
				uvsCount                      = uvs.size();

				tcsU.reserve(tcsU.size() + uvsCount / 2);
				tcsV.reserve(tcsV.size() + uvsCount / 2);

				for(size_t i = 0, size = uvsCount; i < size; i += 2) {
					tcsU.push_back((float)uvs[i+0]);
					tcsV.push_back((float)uvs[i+1]);
				}
			}
			log_trace("      copied vertex attributes");

			for(size_t fi = 0, faceCount = mesh->getFaceCount(); fi < faceCount; ++fi) {
				const uint32_t* vtxIdx = mesh->getFaceVertexIndices(fi);
				prtx::IndexVector vidxs(vtxIdx, vtxIdx + mesh->getFaceVertexCount(fi));

				counts.push_back((int)vidxs.size());
				for(size_t vi = 0, size = vidxs.size(); vi < size; ++vi)
					connects.push_back(base + vidxs[vi]);

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

			base   += (int)verts.size() / 3;
			uvBase += (int)uvsCount     / 2;

			log_trace("copied face attributes");
		}
	}

	bool hasUVS     = tcsU.size() > 0;
	bool hasNormals = normals.size() > 0;

	mayaOutput->setVertices(&vertices[0], vertices.size());
	log_trace("    set vertices");
	mayaOutput->setUVs(hasUVS ? &tcsU[0] : 0, hasUVS ? &tcsV[0] : 0, tcsU.size());
	log_trace("    set uvs");

	mayaOutput->setNormals(hasNormals ? &normals[0] : 0, normals.size());
	mayaOutput->setFaces(
			&counts[0], counts.size(),
			&connects[0], connects.size(),
			hasUVS ? &uvCounts[0]   : 0, hasUVS ? uvCounts.size()   : 0,
			hasUVS ? &uvConnects[0] : 0, hasUVS ? uvConnects.size() : 0
	);
	log_trace("set faces");

	mayaOutput->createMesh();
	log_trace("    maya output: created mesh");

	int startFace = 0;
	for(size_t gi = 0, geoCount = geometries.size(); gi < geoCount; ++gi) {
		prtx::Geometry* geo = geometries[gi].get();

		prtx::MaterialPtr mat = mats[gi].front();

		std::wostringstream matName;
		matName << "m" << cgbName << gi;

		log_wtrace(L"creating material: '%s'") % matName.str();

		int faceCount = 0;
		const prtx::MeshPtrVector& meshes = geo->getMeshes();
		for(size_t mi = 0, meshCount = meshes.size(); mi < meshCount; mi++)
			faceCount += (int)meshes[mi]->getFaceCount();

		std::wstring tex;
		if(mat->diffuseMap().size() > 0 && mat->diffuseMap()[0]->isValid()) {
			prtx::URIPtr texURI = mat->diffuseMap()[0]->getURI();
			log_wtrace(L"trying to set texture uri: %s") % texURI->wstring();
			std::wstring texPath = texURI->getPath();
			mayaOutput->matSetDiffuseTexture(startFace, faceCount, texPath.c_str());
		} else {
			mayaOutput->matSetColor(startFace, faceCount, mat->color_r(), mat->color_g(), mat->color_b());
		}

		startFace += faceCount;
	}
	mayaOutput->finishMesh();

	log_trace("MayaEncoder::convertGeometry: end");
}


void MayaEncoder::finish(prtx::GenerateContext& /*context*/) {
}

