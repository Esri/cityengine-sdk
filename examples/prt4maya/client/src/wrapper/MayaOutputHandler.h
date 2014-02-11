/**
 * Esri CityEngine SDK Maya Plugin Example
 *
 * This example demonstrates the main functionality of the Procedural Runtime API.
 * Esri R&D Center Zurich, Switzerland
 *
 * See http://github.com/ArcGIS/esri-cityengine-sdk for instructions.
 */

#ifndef MAYA_OUTPUT_HANDLER_H_
#define MAYA_OUTPUT_HANDLER_H_

#include <stdexcept>
#include <map>
#include <string>

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
#include "prt/Cache.h"

class MayaOutputHandler : public IMayaOutputHandler {
public:
	class AttributeHolder {
	public:
		AttributeHolder() { }
		AttributeHolder(bool b, double d, std::wstring s) : mBool(b), mFloat(d), mString(s) { }
		virtual ~AttributeHolder() {  }
		bool         mBool;
		double       mFloat;
		std::wstring mString;
	};

public:
	MayaOutputHandler(const MPlug* plug, MDataBlock* data, MStringArray* shadingGroups, MIntArray* shadingRanges) :
		mPlug(plug), mData(data), mShadingGroups(shadingGroups), mShadingRanges(shadingRanges)
	{ }
	virtual ~MayaOutputHandler() {}

	// prt::Callbacks interface
	virtual prt::Status generateError(size_t /*isIndex*/, prt::Status /*status*/, const wchar_t* message) {
		std::wcout << "GENERATE ERROR: " << message << std::endl;
		return prt::STATUS_OK;
	}
	virtual prt::Status assetError(size_t /*isIndex*/, prt::CGAErrorLevel /*level*/, const wchar_t* /*key*/, const wchar_t* /*uri*/, const wchar_t* message) {
		std::wcout << "ASSET ERROR: " << message << std::endl;
		return prt::STATUS_OK;
	}
	virtual prt::Status cgaError(size_t /*isIndex*/, int32_t /*shapeID*/, prt::CGAErrorLevel /*level*/, int32_t /*methodId*/, int32_t /*pc*/, const wchar_t* message) {
		std::wcout << "CGA ERROR: " << message << std::endl;
		return prt::STATUS_OK;
	}
	virtual prt::Status cgaPrint(size_t /*isIndex*/, int32_t /*shapeID*/, const wchar_t* /*txt*/) {
		return prt::STATUS_OK;
	}
	virtual prt::Status cgaReportBool(size_t /*isIndex*/, int32_t /*shapeID*/, const wchar_t* /*key*/, bool /*value*/) {
		return prt::STATUS_OK;
	}
	virtual prt::Status cgaReportFloat(size_t /*isIndex*/, int32_t /*shapeID*/, const wchar_t* /*key*/, double /*value*/) {
		return prt::STATUS_OK;
	}
	virtual prt::Status cgaReportString(size_t /*isIndex*/, int32_t /*shapeID*/, const wchar_t* /*key*/, const wchar_t* /*value*/) {
		return prt::STATUS_OK;
	}
	virtual prt::Status attrBool(size_t /*isIndex*/, int32_t /*shapeID*/, const wchar_t* /*key*/, bool /*value*/);
	virtual prt::Status attrFloat(size_t /*isIndex*/, int32_t /*shapeID*/, const wchar_t* /*key*/, double /*value*/);
	virtual prt::Status attrString(size_t /*isIndex*/, int32_t /*shapeID*/, const wchar_t* /*key*/, const wchar_t* /*value*/);

public:
	virtual void setVertices(double* vtx, size_t size);
	virtual void setNormals(double* nrm, size_t size);
	virtual void setUVs(float* u, float* v, size_t size);

	virtual void setFaces(
			int* 	 counts,
			size_t countsSize,
			int*	 connects,
			size_t connectsSize,
			int*	 normalCounts,
			size_t normalCountsSize,
			int*   normalConnects,
			size_t normalConnectsSize,
			int*   uvCounts,
			size_t uvCountsSize,
			int*   uvConnects,
			size_t uvConnectsSize
	);
	virtual void createMesh();
	virtual void finishMesh();

	virtual int  matCreate(const wchar_t* name, int start, int count);
	virtual void matSetColor(int mh, float r, float g, float b);
	virtual void matSetDiffuseTexture(int mh, const wchar_t* tex);

public:
	const std::map<std::wstring, AttributeHolder>& getAttrs() const { return mAttrs; }

public:
	MFnMesh*			    mFnMesh;

	MFloatPointArray		mVertices;
	MIntArray			    mVerticesCounts;
	MIntArray			    mVerticesConnects;

	MFloatVectorArray		mNormals;
	MIntArray			    mNormalCounts; // not actually necessary, as the encoder ensures same indexing for vertices and vertex normals
	MIntArray			    mNormalConnects; // not actually necessary, as the encoder ensures same indexing for vertices and vertex normals

	MFloatArray			  	mU;
	MFloatArray			  	mV;
	MIntArray			    mUVCounts;
	MIntArray			    mUVConnects;

private:
	// must not be called
	MayaOutputHandler() : mPlug(0), mData(0), mShadingGroups(0), mShadingRanges(0) { }

	const MPlug*		  mPlug;
	MDataBlock*			  mData;
	MStringArray*		  mShadingGroups;
	MIntArray*		    mShadingRanges;

	std::map<std::wstring, AttributeHolder> mAttrs;
};


#endif /* MAYA_OUTPUT_HANDLER_H_ */
