/**
 * Esri CityEngine SDK Maya Plugin Example
 *
 * This example demonstrates the main functionality of the Procedural Runtime API.
 * Esri R&D Center Zurich, Switzerland
 *
 * See http://github.com/ArcGIS/esri-cityengine-sdk for instructions.
 */

#include "node/MayaCallbacks.h"
#include "node/Utilities.h"
#include "node/PRTNode.h"

#include "maya/MItMeshPolygon.h"

#include <cassert>
#include <sstream>
#include <ostream>


namespace {

const bool TRACE = false;
void prtTrace(const std::wstring& arg1, std::size_t arg2) {
	if (TRACE) {
		std::wostringstream wostr;
		wostr << L"[MOH] "<< arg1 << arg2;
		prt::log(wostr.str().c_str(), prt::LOG_TRACE);
	}
}

} // anonymous namespace


void MayaCallbacks::setVertices(const double* vtx, size_t size) {
	prtTrace(L"setVertices: size = ", size);
	mVertices.clear();
	for (size_t i = 0; i < size; i += 3)
		mVertices.append(static_cast<float>(vtx[i]), static_cast<float>(vtx[i+1]), static_cast<float>(vtx[i+2]));
}

void MayaCallbacks::setNormals(const double* nrm, size_t size) {
	prtTrace(L"setNormals: size = ", size);
	mNormals.clear();
	for (size_t i = 0; i < size; i += 3)
		mNormals.append(MVector(nrm[i], nrm[i+1], nrm[i+2]));
}

void MayaCallbacks::setUVs(const double* u, const double* v, size_t size) {
	mU.clear();
	mV.clear();
	for (size_t i = 0; i < size; ++i) {
		mU.append(u[i]);
		mV.append(v[i]);
	}
}

void MayaCallbacks::setFaces(
		const uint32_t* counts, size_t countsSize,
		const uint32_t* connects, size_t connectsSize,
		const uint32_t* uvCounts, size_t uvCountsSize,
		const uint32_t* uvConnects, size_t uvConnectsSize
) {
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

void MayaCallbacks::createMesh() {
	MStatus stat;

	prtu::dbg("--- MayaData::createMesh begin");

	if (mPlug == nullptr || mData == nullptr)
		return;

	MDataHandle outputHandle = mData->outputValue(*mPlug, &stat);
	MCHECK(stat);

	MFnMeshData dataCreator;
	MObject newOutputData = dataCreator.create(&stat);
	MCHECK(stat);

	prtu::dbg("    mVertices.length         = %d", mVertices.length());
	prtu::dbg("    mVerticesCounts.length   = %d", mVerticesCounts.length());
	prtu::dbg("    mVerticesConnects.length = %d", mVerticesConnects.length());
	prtu::dbg("    mNormals.length          = %d", mNormals.length());

	mFnMesh.reset(new MFnMesh());
	MObject oMesh = mFnMesh->create(mVertices.length(), mVerticesCounts.length(), mVertices, mVerticesCounts, mVerticesConnects, newOutputData, &stat);
	MCHECK(stat);

	MPlugArray plugs;
	mPlug->connectedTo(plugs, false, true, &stat);
	MCHECK(stat);
	if (plugs.length() > 0) {
		if(mUVConnects.length() > 0) {
			MString uvSet = "map1";

			MCHECK(mFnMesh->setUVs(mU, mV, &uvSet));

			prtu::dbg("    mU.length          = %d", mU.length());
			prtu::dbg("    mV.length          = %d", mV.length());
			prtu::dbg("    mUVCounts.length   = %d", mUVCounts.length());
			prtu::dbg("    mUVConnects.length = %d", mUVConnects.length());

			MCHECK(mFnMesh->assignUVs(mUVCounts, mUVConnects, &uvSet));
		}
	}

	mShadingGroups->clear();
	mShadingRanges->clear();
	mShadingCmd->clear();

	if(mNormals.length() > 0) {
		prtu::dbg("    mNormals.length        = %d", mNormals.length());

		// NOTE: this assumes that vertices and vertex normals use the same index domain (-> maya encoder)
		MVectorArray expandedNormals(mVerticesConnects.length());
		for (unsigned int i = 0; i < mVerticesConnects.length(); i++)
			expandedNormals[i] = mNormals[mVerticesConnects[i]];

		prtu::dbg("    expandedNormals.length = %d", expandedNormals.length());

		MCHECK(mFnMesh->setVertexNormals(expandedNormals, mVerticesConnects));
	}

	MCHECK(outputHandle.set(newOutputData));
}

MString getGroupName(const wchar_t* name) {
	MString matName(name);
	const unsigned int len = matName.numChars();
	MString result = "prtmat";
	result        += prtu::toCleanId(matName.substringW(matName.rindexW('/') + 1, len));
	result        += "SG";
	return result;
}

MString MayaCallbacks::matCreate(int start, int count, const wchar_t* name) {
	const MString groupName = getGroupName(name);

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

void MayaCallbacks::matSetDiffuseTexture(uint32_t start, uint32_t count, const wchar_t* tex) {
	const MString matName = matCreate(start, count, tex);
	if (matName.numChars() == 0)
		return;
	*mShadingCmd += "prtSetDiffuseTexture(\"" + matName + "\",\"" + tex + "\",\"map1\");\n";
}

void MayaCallbacks::matSetColor(uint32_t start, uint32_t count, double r, double g, double b) {
	wchar_t name[8];
	swprintf(name, 7, L"%02X%02X%02X", (int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0));
	const MString matName = matCreate(start, count, name);
	if(matName.numChars() == 0)
		return;
	*mShadingCmd += "prtSetColor(\"" + matName + "\"," + r + "," + g + "," + b + ");\n";
}

void MayaCallbacks::finishMesh() {
	mFnMesh.reset();
}

prt::Status MayaCallbacks::attrBool(size_t /*isIndex*/, int32_t /*shapeID*/, const wchar_t* key, bool value) {
	mAttrs[key].mBool = value;
	return prt::STATUS_OK;
}

prt::Status MayaCallbacks::attrFloat(size_t /*isIndex*/, int32_t /*shapeID*/, const wchar_t* key, double value) {
	mAttrs[key].mFloat = value;
	return prt::STATUS_OK;
}

prt::Status MayaCallbacks::attrString(size_t /*isIndex*/, int32_t /*shapeID*/, const wchar_t* key, const wchar_t* value) {
	mAttrs[key].mString = value;
	return prt::STATUS_OK;
}
