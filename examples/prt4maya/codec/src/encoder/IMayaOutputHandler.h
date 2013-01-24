#ifndef IMAYA_OUTPUT_HANDLER_H_
#define IMAYA_OUTPUT_HANDLER_H_

#include "prt/Callbacks.h"

class IMayaOutputHandler : public prt::Callbacks {
public:
	virtual ~IMayaOutputHandler() { }

	virtual void setVertices(double* vtx, size_t size) = 0;
	virtual void setNormals(double* nrm, size_t size) = 0;
	virtual void setUVs(float* u, float* v, size_t size) = 0;

	// connects is maya speak for indices
	virtual void setFaces(int* counts, size_t countsSize, int* connects, size_t connectsSize, int* normalCounts, size_t normalCountsSize, int* normalConnects, size_t normalConnectsSize, int* uvCounts, size_t uvCountsSize, int* uvConnects, size_t uvConnectsSize) = 0;
	virtual void createMesh() = 0;
	virtual void finishMesh() = 0;

	virtual int  matCreate(const wchar_t* name, int start, int count) = 0;
	virtual void matSetColor(int mh, float r, float g, float b) = 0;
	virtual void matSetDiffuseTexture(int mh, const wchar_t* tex) = 0;
};

#endif /* IMAYA_OUTPUT_HANDLER_H_ */

