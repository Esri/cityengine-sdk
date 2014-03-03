/**
 * Esri CityEngine SDK Unity Plugin Example
 *
 * This example demonstrates the main functionality of the Procedural Runtime API.
 * Esri R&D Center Zurich, Switzerland
 *
 * See http://github.com/ArcGIS/esri-cityengine-sdk for instructions.
 */

#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <numeric>

#include <boost/filesystem/path.hpp>
#include <boost/foreach.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/unordered_map.hpp>

#include "prt/prt.h"

#include "prtx/Exception.h"
#include "prtx/Log.h"
#include "prtx/Geometry.h"
#include "prtx/Material.h"
#include "prtx/Shape.h"
#include "prtx/ShapeIterator.h"
#include "prtx/ExtensionManager.h"
#include "prtx/GenerateContext.h"

#include "IUnityOutputHandler.h"
#include "UnityEncoder.h"


const std::wstring UnityEncoder::ID          = L"com.esri.prt.examples.UnityEncoder";
const std::wstring UnityEncoder::NAME        = L"Unity Encoder";
const std::wstring UnityEncoder::DESCRIPTION = L"Encodes geometry into Unity-compatible meshes.";


UnityEncoder::UnityEncoder(const std::wstring& id, const prt::AttributeMap* options, prt::Callbacks* callbacks)
	: prtx::GeometryEncoder(id, options, callbacks)
{
}

UnityEncoder::~UnityEncoder() {
}

void UnityEncoder::init(prtx::GenerateContext& /*context*/) {
	// check whether a correct callback has been provided
	IUnityOutputHandler* outputHandler = dynamic_cast<IUnityOutputHandler*>(getCallbacks());
	if(outputHandler == NULL)
		throw(prtx::StatusException(prt::STATUS_ILLEGAL_CALLBACK_OBJECT));
}

void UnityEncoder::encode(prtx::GenerateContext& context, size_t initialShapeIndex) {
	const prtx::InitialShape* initialShape = context.getInitialShape(initialShapeIndex);
	IUnityOutputHandler* outputHandler = dynamic_cast<IUnityOutputHandler*>(getCallbacks());

	// Setup two namespaces for mesh and material objects and initialize the encode preprator.
	// The namespaces are used to create unique names for all mesh and material objects.
	prtx::DefaultNamePreparator namePrep;
	prtx::NamePreparator::NamespacePtr nsMesh     = namePrep.newNamespace();
	prtx::NamePreparator::NamespacePtr nsMaterial = namePrep.newNamespace();

	// Create the encode preparator and collect the resulting shapes. In case the shape generation fails, we collect the initial shape.
	prtx::EncodePreparatorPtr encodePreparator = prtx::EncodePreparator::create(true, namePrep, nsMesh, nsMaterial);
	try {
		prtx::LeafIteratorPtr li = prtx::LeafIterator::create(context, initialShapeIndex);
		for(prtx::ShapePtr shape = li->getNext(); shape.get() != 0; shape = li->getNext())
			encodePreparator->add(context.getCache(), shape);
	} catch(...) {
		encodePreparator->add(context.getCache(), *initialShape, initialShapeIndex);
	}

	// Prepare the collected shapes and retrieve the result
	prtx::EncodePreparator::PreparationFlags prepFlags;
	prepFlags.instancing(false);
	prepFlags.mergeByMaterial(true);
	prepFlags.triangulate(true);
	prepFlags.mergeVertices(true);
	prepFlags.cleanupVertexNormals(true);
	prepFlags.cleanupUVs(true);
	prepFlags.processVertexNormals(prtx::VertexNormalProcessor::SET_MISSING_TO_FACE_NORMALS);
	prepFlags.indexSharing(prtx::EncodePreparator::PreparationFlags::INDICES_SAME_FOR_ALL_VERTEX_ATTRIBUTES);
	std::vector<prtx::EncodePreparator::FinalizedInstance> finalizedInstances;
	encodePreparator->fetchFinalizedInstances(finalizedInstances, prepFlags);

	// Determine the rule file name
	size_t start = 0;
	size_t end   = 0;
	wchar_t* ruleFile = _wcsdup(initialShape->getRuleFile());
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

	// Convert geometry to representation that can be readily used in Unity and emit the resulting meshes and materials to the output handler
	convertGeometry(ruleFile + start, finalizedInstances, outputHandler);
	free(ruleFile);
}

