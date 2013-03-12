/*
 * MayaData.cpp
 *
 *  Created on: Oct 12, 2012
 *      Author: shaegler
 */

#include <cassert>

#include "maya/MItMeshPolygon.h"
#include "wrapper/MayaOutputHandler.h"
#include "prt4mayaNode.h"

#define MCHECK(_stat_) {if(MS::kSuccess != _stat_) {DBG("maya err at line %d: %s %d\n", __LINE__, _stat_.errorString().asChar(), _stat_.statusCode());}}


void MayaOutputHandler::setVertices(double* vtx, size_t size) {
	mVertices.clear();
	for (size_t i = 0; i < size; i += 3)
		mVertices.append((float)vtx[i], (float)vtx[i+1], (float)vtx[i+2]);
}


void MayaOutputHandler::setNormals(double* nrm, size_t size) {
	mNormals.clear();
	for (size_t i = 0; i < size; i += 3)
		mNormals.append((float)nrm[i], (float)nrm[i+1], (float)nrm[i+2]);
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
	mCounts.clear();
	for (size_t i = 0; i < countsSize; ++i)
		mCounts.append(counts[i]);

	mConnects.clear();
	for (size_t i = 0; i < connectsSize; ++i)
		mConnects.append(connects[i]);

	mNormalCounts.clear();
	for (size_t i = 0; i < normalCountsSize; ++i)
		mNormalCounts.append(normalCounts[i]);

	mNormalConnects.clear();
	for (size_t i = 0; i < normalConnectsSize; ++i)
		mNormalConnects.append(normalConnects[i]);

	mUVCounts.clear();
	for (size_t i = 0; i < uvCountsSize; ++i)
		mUVCounts.append(uvCounts[i]);

	mUVConnects.clear();
	for (size_t i = 0; i < uvConnectsSize; ++i)
		mUVConnects.append(uvConnects[i]);

	assert(mCounts.length() == mNormalCounts.length());
	assert(mCounts.length() == mUVCounts.length());
}


// maya api tutorial: http://ewertb.soundlinker.com/maya.php
void MayaOutputHandler::createMesh() {
	MStatus stat;

	std::cout << "--- MayaData::createMesh begin" << std::endl;

	if(mPlug == 0 || mData == 0) return;

	MDataHandle outputHandle = mData->outputValue(*mPlug, &stat);
	MCHECK(stat);

	MFnMeshData dataCreator;
	MObject newOutputData = dataCreator.create(&stat);
	MCHECK(stat);

	mFnMesh = new MFnMesh();
	MObject oMesh = mFnMesh->create(mVertices.length(), mCounts.length(), mVertices, mCounts, mConnects, newOutputData, &stat);
	MCHECK(stat);

	int ni = 0;
	int vi = 0;
	for(unsigned int fi = 0; fi < mCounts.length(); ++fi) {
		if (mNormalCounts[fi] > 0) {
			for (int v = 0; v < mNormalCounts[fi]; ++v, ++ni, ++vi) {
				MVector nrm(mNormals[mNormalConnects[ni]]);
				std::cout << "fi = " << fi << ", v = " << v << ", vtx index = " << mConnects[vi] << ", nrm index = " << mNormalConnects[ni] << ", nrm = " << nrm << std::endl;
				stat = mFnMesh->setFaceVertexNormal(nrm, fi, mConnects[vi]);
				MCHECK(stat);
			}
		}
		else
			vi += mCounts[fi];
	}

	MPlugArray plugs;
	bool isConnected = mPlug->connectedTo(plugs, false, true, &stat);
	MCHECK(stat);
	DBG("    plug is connected: %d; %d plugs\n", isConnected, plugs.length());
	if (plugs.length() > 0) {
		if(mUVConnects.length() > 0) {
			MString uvSet = "map1";

			stat = mFnMesh->setUVs(mU, mV, &uvSet);
			MCHECK(stat);

			stat = mFnMesh->assignUVs(mUVCounts, mUVConnects, &uvSet);
			MCHECK(stat);
		}
	}

	mShadingGroups->clear();
	mShadingRanges->clear();

	stat = outputHandle.set(newOutputData);
	MCHECK(stat);

	DBG("    mayaVertices.length = %d", mVertices.length());
	DBG("    mayaCounts.length   = %d", mCounts.length());
	DBG("    mayaConnects.length = %d", mConnects.length());
}


int MayaOutputHandler::matCreate(const wchar_t* name, int start, int count) {
	MStatus stat;

	MString cmd("createShadingGroup(\"");
	cmd += name;
	cmd += "\")";
	MString result = MGlobal::executeCommandStringResult(cmd, false, false, &stat);
	DBG("mel cmd '%s' executed, result = '%s'", cmd.asChar(), result.asChar());

	mShadingGroups->append(result);
	mShadingRanges->append(start);
	mShadingRanges->append(start + count - 1);

	return mShadingGroups->length() - 1;
}


void MayaOutputHandler::matSetDiffuseTexture(int mh, const wchar_t* tex) {
	MStatus stat;

	MString sg = (*mShadingGroups)[mh];
	MString cmd("prtSetDiffuseTexture(\"");
	cmd += sg + "\", \"" + tex + "\", \"map1\")";
	MString result = MGlobal::executeCommandStringResult(cmd, false, false, &stat);
	DBG("mel cmd '%s' executed, result = '%s'", cmd.asChar(), result.asChar());
}


void MayaOutputHandler::matSetColor(int mh, float r, float g, float b) {

}


void MayaOutputHandler::finishMesh() {
	delete mFnMesh;
}
