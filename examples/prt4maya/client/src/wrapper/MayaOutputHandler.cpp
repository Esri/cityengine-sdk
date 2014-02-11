/**
 * Esri CityEngine SDK Maya Plugin Example
 *
 * This example demonstrates the main functionality of the Procedural Runtime API.
 * Esri R&D Center Zurich, Switzerland
 *
 * See http://github.com/ArcGIS/esri-cityengine-sdk for instructions.
 */

#include <cassert>
#include <sstream>
#include <ostream>

#include "maya/MItMeshPolygon.h"

#include "wrapper/MayaOutputHandler.h"

#include "prt4maya/Utilities.h"
#include "prt4maya/PRTNode.h"

namespace {
static const bool TRACE = false;
void prtTrace(const std::wstring& arg1, std::size_t arg2) {
	if (TRACE) {
		std::wostringstream wostr;
		wostr << L"[MOH] "<< arg1 << arg2;
		prt::log(wostr.str().c_str(), prt::LOG_TRACE);
	}
}
}

void MayaOutputHandler::setVertices(double* vtx, size_t size) {
	prtTrace(L"setVertices: size = ", size);
	mVertices.clear();
	for (size_t i = 0; i < size; i += 3)
		mVertices.append((float)vtx[i], (float)vtx[i+1], (float)vtx[i+2]);
}


void MayaOutputHandler::setNormals(double* nrm, size_t size) {
	prtTrace(L"setNormals: size = ", size);
	mNormals.clear();
	for (size_t i = 0; i < size; i += 3)
		mNormals.append(MVector((float)nrm[i], (float)nrm[i+1], (float)nrm[i+2]));
}


void MayaOutputHandler::setUVs(float* u, float* v, size_t size) {
	mU.clear();
	mV.clear();
	for (size_t i = 0; i < size; ++i) {
		mU.append(u[i]);
		mV.append(v[i]);
	}
}

void MayaOutputHandler::setFaces(int* counts, size_t countsSize, int* connects, size_t connectsSize, int* normalCounts, size_t normalCountsSize, int* normalConnects, size_t normalConnectsSize, int* uvCounts, size_t uvCountsSize, int* uvConnects, size_t uvConnectsSize) {
	mVerticesCounts.clear();
	for (size_t i = 0; i < countsSize; ++i)
		mVerticesCounts.append(counts[i]);
	prtTrace(L"countsSize = ", countsSize);

	mVerticesConnects.clear();
	for (size_t i = 0; i < connectsSize; ++i)
		mVerticesConnects.append(connects[i]);
	prtTrace(L"connectsSize = ", connectsSize);

	mNormalCounts.clear();
	for (size_t i = 0; i < normalCountsSize; ++i)
		mNormalCounts.append(normalCounts[i]);
	prtTrace(L"normalCountsSize = ", normalCountsSize);

	mNormalConnects.clear();
	for (size_t i = 0; i < normalConnectsSize; ++i)
		mNormalConnects.append(normalConnects[i]);
	prtTrace(L"normalConnectsSize = ", normalConnectsSize);

	mUVCounts.clear();
	for (size_t i = 0; i < uvCountsSize; ++i)
		mUVCounts.append(uvCounts[i]);

	mUVConnects.clear();
	for (size_t i = 0; i < uvConnectsSize; ++i)
		mUVConnects.append(uvConnects[i]);
}

#define VERBOSE_NORMALS 0