void UnityEncoder::finish(prtx::GenerateContext& /*context*/) {
}


namespace {
	struct MaterialPtrComparator {
		bool operator()(const prtx::Material* a, const prtx::Material* b) const {
			if(a == 0)
				return b != 0;
			if(b == 0)
				return false;
			return *a < *b;
		}
	};

	struct MaterialMesh {
		const prtx::Mesh* mMesh;
		const prtx::Material* mMaterial;

		MaterialMesh(const prtx::Mesh* mesh, const prtx::Material* material)
			: mMesh(mesh), mMaterial(material) {}
	};
}

void UnityEncoder::convertGeometry(const wchar_t* cgbName, const std::vector<prtx::EncodePreparator::FinalizedInstance>& instances, IUnityOutputHandler* unityOutput) {
	const wchar_t* specialMaterial = unityOutput->getSpecialMaterial();
	if(specialMaterial != NULL && specialMaterial[0] == 0)
		specialMaterial = NULL;

	const size_t maxVerticesPerMesh = 65534;		// limit imposed by Unity

	// Compile FIFO list of (mesh, material) pairs
	std::vector<MaterialMesh> materialMeshes;
	std::vector<MaterialMesh> separateMaterialMeshes;
	for(std::vector<prtx::EncodePreparator::FinalizedInstance>::const_reverse_iterator itInst = instances.rbegin(), itInstEnd = instances.rend(); itInst != itInstEnd; ++itInst) {
		const prtx::Geometry* geo = itInst->getGeometry().get();
		const prtx::MeshPtrVector& meshes = geo->getMeshes();
		const prtx::MaterialPtrVector& materials = itInst->getMaterials();

		prtx::MaterialPtrVector::const_reverse_iterator itMaterial = materials.rbegin();
		for(prtx::MeshPtrVector::const_reverse_iterator itMesh = meshes.rbegin(), itMeshEnd = meshes.rend(); itMesh != itMeshEnd; ++itMesh, ++itMaterial) {
			const prtx::Material* material = itMaterial->get();
			if(specialMaterial != NULL && material->name() == specialMaterial)
				separateMaterialMeshes.push_back(MaterialMesh(itMesh->get(), material));
			else
				materialMeshes.push_back(MaterialMesh(itMesh->get(), material));
		}
	}

	std::vector<float> vertices;
	std::vector<float> normals;
	std::vector<float> texcoords;
	std::vector<uint32_t> indices;
	std::vector<uint32_t> subMeshIndexCounts;
	std::vector<uint32_t> subMeshMaterials;

	typedef std::map<const prtx::Material*, uint32_t, MaterialPtrComparator> MaterialMap;
	MaterialMap uniqueMaterials;

	// Output all meshes, greedily merging meshes to a single Unity mesh
	while(true) {
		// If all meshes with a normal material have been processed, proceed with a single special material at a time
		if(materialMeshes.empty()) {
			if(separateMaterialMeshes.empty())
				break;
			materialMeshes.push_back(separateMaterialMeshes.back());
			separateMaterialMeshes.pop_back();
		}

		size_t numVertices = 0;
		size_t numIndices = 0;
		size_t numSubMeshes = 0;
		bool hasUVs = false;

		vertices.clear();
		normals.clear();
		texcoords.clear();
		indices.clear();
		subMeshIndexCounts.clear();
		subMeshMaterials.clear();

		// If a single mesh exceeds the vertex limit, split it into multiple Unity meshes
		size_t numMeshVertices = materialMeshes.back().mMesh->getVertexCoords().size() / 3;
		if(numMeshVertices > maxVerticesPerMesh) {
			MaterialMesh mm = materialMeshes.back();
			materialMeshes.pop_back();

			if(mm.mMesh->getUVSetsCount() > 0 && mm.mMesh->getUVCoords(0).size() > 0)
				hasUVs = true;
			numSubMeshes = 1;

			typedef boost::unordered_map<uint32_t, uint32_t> IndexMap;
			IndexMap indexMap;
			boost::dynamic_bitset<> useVertices(numMeshVertices);
			const uint32_t numFaces = mm.mMesh->getFaceCount();

			for(uint32_t firstFace = 0, endFace; ; firstFace = endFace) {
				// Add faces until the number of referenced vertices exceeds the limit
				endFace = numFaces;
				uint32_t numUsedVertices = 0;
				for(uint32_t f = firstFace; f < numFaces; f++) {
					assert(mm.mMesh->getFaceVertexCount(f) == 3);
					const uint32_t* faceIndices = mm.mMesh->getFaceVertexIndices(f);
					uint32_t newVertices = 0;
					for(size_t i = 0; i < 3; i++) {
						if(!useVertices.test(faceIndices[i]))
							newVertices++;
					}
					if(numUsedVertices + newVertices <= maxVerticesPerMesh) {
						if(newVertices > 0) {
							useVertices.set(faceIndices[0]);
							useVertices.set(faceIndices[1]);
							useVertices.set(faceIndices[2]);
							numUsedVertices += newVertices;
						}
					} else {
						endFace = f;
						break;
					}
				}

				// Add vertex data for all referenced vertices
				indexMap.reserve(maxVerticesPerMesh);
				uint32_t nextIndex = 0;
				const prtx::DoubleVector& verts = mm.mMesh->getVertexCoords();
				const prtx::DoubleVector& norms = mm.mMesh->getVertexNormalsCoords();
				const prtx::DoubleVector* uvs = hasUVs ? &mm.mMesh->getUVCoords(0) : NULL;
				for(boost::dynamic_bitset<>::size_type i = useVertices.find_first(); i != useVertices.npos; i = useVertices.find_next(i)) {
					indexMap.insert(std::make_pair(static_cast<uint32_t>(i), nextIndex++));

					vertices.push_back(static_cast<float>(verts[i*3 + 0]));
					vertices.push_back(static_cast<float>(verts[i*3 + 1]));
					vertices.push_back(static_cast<float>(verts[i*3 + 2]));
					normals.push_back(static_cast<float>(norms[i*3 + 0]));
					normals.push_back(static_cast<float>(norms[i*3 + 1]));
					normals.push_back(static_cast<float>(norms[i*3 + 2]));
					if(hasUVs) {
						texcoords.push_back(static_cast<float>((*uvs)[i*2 + 0]));
						texcoords.push_back(static_cast<float>((*uvs)[i*2 + 1]));
					}
				}

				// Adjust and add indices for all included faces
				for(uint32_t f = firstFace; f < endFace; f++) {
					const uint32_t* faceIndices = mm.mMesh->getFaceVertexIndices(f);
					for(size_t i = 0; i < 3; i++)
						indices.push_back(indexMap[faceIndices[i]]);
				}
				subMeshIndexCounts.push_back(static_cast<uint32_t>(indices.size()));
				subMeshMaterials.push_back(uniqueMaterials.insert(std::make_pair(mm.mMaterial, static_cast<uint32_t>(uniqueMaterials.size()))).first->second);
				numVertices = numUsedVertices;
				numIndices = indices.size();

				// If there are remaining faces, output Unity mesh and continue with these
				if(endFace < numFaces) {
					unityOutput->newMesh(cgbName, numVertices, vertices.data(), normals.data(), hasUVs ? texcoords.data() : NULL, numSubMeshes, subMeshIndexCounts.data(), indices.data(), subMeshMaterials.data());

					useVertices.reset();
					indexMap.clear();
					vertices.clear();
					normals.clear();
					texcoords.clear();
					indices.clear();
					subMeshIndexCounts.clear();
					subMeshMaterials.clear();
				} else {
					break;
				}
			}
		}

		// Determine number of meshes that can be included before exceeding the vertex limit, moving these to the start of the FIFO list
		size_t numMeshesToInclude = 0;
		for(std::vector<MaterialMesh>::reverse_iterator it = materialMeshes.rbegin(), itEnd = materialMeshes.rend(), itLast = it; it != itEnd; ++it) {
			size_t numMeshVertices = it->mMesh->getVertexCoords().size() / 3;
			if(numMeshVertices + numVertices <= maxVerticesPerMesh) {
				numVertices += numMeshVertices;
				numIndices += it->mMesh->getFaceCount() * 3;
				if(it->mMesh->getUVSetsCount() > 0 && it->mMesh->getUVCoords(0).size() > 0)
					hasUVs = true;
				numMeshesToInclude++;

				if(itLast != it) {
					MaterialMesh mm = *it;
					for(std::vector<MaterialMesh>::reverse_iterator itDst = it; itDst != itLast; ) {
						std::vector<MaterialMesh>::reverse_iterator itSrc = itDst; --itSrc;
						*itDst = *itSrc;
						itDst = itSrc;
					}
					*itLast = mm;
				}
				++itLast;
			}
		}

		// Add these meshes to the new aggregate Unity mesh
		if(numMeshesToInclude > 0) {
			numSubMeshes += numMeshesToInclude;
			uint32_t firstVertex = static_cast<uint32_t>(vertices.size() / 3);
			vertices.reserve(numVertices * 3);
			normals.reserve(numVertices * 3);
			if(hasUVs) {
				texcoords.reserve(numVertices * 2);
				texcoords.resize(firstVertex * 2, 0.0f);
			}
			indices.reserve(numIndices);
			subMeshIndexCounts.reserve(numSubMeshes);
			subMeshMaterials.reserve(numSubMeshes);

			while(numMeshesToInclude-- > 0) {
				MaterialMesh mm = materialMeshes.back();
				materialMeshes.pop_back();

				const prtx::DoubleVector& verts = mm.mMesh->getVertexCoords();
				const size_t numMeshVertices = verts.size() / 3;
				for(prtx::DoubleVector::const_iterator it = verts.begin(), itEnd = verts.end(); it != itEnd; ++it)
					vertices.push_back(static_cast<float>(*it));

				const prtx::DoubleVector& norms = mm.mMesh->getVertexNormalsCoords();
				assert(norms.size() == numMeshVertices * 3);
				for(prtx::DoubleVector::const_iterator it = norms.begin(), itEnd = norms.end(); it != itEnd; ++it)
					normals.push_back(static_cast<float>(*it));

				if(hasUVs) {
					if(mm.mMesh->getUVSetsCount() > 0 && mm.mMesh->getUVCoords(0).size() > 0) {
						const prtx::DoubleVector& uvs = mm.mMesh->getUVCoords(0);
						assert(uvs.size() == numMeshVertices * 2);
						for(prtx::DoubleVector::const_iterator it = uvs.begin(), itEnd = uvs.end(); it != itEnd; ++it)
							texcoords.push_back(static_cast<float>(*it));
					} else {
						texcoords.insert(texcoords.end(), numMeshVertices * 2, 0.0f);
					}
				}

				const uint32_t numFaces = mm.mMesh->getFaceCount();
				for(uint32_t f = 0; f < numFaces; f++) {
					assert(mm.mMesh->getFaceVertexCount(f) == 3);
					const uint32_t* faceIndices = mm.mMesh->getFaceVertexIndices(f);
					for(size_t i = 0; i < 3; i++)
						indices.push_back(faceIndices[i] + firstVertex);
				}
				subMeshIndexCounts.push_back(numFaces * 3);
				subMeshMaterials.push_back(uniqueMaterials.insert(std::make_pair(mm.mMaterial, static_cast<uint32_t>(uniqueMaterials.size()))).first->second);

				firstVertex += static_cast<uint32_t>(numMeshVertices);
			}
		}

		// Output Unity mesh
		unityOutput->newMesh(cgbName, numVertices, vertices.data(), normals.data(), hasUVs ? texcoords.data() : NULL, numSubMeshes, subMeshIndexCounts.data(), indices.data(), subMeshMaterials.data());
	}

	// Enumerate materials according to their id
	std::vector<const prtx::Material*> materials(uniqueMaterials.size());
	for(MaterialMap::const_iterator it = uniqueMaterials.begin(); it != uniqueMaterials.end(); ++it)
		materials[it->second] = it->first;

	// Output materials
	for(size_t m = 0; m < materials.size(); m++) {
		const prtx::Material* material = materials[m];
		uint32_t id = static_cast<uint32_t>(m);
		unityOutput->newMaterial(id, material->name().c_str());
		unityOutput->matSetColor(id, static_cast<float>(material->color_r()), static_cast<float>(material->color_g()), static_cast<float>(material->color_b()));
		if(material->diffuseMap().size() > 0 && material->diffuseMap()[0]->isValid()) {
			prtx::URIPtr texURI = material->diffuseMap()[0]->getURI();
			std::wstring texPath = texURI->getPath();
			unityOutput->matSetDiffuseTexture(id, texPath.c_str());
		}
	}
}
