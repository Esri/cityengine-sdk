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

using namespace prtUtils;

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
		mNormals.append(MVector(nrm[i], nrm[i+1], nrm[i+2]));
}


void MayaOutputHandler::setUVs(float* u, float* v, size_t size) {
	mU.clear();
	mV.clear();
	for (size_t i = 0; i < size; ++i) {
		mU.append(u[i]);
		mV.append(v[i]);
	}
}

void MayaOutputHandler::setFaces(int* counts, size_t countsSize, int* connects, size_t connectsSize, int* uvCounts, size_t uvCountsSize, int* uvConnects, size_t uvConnectsSize) {
	mVerticesCounts.clear();
	for (size_t i = 0; i < countsSize; ++i)
		mVerticesCounts.append(counts[i]);
	prtTrace(L"countsSize = ", countsSize);

	mVerticesConnects.clear();
	for (size_t i = 0; i < connectsSize; ++i)
		mVerticesConnects.append(connects[i]);
	prtTrace(L"connectsSize = ", connectsSize);

	mUVCounts.clear();
	for (size_t i = 0; i < uvCountsSize; ++i)
		mUVCounts.append(uvCounts[i]);

	mUVConnects.clear();
	for (size_t i = 0; i < uvConnectsSize; ++i)
		mUVConnects.append(uvConnects[i]);
}

#define VERBOSE_NORMALS 0

void MayaOutputHandler::createMesh() {
	MStatus stat;

	DBG("--- MayaData::createMesh begin");

	if(mPlug == 0 || mData == 0) return;

	MDataHandle outputHandle = mData->outputValue(*mPlug, &stat);
	MCHECK(stat);

	MFnMeshData dataCreator;
	MObject newOutputData = dataCreator.create(&stat);
	MCHECK(stat);

	DBG("    mVertices.length         = %d", mVertices.length());
	DBG("    mVerticesCounts.length   = %d", mVerticesCounts.length());
	DBG("    mVerticesConnects.length = %d", mVerticesConnects.length());
	DBG("    mNormals.length          = %d", mNormals.length());

	mFnMesh = new MFnMesh();
	MObject oMesh = mFnMesh->create(mVertices.length(), mVerticesCounts.length(), mVertices, mVerticesCounts, mVerticesConnects, newOutputData, &stat);
	MCHECK(stat);

	MPlugArray plugs;
	bool isConnected = mPlug->connectedTo(plugs, false, true, &stat);
	MCHECK(stat);
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
	mShadingCmd->clear();

	if(mNormals.length() > 0) {
		DBG("    mNormals.length        = %d", mNormals.length());

		// NOTE: this expects that vertices and vertex normals use the same index domain (-> maya encoder)
		MVectorArray expandedNormals(mVerticesConnects.length());
		for (unsigned int i = 0; i < mVerticesConnects.length(); i++)
			expandedNormals[i] = mNormals[mVerticesConnects[i]];
		
		DBG("    expandedNormals.length = %d", expandedNormals.length());

		MCHECK(mFnMesh->setVertexNormals(expandedNormals, mVerticesConnects));
	}

	MCHECK(outputHandle.set(newOutputData));
}

MString getGroupName(const wchar_t* name) {
	MString matName(name);
	int     len    = matName.numChars();
	MString result = "prtmat";
	result        += toCleanId(matName.substringW(matName.rindexW('/') + 1, len));
	result        += "SG";
	return result;
}

MString MayaOutputHandler::matCreate(int start, int count, const wchar_t* name) {
  MString groupName = getGroupName(name);

	bool createGroup = true;
	for(int i = PRTNode::theShadingGroups.length(); --i >= 0;)
		if(PRTNode::theShadingGroups[i] == groupName)  {
			createGroup = false;
			break;
		}
	
	if(createGroup) {
		PRTNode::theShadingGroups.append(groupName);
		*mShadingCmd += "sets -renderable true -noSurfaceShader true -empty -name (\"" + groupName + "\");\n";
	}

	mShadingGroups->append(groupName);
	mShadingRanges->append(start);
	mShadingRanges->append(start + count - 1);

	return createGroup ? groupName : MString();
}


void MayaOutputHandler::matSetDiffuseTexture(int start, int count, const wchar_t* tex) {
	MString matName = matCreate(start, count, tex);
	if(matName.numChars() == 0) return;

	*mShadingCmd += "prtSetDiffuseTexture(\"" + matName + "\",\"" + tex + "\",\"map1\");\n";
}

void MayaOutputHandler::matSetColor(int start, int count, float r, float g, float b) {
	wchar_t name[8];
	swprintf(name, 7, L"%02X%02X%02X", (int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0));
	MString matName = matCreate(start, count, name);
	if(matName.numChars() == 0) return;

	*mShadingCmd += "prtSetColor(\"" + matName + "\"," + r + "," + g + "," + b + ");\n";
}

void MayaOutputHandler::finishMesh() {
	delete mFnMesh;
}

prt::Status MayaOutputHandler::attrBool(size_t /*isIndex*/, int32_t /*shapeID*/, const wchar_t* key, bool value) {
	mAttrs[key].mBool = value;
	return prt::STATUS_OK;
}

prt::Status MayaOutputHandler::attrFloat(size_t /*isIndex*/, int32_t /*shapeID*/, const wchar_t* key, double value) {
	mAttrs[key].mFloat = value;
	return prt::STATUS_OK;
}

prt::Status MayaOutputHandler::attrString(size_t /*isIndex*/, int32_t /*shapeID*/, const wchar_t* key, const wchar_t* value) {
	mAttrs[key].mString = value;
	return prt::STATUS_OK;
}
