/**
 * Esri CityEngine SDK Maya Plugin Example
 *
 * This example demonstrates the main functionality of the Procedural Runtime API.
 * Esri R&D Center Zurich, Switzerland
 *
 * See http://github.com/ArcGIS/esri-cityengine-sdk for instructions.
 */

#pragma once

#include "prt/Callbacks.h"
#include "prtx/Material.h"

class IMayaCallbacks : public prt::Callbacks {
public:
	virtual ~IMayaCallbacks() { }

	virtual void setVertices(const double* vtx, size_t size) = 0;
	virtual void setNormals(const double* nrm, size_t size) = 0;
	virtual void setUVs(const double* u, const double* v, size_t size) = 0;
    virtual void setMaterial(uint32_t start, uint32_t count, const prtx::MaterialPtr& mat) = 0;

	virtual void setFaces(
			const uint32_t* counts, size_t countsSize,
			const uint32_t* connects, size_t connectsSize,
			const uint32_t* uvCounts, size_t uvCountsSize,
			const uint32_t* uvConnects, size_t uvConnectsSize
	) = 0;

	virtual void createMesh() = 0;
	virtual void finishMesh() = 0;

};
