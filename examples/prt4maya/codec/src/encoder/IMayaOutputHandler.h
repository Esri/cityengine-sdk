/**
 * Esri CityEngine SDK Maya Plugin Example
 *
 * This example demonstrates the main functionality of the Procedural Runtime API.
 * Esri R&D Center Zurich, Switzerland
 *
 * See http://github.com/ArcGIS/esri-cityengine-sdk for instructions.
 */

#ifndef IMAYA_OUTPUT_HANDLER_H_
#define IMAYA_OUTPUT_HANDLER_H_

#include "prt/Callbacks.h"


class IMayaOutputHandler : public prt::Callbacks {
public:
	virtual ~IMayaOutputHandler() { }

	virtual void setVertices(double* vtx, size_t size) = 0;
	virtual void setNormals(double* nrm, size_t size) = 0;
	virtual void setUVs(float* u, float* v, size_t size) = 0;

	virtual void setFaces(int* counts, size_t countsSize, int* connects, size_t connectsSize, int* uvCounts, size_t uvCountsSize, int* uvConnects, size_t uvConnectsSize) = 0;
	virtual void createMesh() = 0;
	virtual void finishMesh() = 0;

	virtual void matSetColor(int start, int count, float r, float g, float b) = 0;
	virtual void matSetDiffuseTexture(int start, int count, const wchar_t* tex) = 0;
};

#endif /* IMAYA_OUTPUT_HANDLER_H_ */