// maya api tutorial: http://ewertb.soundlinker.com/maya.php
void MayaOutputHandler::createMesh() {
	MStatus stat;

	DBGL(L"--- MayaData::createMesh begin");

	if(mPlug == 0 || mData == 0) return;

	MDataHandle outputHandle = mData->outputValue(*mPlug, &stat);
	MCHECK(stat);

	MFnMeshData dataCreator;
	MObject newOutputData = dataCreator.create(&stat);
	MCHECK(stat);

	DBGL(L"    mVertices.length         = %d", mVertices.length());
	DBGL(L"    mVerticesCounts.length   = %d", mVerticesCounts.length());
	DBGL(L"    mVerticesConnects.length = %d", mVerticesConnects.length());

	mFnMesh = new MFnMesh();
	MObject oMesh = mFnMesh->create(mVertices.length(), mVerticesCounts.length(), mVertices, mVerticesCounts, mVerticesConnects, newOutputData, &stat);
	MCHECK(stat);

	MPlugArray plugs;
	bool isConnected = mPlug->connectedTo(plugs, false, true, &stat);
	MCHECK(stat);
	DBGL(L"    plug is connected: %d; %d plugs", isConnected, plugs.length());
	if (plugs.length() > 0) {
		if(mUVConnects.length() > 0) {
			MString uvSet = "map1";

			MCHECK(mFnMesh->setUVs(mU, mV, &uvSet));

			DBGL(L"    mU.length          = %d", mU.length());
			DBGL(L"    mV.length          = %d", mV.length());
			DBGL(L"    mUVCounts.length   = %d", mUVCounts.length());
			DBGL(L"    mUVConnects.length = %d", mUVConnects.length());

			MCHECK(mFnMesh->assignUVs(mUVCounts, mUVConnects, &uvSet));
		}
	}

	mShadingGroups->clear();
	mShadingRanges->clear();

	if(mNormals.length() > 0) {
		DBGL(L"    mNormals.length        = %d", mNormals.length());
		DBGL(L"    mNormalCounts.length   = %d", mNormalCounts.length());
		DBGL(L"    mNormalConnects.length = %d", mNormalConnects.length());

#if VERBOSE_NORMALS == 1
		{
			size_t nc = 0;
			for (size_t i = 0; i < mNormalCounts.length(); i++) {
				DBGL(L"    face %d: #vtx = %d", i, mNormalCounts[i]);
				for (size_t ni = 0; ni < mNormalCounts[i]; ni++) {
					size_t vidx = mVerticesConnects[nc];
					size_t nidx = mNormalConnects[nc++];
					DBGL(L"      vidx = %d; mNormals[%d] = (%f, %f, %f)", vidx, nidx, mNormals[nidx].x, mNormals[nidx].y, mNormals[nidx].z);
				}
			}
		}

		{
			MFloatVectorArray exNormals;
			MCHECK(mFnMesh->getNormals(exNormals));
			DBGL(L"    exNormals.length = %d", exNormals.length());

			MIntArray exVertexIds, exVertexIdCounts;
			MCHECK(mFnMesh->getVertices(exVertexIdCounts, exVertexIds));

			MIntArray exNormalIds, exNormalIdCnts;
			MCHECK(mFnMesh->getNormalIds(exNormalIdCnts, exNormalIds));

			size_t nc = 0;
			for (size_t i = 0; i < exNormalIdCnts.length(); i++) {
				DBGL(L"    ex face %d: #vtx = %d", i, exNormalIdCnts[i]);
				for (size_t ni = 0; ni < exNormalIdCnts[i]; ni++) {
					size_t vidx = exVertexIds[nc];
					size_t nidx = exNormalIds[nc++];
					DBGL(L"      vidx = %d; exNormals[%d] = (%f, %f, %f)", vidx, nidx, exNormals[nidx].x, exNormals[nidx].y, exNormals[nidx].z);
				}
			}
		}
#endif

		// NOTE: this expects that vertices and vertex normals use the same index domain (-> maya encoder)
		MVectorArray expandedNormals(mVerticesConnects.length());
		for (size_t i = 0; i < mVerticesConnects.length(); i++) {
			expandedNormals[i] = mNormals[mVerticesConnects[i]];
		}
		DBGL(L"    expandedNormals.length = %d", expandedNormals.length());

		MCHECK(mFnMesh->setVertexNormals(expandedNormals, mVerticesConnects));
	} // hasNormals?

	MMeshSmoothOptions smoothOpts;
	smoothOpts.setSmoothness(0.0f);
	smoothOpts.setKeepHardEdge(true);
	smoothOpts.setPropEdgeHardness(true);
	smoothOpts.setKeepBorderEdge(true);
	mFnMesh->setSmoothMeshDisplayOptions(smoothOpts);

	stat = outputHandle.set(newOutputData);
	MCHECK(stat);
}


int MayaOutputHandler::matCreate(const wchar_t* name, int start, int count) {
	MStatus stat;

	MString cmd("createShadingGroup(\"");
	cmd += name;
	cmd += "\")";
	MString result = MGlobal::executeCommandStringResult(cmd, false, false, &stat);
	DBGL(L"mel cmd '%s' executed, result = '%s'", cmd.asChar(), result.asChar());

	mShadingGroups->append(result);
	mShadingRanges->append(start);
	mShadingRanges->append(start + count - 1);

	return mShadingGroups->length() - 1;
}


void MayaOutputHandler::matSetDiffuseTexture(int mh, const wchar_t* tex) {
	MStatus stat;

	MString sg = (*mShadingGroups)[mh];
	MString cmd("prtSetDiffuseTexture(\"");
	cmd += sg + "\",\"" + tex + "\",\"map1\")";
	MString result = MGlobal::executeCommandStringResult(cmd, false, false, &stat);
	DBGL(L"mel cmd '%s' executed, result = '%s'", cmd.asChar(), result.asChar());
}


void MayaOutputHandler::matSetColor(int mh, float r, float g, float b) {
	MStatus stat;

	MString sg = (*mShadingGroups)[mh];
	MString cmd("prtSetColor(\"");
	cmd += sg + "\"," + r + "," + g + "," + b + ")";
	MString result = MGlobal::executeCommandStringResult(cmd, false, false, &stat);
	DBGL(L"mel cmd '%s' executed, result = '%s'", cmd.asChar(), result.asChar());
}


void MayaOutputHandler::finishMesh() {
	delete mFnMesh;
}


prt::Status MayaOutputHandler::attrBool(size_t /*isIndex*/, int32_t /*shapeID*/, const wchar_t* key, bool value) {
	mAttrs[stripStyle(key)].mBool = value;
	return prt::STATUS_OK;
}


prt::Status MayaOutputHandler::attrFloat(size_t /*isIndex*/, int32_t /*shapeID*/, const wchar_t* key, double value) {
	mAttrs[stripStyle(key)].mFloat = value;
	return prt::STATUS_OK;
}


prt::Status MayaOutputHandler::attrString(size_t /*isIndex*/, int32_t /*shapeID*/, const wchar_t* key, const wchar_t* value) {
	mAttrs[stripStyle(key)].mString = value;
	return prt::STATUS_OK;
}
