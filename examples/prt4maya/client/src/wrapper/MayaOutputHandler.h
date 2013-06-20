/*
 * MayaData.h
 *
 *  Created on: Oct 12, 2012
 *      Author: shaegler
 */

#ifndef MAYA_OUTPUT_HANDLER_H_
#define MAYA_OUTPUT_HANDLER_H_

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

#include "IMayaOutputHandler.h"


class MayaOutputHandler : public IMayaOutputHandler {
public:
	MayaOutputHandler(const MPlug* plug, MDataBlock* data, MStringArray* shadingGroups, MIntArray* shadingRanges) :
		mPlug(plug), mData(data), mShadingGroups(shadingGroups), mShadingRanges(shadingRanges), mCache(prt::CacheBuilder::createCache(prt::CacheBuilder::CACHE_TYPE_DEFAULT))
	{ }
	virtual ~MayaOutputHandler() {
		if(mCache) mCache->destroy();
	}

	virtual prt::Status assetError(size_t isIndex, prt::CGAErrorLevel level, const wchar_t* key, const wchar_t* uri, const wchar_t* message) { throw std::runtime_error("Not implemented yet"); return prt::STATUS_OK; }
	virtual prt::Status generateError(size_t /*isIndex*/, const wchar_t* /* message*/) { throw std::runtime_error("Not implemented yet"); return prt::STATUS_OK; }
	virtual prt::Status cgaError(size_t isIndex, int32_t shapeID, prt::CGAErrorLevel level, int32_t methodId, int32_t pc, const wchar_t* message) { throw std::runtime_error("Not implemented yet"); return prt::STATUS_OK; }
	virtual prt::Status cgaPrint(size_t isIndex, int32_t shapeID, const wchar_t* txt) { throw std::runtime_error("Not implemented yet"); return prt::STATUS_OK; }
	virtual prt::Status cgaReportBool(size_t isIndex, int32_t shapeID, const wchar_t* key, bool value) { throw std::runtime_error("Not implemented yet"); return prt::STATUS_OK; }
	virtual prt::Status cgaReportFloat(size_t isIndex, int32_t shapeID, const wchar_t* key, double value) { throw std::runtime_error("Not implemented yet"); return prt::STATUS_OK; }
	virtual prt::Status cgaReportString(size_t isIndex, int32_t shapeID, const wchar_t* key, const wchar_t* value) { throw std::runtime_error("Not implemented yet"); return prt::STATUS_OK; }

	virtual prt::Status openCGAError() { throw std::runtime_error("Not implemented yet"); return prt::STATUS_OK; }
	virtual prt::Status openCGAPrint() { throw std::runtime_error("Not implemented yet"); return prt::STATUS_OK; }
	virtual prt::Status openCGAReport() { throw std::runtime_error("Not implemented yet"); return prt::STATUS_OK; }
	virtual prt::Status closeCGAError() { throw std::runtime_error("Not implemented yet"); return prt::STATUS_OK; }
	virtual prt::Status closeCGAPrint() { throw std::runtime_error("Not implemented yet"); return prt::STATUS_OK; }
	virtual prt::Status closeCGAReport() { throw std::runtime_error("Not implemented yet"); return prt::STATUS_OK; }

	virtual const void* getTransientBlob(prt::ContentType type, const wchar_t* key) {
		return mCache->getTransientBlob(type, key);
	}
	virtual const void* insertTransientBlob(prt::ContentType type, const wchar_t* key, const void* ptr) {
		return mCache->insertTransientBlob(type, key, ptr);
	}
	virtual bool tryLockPersistentBlobs(prt::ContentType type, const wchar_t* key) {
		return mCache->tryLockPersistentBlobs(type, key);
	}
	virtual void insertPersistentBlobAndLock(PersistentBlobType type, const wchar_t* key, const void* data, size_t size) {
		mCache->insertPersistentBlobAndLock(type, key, data, size);
	}
	virtual const void* getPersistentBlob(PersistentBlobType type, const wchar_t* key, size_t* size) {
		return mCache->getPersistentBlob(type, key, size);
	}
	virtual void releasePersistentBlob(PersistentBlobType type, const wchar_t* key) {
		mCache->releasePersistentBlob(type, key);
	}
	virtual void unlockPersistentBlob(PersistentBlobType type, const wchar_t* key) {
		mCache->unlockPersistentBlob(type, key);
	}


public:
	virtual void setVertices(double* vtx, size_t size);
	virtual void setNormals(double* nrm, size_t size);
	virtual void setUVs(float* u, float* v, size_t size);

	virtual void setFaces(int* counts, size_t countsSize, int* connects, size_t connectsSize, int* normalCounts, size_t normalCountsSize, int* normalConnects, size_t normalConnectsSize, int* uvCounts, size_t uvCountsSize, int* uvConnects, size_t uvConnectsSize);
	virtual void createMesh();
	virtual void finishMesh();

	virtual int  matCreate(const wchar_t* name, int start, int count);
	virtual void matSetColor(int mh, float r, float g, float b);
	virtual void matSetDiffuseTexture(int mh, const wchar_t* tex);

public:
	MFnMesh*			    mFnMesh;

	MFloatPointArray	mVertices;
	MIntArray			    mCounts;
	MIntArray			    mConnects;

	MFloatPointArray	mNormals;
	MIntArray			    mNormalCounts;
	MIntArray			    mNormalConnects;

	MFloatArray			  mU;
	MFloatArray			  mV;
	MIntArray			    mUVCounts;
	MIntArray			    mUVConnects;

private:
	// must not be called
	MayaOutputHandler() : mPlug(0), mData(0), mShadingGroups(0), mShadingRanges(0) { }

	const MPlug*		  mPlug;
	MDataBlock*			  mData;
	MStringArray*		  mShadingGroups;
	MIntArray*		      mShadingRanges;

	prt::Cache*			  mCache;
};


#endif /* MAYA_OUTPUT_HANDLER_H_ */
