/**
 * Esri CityEngine SDK Maya Plugin Example
 *
 * This example demonstrates the main functionality of the Procedural Runtime API.
 *
 * See README.md in https://github.com/Esri/esri-cityengine-sdk for build instructions.
 *
 * Copyright (c) 2012-2019 Esri R&D Center Zurich
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "node/MayaCallbacks.h"
#include "node/Utilities.h"
#include "prtModifier/PRTModifierNode.h"
#include "node/PRTMaterialNode.h"

#include "prt/StringUtils.h"

#include <maya/adskDataStream.h>
#include <maya/adskDataChannel.h>
#include <maya/adskDataAssociations.h>
#include <maya/adskDataAccessorMaya.h>

#include <sstream>

namespace {

	const bool TRACE = false;
	void prtTrace(const std::wstring& arg1, std::size_t arg2) {
		if (TRACE) {
			std::wostringstream wostr;
			wostr << L"[MOH] " << arg1 << arg2;
			prt::log(wostr.str().c_str(), prt::LOG_TRACE);
		}
	}

} // anonymous namespace


void MayaCallbacks::setVertices(const double* vtx, size_t size) {
	prtTrace(L"setVertices: size = ", size);
	mVertices.clear();
	for (size_t i = 0; i < size; i += 3)
		mVertices.append(static_cast<float>(vtx[i]), static_cast<float>(vtx[i + 1]), static_cast<float>(vtx[i + 2]));
}

void MayaCallbacks::setNormals(const double* nrm, size_t size) {
	prtTrace(L"setNormals: size = ", size);
	mNormals.clear();
	for (size_t i = 0; i < size; i += 3)
		mNormals.append(MVector(nrm[i], nrm[i + 1], nrm[i + 2]));
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

void MayaCallbacks::setMaterial(uint32_t start, uint32_t count, const prtx::MaterialPtr& mat) {
	mMaterials.push_back(mat);
	mShadingRanges.append(start);
	mShadingRanges.append(start + count - 1);
}

void MayaCallbacks::createMesh() {
	MStatus stat;

	prtu::dbg("--- MayaData::createMesh begin");

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


	if (mUVConnects.length() > 0) {
		MString uvSet = "map1";

		MCHECK(mFnMesh->setUVs(mU, mV, &uvSet));

		prtu::dbg("    mU.length          = %d", mU.length());
		prtu::dbg("    mV.length          = %d", mV.length());
		prtu::dbg("    mUVCounts.length   = %d", mUVCounts.length());
		prtu::dbg("    mUVConnects.length = %d", mUVConnects.length());

		MCHECK(mFnMesh->assignUVs(mUVCounts, mUVConnects, &uvSet));
	}

	if (mNormals.length() > 0) {
		prtu::dbg("    mNormals.length        = %d", mNormals.length());

		// NOTE: this assumes that vertices and vertex normals use the same index domain (-> maya encoder)
		MVectorArray expandedNormals(mVerticesConnects.length());
		for (unsigned int i = 0; i < mVerticesConnects.length(); i++)
			expandedNormals[i] = mNormals[mVerticesConnects[i]];

		prtu::dbg("    expandedNormals.length = %d", expandedNormals.length());

		MCHECK(mFnMesh->setVertexNormals(expandedNormals, mVerticesConnects));
	}

	MFnMesh outputMesh(outMeshObj);
	outputMesh.copyInPlace(oMesh);

	// create material metadata
	size_t maxStringLength = 400;
	adsk::Data::Structure* fStructure;	  // Structure to use for creation
	fStructure = adsk::Data::Structure::structureByName(gPRTMatStructure.c_str());
	if (fStructure == NULL)
	{
		// Register our structure since it is not registered yet.
		fStructure = adsk::Data::Structure::create();
		fStructure->setName(gPRTMatStructure.c_str());
		//workaround: using kString type crashes maya when setting metadata elememts. Therefore we use array of kUInt8
		fStructure->addMember(adsk::Data::Member::kUInt8, maxStringLength + 1, gPRTMatMemberTexture.c_str());
		fStructure->addMember(adsk::Data::Member::kDouble, 3, gPRTMatMemberColor.c_str());
		fStructure->addMember(adsk::Data::Member::kInt32, 1, gPRTMatMemberFaceStart.c_str());
		fStructure->addMember(adsk::Data::Member::kInt32, 1, gPRTMatMemberFaceEnd.c_str());
		adsk::Data::Structure::registerStructure(*fStructure);
	}

	MCHECK(stat);
	MFnMesh inputMesh(inMeshObj);

	adsk::Data::Associations newMetadata(inputMesh.metadata(&stat));
	newMetadata.makeUnique();
	MCHECK(stat);
	adsk::Data::Channel newChannel = newMetadata.channel(gPRTMatChannel);
	adsk::Data::Stream newStream(*fStructure, gPRTMatStream);

	newChannel.setDataStream(newStream);
	newMetadata.setChannel(newChannel);

	for (unsigned int i = 0; i < mMaterials.size(); i++) {
		adsk::Data::Handle handle(*fStructure);

		prtx::MaterialPtr mat = mMaterials[i];

		if (mat->diffuseMap().size() > 0 && mat->diffuseMap().front()->isValid()) {
			const prtx::URIPtr texURI = mat->diffuseMap().front()->getURI();
			const std::wstring texPathW = texURI->getPath();

			if (texPathW.length() >= maxStringLength) {
				const std::wstring msg = L"Maximum texture path size is " + std::to_wstring(maxStringLength);
				prt::log(msg.c_str(), prt::LOG_ERROR);
			}

			size_t maxStringLengthTmp = maxStringLength;
			//workaround: transporting string as uint8 array, because using asString crashes maya
			prt::StringUtils::toOSNarrowFromUTF16(texPathW.c_str(), (char*)handle.asUInt8(), &maxStringLengthTmp);
		}

		handle.setPositionByMemberName(gPRTMatMemberColor.c_str());
		handle.asDouble()[0] = mat->color_r();
		handle.asDouble()[1] = mat->color_g();
		handle.asDouble()[2] = mat->color_b();

		handle.setPositionByMemberName(gPRTMatMemberFaceStart.c_str());
		handle.asInt32()[0] = mShadingRanges[i * 2];

		handle.setPositionByMemberName(gPRTMatMemberFaceEnd.c_str());
		handle.asInt32()[0] = mShadingRanges[i * 2 + 1];

		newStream.setElement(i, handle);
	}

	outputMesh.setMetadata(newMetadata);
}

void MayaCallbacks::finishMesh() {
	mFnMesh.reset();
	mMaterials.clear();
	mShadingRanges.clear();
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
