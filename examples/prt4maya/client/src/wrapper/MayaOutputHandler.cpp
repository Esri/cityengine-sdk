/**
 * Esri CityEngine SDK Maya Plugin Example
 *
 * This example demonstrates the main functionality of the Procedural Runtime API.
 * Esri R&D Center Zurich, Switzerland
 *
 * See http://github.com/ArcGIS/esri-cityengine-sdk for instructions.
 */

#include <cassert>

#include "maya/MItMeshPolygon.h"
#include "wrapper/MayaOutputHandler.h"
#include "prt4maya/prt4mayaNode.h"

#define MCHECK(_stat_) {if(MS::kSuccess != _stat_) {DBG("maya err at line %d: %s %d\n", __LINE__, _stat_.errorString().asChar(), _stat_.statusCode());}}


void MayaOutputHandler::setVertices(double* vtx, size_t size) {
	mVertices.clear();
	for (size_t i = 0; i < size; i += 3)
		mVertices.append((float)vtx[i], (float)vtx[i+1], (float)vtx[i+2]);
}


void MayaOutputHandler::setNormals(double* nrm, size_t size) {
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
}


// maya api tutorial: http://ewertb.soundlinker.com/maya.php
void MayaOutputHandler::createMesh() {
	MStatus stat;

	DBG("--- MayaData::createMesh begin");

	if(mPlug == 0 || mData == 0) return;

	MDataHandle outputHandle = mData->outputValue(*mPlug, &stat);
	MCHECK(stat);

	MFnMeshData dataCreator;
	MObject newOutputData = dataCreator.create(&stat);
	MCHECK(stat);

	mFnMesh = new MFnMesh();
	MObject oMesh = mFnMesh->create(mVertices.length(), mCounts.length(), mVertices, mCounts, mConnects, newOutputData, &stat);
	MCHECK(stat);

/*	int ni = 0;
	int vi = 0;
	for(unsigned int fi = 0; fi < mNormalCounts.length(); ++fi) {
		if (mNormalCounts[fi] > 0) {
			for (int v = 0; v < mNormalCounts[fi]; ++v, ++ni, ++vi) {
				MVector nrm(mNormals[mNormalConnects[ni]]);
				stat = mFnMesh->setFaceVertexNormal(nrm, fi, mConnects[vi]);
				MCHECK(stat);
			}
		}
		else
			vi += mCounts[fi];
	}
*/

	MPlugArray plugs;
	bool isConnected = mPlug->connectedTo(plugs, false, true, &stat);
	MCHECK(stat);
	DBG("    plug is connected: %d; %d plugs", isConnected, plugs.length());
	if (plugs.length() > 0) {
		if(mUVConnects.length() > 0) {
			MString uvSet = "map1";

			MCHECK(mFnMesh->setUVs(mU, mV, &uvSet));

			DBG("    mU.length          = %d", mU.length());
			DBG("    mV.length          = %d", mV.length());	
			DBG("    mUVCounts.length   = %d", mUVCounts.length());	
			DBG("    mUVConnects.length = %d", mUVConnects.length());

			MCHECK(mFnMesh->assignUVs(mUVCounts, mUVConnects, &uvSet));
		}
	}

	mShadingGroups->clear();
	mShadingRanges->clear();

	mFnMesh->setFaceVertexNormals(mNormals, mNormalCounts, mNormalConnects);
	//mFnMesh->setEdgeSmoothing()

	MMeshSmoothOptions smoothOpts;
	smoothOpts.setSmoothness(0.0f);
	smoothOpts.setKeepHardEdge(true);
	smoothOpts.setPropEdgeHardness(true);
	smoothOpts.setKeepBorderEdge(true);
	mFnMesh->setSmoothMeshDisplayOptions(smoothOpts);

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


prt::Status MayaOutputHandler::attrBool(
		size_t /*isIndex*/,
		int32_t /*shapeID*/,
		const wchar_t* key,
		bool value
) {
	mAttrs[key].mBool = value;
	std::wcout << L"evalBool: " << key << L" = " << value << std::endl;
	return prt::STATUS_OK;
}


prt::Status MayaOutputHandler::attrFloat(
		size_t /*isIndex*/,
		int32_t /*shapeID*/,
		const wchar_t* key,
		double value
) {
	mAttrs[key].mFloat = value;
	std::wcout << L"evalFloat: " << key << L" = " << value << std::endl;
	return prt::STATUS_OK;
}


prt::Status MayaOutputHandler::attrString(
		size_t /*isIndex*/,
		int32_t /*shapeID*/,
		const wchar_t* key,
		const wchar_t* value
) {
	mAttrs[key].mString = value;
	std::wcout << L"evalString: " << key << L" = " << value << std::endl;
	return prt::STATUS_OK;
}
