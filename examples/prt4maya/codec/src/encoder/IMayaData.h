#ifndef IMAYA_DATA_H_
#define IMAYA_DATA_H_

class IMayaData {
public:
	virtual ~IMayaData() { }

	virtual void setVertices(double* vtx, size_t size) = 0;
	virtual void setNormals(double* nrm, size_t size) = 0;
	virtual void setUVs(float* u, float* v, size_t size) = 0;

	virtual void setFaces(int* counts, size_t countsSize, int* connects, size_t connectsSize, int* tcConnects, size_t tcConnectsSize) = 0;
	virtual void createMesh() = 0;
};

#endif /* IMAYA_DATA_H_ */

