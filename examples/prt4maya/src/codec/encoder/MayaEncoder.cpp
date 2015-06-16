/**
 * Esri CityEngine SDK Maya Plugin Example
 *
 * This example demonstrates the main functionality of the Procedural Runtime API.
 * Esri R&D Center Zurich, Switzerland
 *
 * See http://github.com/ArcGIS/esri-cityengine-sdk for instructions.
 */

#include "encoder/IMayaCallbacks.h"
#include "encoder/MayaEncoder.h"

#include "prtx/Exception.h"
#include "prtx/Log.h"
#include "prtx/Geometry.h"
#include "prtx/Material.h"
#include "prtx/Shape.h"
#include "prtx/ShapeIterator.h"
#include "prtx/ExtensionManager.h"
#include "prtx/GenerateContext.h"
#include "prtx/EncodePreparator.h"

#include <iostream>
#include <sstream>
#include <vector>
#include <numeric>
#include <algorithm>


namespace {

const bool DBG = true;

const prtx::EncodePreparator::PreparationFlags PREP_FLAGS = prtx::EncodePreparator::PreparationFlags()
	.instancing(false)
	.mergeByMaterial(true)
	.triangulate(false)
	.mergeVertices(false)
	.cleanupVertexNormals(false)
	.cleanupUVs(false)
	.processVertexNormals(prtx::VertexNormalProcessor::SET_MISSING_TO_FACE_NORMALS)
	.indexSharing(prtx::EncodePreparator::PreparationFlags::INDICES_SAME_FOR_VERTICES_AND_NORMALS);

std::wstring getBaseName(const wchar_t* path) {
	std::wstring bn{ path };
	size_t p = bn.find_last_of(L"/\\");
	size_t e = bn.find_last_of(L'.');
	if (e == std::wstring::npos || e < p)
		return bn.substr(p+1);
	return bn.substr(p+1, e);
}

} // namespace


const std::wstring MayaEncoder::ID          = L"MayaEncoder";
const std::wstring MayaEncoder::NAME        = L"Autodesk(tm) Maya(tm) Encoder";
const std::wstring MayaEncoder::DESCRIPTION	= L"Encodes geometry into Autodesk(tm) Maya(tm) format.";


MayaEncoder::MayaEncoder(const std::wstring& id, const prt::AttributeMap* options, prt::Callbacks* callbacks)
: prtx::GeometryEncoder(id, options, callbacks) { }

void MayaEncoder::init(prtx::GenerateContext&) {
	prt::Callbacks* cb = getCallbacks();
	IMayaCallbacks* oh = dynamic_cast<IMayaCallbacks*>(cb);
	if (oh == nullptr)
		throw prtx::StatusException(prt::STATUS_ILLEGAL_CALLBACK_OBJECT);
}

void MayaEncoder::encode(prtx::GenerateContext& context, size_t initialShapeIndex) {
	prtx::InitialShape const* initialShape = context.getInitialShape(initialShapeIndex);

	prtx::DefaultNamePreparator namePrep;
	prtx::NamePreparator::NamespacePtr nsMesh{ namePrep.newNamespace() };
	prtx::NamePreparator::NamespacePtr nsMaterial{ namePrep.newNamespace() };
	prtx::EncodePreparatorPtr encPrep = prtx::EncodePreparator::create(true, namePrep, nsMesh, nsMaterial);

	prtx::LeafIteratorPtr li = prtx::LeafIterator::create(context, initialShapeIndex);
	for (prtx::ShapePtr shape = li->getNext(); shape != nullptr; shape = li->getNext())
		encPrep->add(context.getCache(), shape, initialShape->getAttributeMap());

	prtx::GeometryPtrVector geometries;
	std::vector<prtx::DoubleVector> trafos;
	std::vector<prtx::MaterialPtrVector> materials;

	prtx::EncodePreparator::InstanceVector finalizedInstances;
	encPrep->fetchFinalizedInstances(finalizedInstances, PREP_FLAGS);
	for (const auto& fi: finalizedInstances) {
		geometries.emplace_back(fi.getGeometry());
		trafos.emplace_back(fi.getTransformation());
		materials.emplace_back(fi.getMaterials());
	}

	std::wstring cgbName = getBaseName(initialShape->getRuleFile());
	convertGeometry(cgbName, geometries, materials);
}

