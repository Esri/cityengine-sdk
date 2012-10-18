/*
 * MayaData.h
 *
 *  Created on: Oct 12, 2012
 *      Author: shaegler
 */

#ifndef MAYADATA_H_
#define MAYADATA_H_

#include "maya/MDataHandle.h"
#include "maya/MStatus.h"
#include "maya/MObject.h"
#include "maya/MDoubleArray.h"
#include "maya/MPointArray.h"
#include "maya/MPoint.h"
#include "maya/MString.h"
#include "maya/MFileIO.h"
#include "maya/MLibrary.h"
#include "maya/MIOStream.h"
#include "maya/MGlobal.h"
#include "maya/MStringArray.h"
#include "maya/MFloatArray.h"
#include "maya/MFloatPoint.h"
#include "maya/MFloatPointArray.h"
#include "maya/MDataBlock.h"
#include "maya/MDataHandle.h"
#include "maya/MIntArray.h"
#include "maya/MDoubleArray.h"
#include "maya/MLibrary.h"
#include "maya/MPlug.h"
#include "maya/MDGModifier.h"
#include "maya/MSelectionList.h"
#include "maya/MDagPath.h"
#include "maya/MFileObject.h"

#include "maya/MFnNurbsSurface.h"
#include "maya/MFnMesh.h"
#include "maya/MFnMeshData.h"
#include "maya/MFnLambertShader.h"
#include "maya/MFnTransform.h"
#include "maya/MFnSet.h"
#include "maya/MFnPartition.h"

#include "IMayaData.h"


class MayaData : public IMayaData {
public:
	MayaData(const MPlug* plug, MDataBlock* data, MStringArray* shadingGroups, MIntArray* shadingRanges) : mPlug(plug), mData(data), mShadingGroups(shadingGroups), mShadingRanges(shadingRanges) { }
	virtual ~MayaData() { }

public:
	virtual void setVertices(double* vtx, size_t size);
	virtual void setNormals(double* nrm, size_t size);
	virtual void setUVs(float* u, float* v, size_t size);

	virtual void setFaces(int* counts, size_t countsSize, int* connects, size_t connectsSize, int* tcConnects, size_t tcConnectsSize);
	virtual void createMesh();

public: // FIXME
	const MPlug*		mPlug;
	MDataBlock*			mData;
	MStringArray*		mShadingGroups;
	MIntArray*			mShadingRanges;


	MFloatPointArray	mVertices;
	MIntArray			mCounts;
	MIntArray			mConnects;

	MFloatArray			mU;
	MFloatArray			mV;
	MIntArray			mUVConnects;

private:
	// must not be called
	MayaData() : mPlug(0), mData(0), mShadingGroups(0), mShadingRanges(0) { }
};


#endif /* MAYADATA_H_ */
