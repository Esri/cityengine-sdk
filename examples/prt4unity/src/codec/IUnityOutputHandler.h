/**
 * Esri CityEngine SDK Unity Plugin Example
 *
 * This example demonstrates the main functionality of the Procedural Runtime API.
 * Esri R&D Center Zurich, Switzerland
 *
 * See http://github.com/ArcGIS/esri-cityengine-sdk for instructions.
 */

#ifndef IUNITY_OUTPUT_HANDLER_H_
#define IUNITY_OUTPUT_HANDLER_H_

#include "prt/Callbacks.h"

class IUnityOutputHandler : public prt::Callbacks {
public:
	virtual ~IUnityOutputHandler() { }

	/**
	 * Adds a new mesh.
	 *
	 * @param      name          Name of the mesh
	 * @param      numVertices   Number of vertices
	 * @param      vertices      Vertex positions (3*numVertices values)
	 * @param      normals       Vertex normals (3*numVertices values)
	 * @param      texcoords     Texture coordinates (2*numVertices values or NULL)
	 * @param      numSubMeshes  Number of sub meshes
	 * @param      numIndices    Number of indices in each sub mesh (numSubMeshes values)
	 * @param      indices       Indices of all sub meshes (count: sum of all numIndices)
	 * @param      materials     Material id for each sub mesh (numSubMeshes values)
	 */
	virtual void newMesh(const wchar_t* name, size_t numVertices, const float* vertices, const float* normals, const float* texcoords, size_t numSubMeshes, const uint32_t* numIndices, const uint32_t* indices, const uint32_t* materials) = 0;

	/**
	 * Adds a new material
	 *
	 * @param      id            Id of the material
	 * @param      name          Name of the material
	 */
	virtual void newMaterial(uint32_t id, const wchar_t* name) = 0;

	/**
	 * Sets a material's color
	 *
	 * @param      id            Id of the material
	 * @param      r             Red
	 * @param      g             Green
	 * @param      b             Blue
	 */
	virtual void matSetColor(uint32_t id, float r, float g, float b) = 0;

	/**
	 * Sets a material's diffuse texture
	 *
	 * @param      id            Id of the material
	 * @param      tex           Path of texture
	 */
	virtual void matSetDiffuseTexture(uint32_t id, const wchar_t* tex) = 0;

	/**
	 * Returns the special material for which a separate mesh should be returned (or multiple if necessitated by Unity's per-mesh vertex limit)
	 */
	virtual const wchar_t* getSpecialMaterial() = 0;
};

#endif /* IUNITY_OUTPUT_HANDLER_H_ */