void MayaEncoder::convertGeometry(const std::wstring& cgbName, const prtx::GeometryPtrVector& geometries, const std::vector<prtx::MaterialPtrVector>& mats) {
	std::vector<double>	vertices, normals, tcsU, tcsV;
	std::vector<uint32_t> counts, connects, uvCounts, uvConnects;
	uint32_t base{0}, uvBase{0};

	for (const prtx::GeometryPtr& geo: geometries) {
		const prtx::MeshPtrVector& meshes = geo->getMeshes();
		for (const prtx::MeshPtr& mesh: meshes) {
			const uint32_t				faceCnt		= mesh->getFaceCount();
			const prtx::DoubleVector&	verts		= mesh->getVertexCoords();
			const prtx::DoubleVector&	norms		= mesh->getVertexNormalsCoords();
			bool						hasUVs		= mesh->getUVSetsCount() > 0;
			size_t						uvsCount	= 0;

			vertices.insert(vertices.end(), verts.cbegin(), verts.cend());
			normals.insert(normals.end(), norms.cbegin(), norms.cend());

			if (hasUVs) {
				const prtx::DoubleVector& uvs = mesh->getUVCoords(0);
				uvsCount = uvs.size();
				tcsU.reserve(tcsU.size() + uvsCount/2);
				tcsV.reserve(tcsV.size() + uvsCount/2);
				for(size_t i = 0, size = uvsCount; i < size; i += 2) {
					tcsU.push_back(uvs[i+0]);
					tcsV.push_back(uvs[i+1]);
				}
			}

			counts.reserve(counts.size() + faceCnt);
			uvCounts.reserve(uvCounts.size() + faceCnt);

			for (uint32_t fi = 0; fi < faceCnt; ++fi) {
				const uint32_t* vtxIdx = mesh->getFaceVertexIndices(fi);
				const uint32_t vtxCnt = mesh->getFaceVertexCount(fi);
				counts.push_back(vtxCnt);
				std::for_each(vtxIdx, vtxIdx + vtxCnt, [&connects,&base](uint32_t idx){ connects.push_back(base + idx); });

				if (hasUVs && mesh->getFaceUVCount(fi, 0) > 0) {
					assert(mesh->getFaceUVCount(fi, 0) == mesh->getFaceVertexCount(fi));
					const uint32_t* uv0Idx = mesh->getFaceUVIndices(fi, 0);
					const uint32_t uv0IdxCnt = mesh->getFaceUVCount(fi, 0);
					uvCounts.push_back(uv0IdxCnt);
					std::for_each(uv0Idx, uv0Idx + uv0IdxCnt, [&uvConnects,&uvBase](uint32_t uvIdx){ uvConnects.push_back(uvBase + uvIdx); });
				}
				else
					uvCounts.push_back(0);
			}

			base   += static_cast<uint32_t>(verts.size()) / 3;
			uvBase += static_cast<uint32_t>(uvsCount) / 2;
		}
	}

	IMayaCallbacks* mayaOutput = dynamic_cast<IMayaCallbacks*>(getCallbacks());

	mayaOutput->setVertices(vertices.data(), vertices.size());
	mayaOutput->setUVs(tcsU.data(), tcsV.data(), tcsU.size());

	mayaOutput->setNormals(normals.data(), normals.size());
	mayaOutput->setFaces(
			counts.data(), counts.size(),
			connects.data(), connects.size(),
			uvCounts.data(), uvCounts.size(),
			uvConnects.data(), uvConnects.size()
	);

	mayaOutput->createMesh();

	uint32_t startFace = 0;
	uint32_t geoIdx = 0;
	auto matIt = mats.cbegin();
	for (auto geoIt = geometries.cbegin(); geoIt != geometries.cend(); ++geoIt, ++matIt) {
		const prtx::GeometryPtr& geo = *geoIt;
		const prtx::MaterialPtr& mat = matIt->front();

		const prtx::MeshPtrVector& meshes = geo->getMeshes();
		uint32_t faceCount = std::accumulate(meshes.begin(), meshes.end(), 0, [](uint32_t c, const prtx::MeshPtr& m) {
			return c + m->getFaceCount();
		});

		std::wstring tex;
		if (mat->diffuseMap().size() > 0 && mat->diffuseMap().front()->isValid()) {
			prtx::URIPtr texURI = mat->diffuseMap().front()->getURI();
			std::wstring texPath = texURI->getPath();
			mayaOutput->matSetDiffuseTexture(startFace, faceCount, texPath.c_str());
		} else {
			mayaOutput->matSetColor(startFace, faceCount, mat->color_r(), mat->color_g(), mat->color_b());
		}

		startFace += faceCount;
	}

	mayaOutput->finishMesh();
}
