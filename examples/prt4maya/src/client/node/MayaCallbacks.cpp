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
#include "prtModifier/PRTModifierNode.h"
#include "node/PRTMaterialNode.h"

#include "prt/StringUtils.h"
#include "prtx/Material.h"

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

	std::wstring uriToPath(const prtx::TexturePtr& t) {
		return t->getURI()->getPath();
	}

	// we blacklist all CGA-style material attribute keys, see prtx/Material.h
	const std::set<std::wstring> MATERIAL_ATTRIBUTE_BLACKLIST = {
		L"ambient.b",
		L"ambient.g",
		L"ambient.r",
		L"bumpmap.rw",
		L"bumpmap.su",
		L"bumpmap.sv",
		L"bumpmap.tu",
		L"bumpmap.tv",
		L"color.a",
		L"color.b",
		L"color.g",
		L"color.r",
		L"color.rgb",
		L"colormap.rw",
		L"colormap.su",
		L"colormap.sv",
		L"colormap.tu",
		L"colormap.tv",
		L"dirtmap.rw",
		L"dirtmap.su",
		L"dirtmap.sv",
		L"dirtmap.tu",
		L"dirtmap.tv",
		L"normalmap.rw",
		L"normalmap.su",
		L"normalmap.sv",
		L"normalmap.tu",
		L"normalmap.tv",
		L"opacitymap.rw",
		L"opacitymap.su",
		L"opacitymap.sv",
		L"opacitymap.tu",
		L"opacitymap.tv",
		L"specular.b",
		L"specular.g",
		L"specular.r",
		L"specularmap.rw",
		L"specularmap.su",
		L"specularmap.sv",
		L"specularmap.tu",
		L"specularmap.tv",
		L"bumpmap",
		L"colormap",
		L"dirtmap",
		L"normalmap",
		L"opacitymap",
		L"opacitymap.mode",
		L"specularmap"

	#if PRT_VERSION_MAJOR > 1
		// also blacklist CGA-style PBR attrs from CE 2019.0, PRT 2.x
		,
		L"emissive.b",
		L"emissive.g",
		L"emissive.r",
		L"emissivemap.rw",
		L"emissivemap.su",
		L"emissivemap.sv",
		L"emissivemap.tu",
		L"emissivemap.tv",
		L"metallicmap.rw",
		L"metallicmap.su",
		L"metallicmap.sv",
		L"metallicmap.tu",
		L"metallicmap.tv",
		L"occlusionmap.rw",
		L"occlusionmap.su",
		L"occlusionmap.sv",
		L"occlusionmap.tu",
		L"occlusionmap.tv",
		L"roughnessmap.rw",
		L"roughnessmap.su",
		L"roughnessmap.sv",
		L"roughnessmap.tu",
		L"roughnessmap.tv",
		L"emissivemap",
		L"metallicmap",
		L"occlusionmap",
		L"roughnessmap"
	#endif
	};

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
	size_t maxFloatArrayLength = 5;
	size_t maxStringArrayLength = 2;

	adsk::Data::Structure* fStructure;	  // Structure to use for creation
	fStructure = adsk::Data::Structure::structureByName(gPRTMatStructure.c_str());
	if (fStructure == NULL)
	{
		// Register our structure since it is not registered yet.
		fStructure = adsk::Data::Structure::create();
		fStructure->setName(gPRTMatStructure.c_str());

		fStructure->addMember(adsk::Data::Member::kInt32, 1, gPRTMatMemberFaceStart.c_str());
		fStructure->addMember(adsk::Data::Member::kInt32, 1, gPRTMatMemberFaceEnd.c_str());

		prtx::MaterialBuilder mb;
		prtx::MaterialPtr m = mb.createShared();
		const prtx::WStringVector&    keys = m->getKeys();

		for (const auto& key : keys) {
			if (MATERIAL_ATTRIBUTE_BLACKLIST.count(key) > 0)
				continue;

			adsk::Data::Member::eDataType type;
			size_t size = 0;
			size_t arrayLength = 1;

			switch (m->getType(key)) {
			case prt::Attributable::PT_BOOL: type = adsk::Data::Member::kBoolean; size = 1;  break;
			case prt::Attributable::PT_FLOAT: type = adsk::Data::Member::kDouble; size = 1; break;
			case prt::Attributable::PT_INT: type = adsk::Data::Member::kInt32; size = 1; break;

			//workaround: using kString type crashes maya when setting metadata elememts. Therefore we use array of kUInt8
			case prt::Attributable::PT_STRING: type = adsk::Data::Member::kUInt8; size = maxStringLength;  break;
			case prt::Attributable::PT_BOOL_ARRAY: type = adsk::Data::Member::kBoolean; size = maxStringLength; break;
			case prt::Attributable::PT_INT_ARRAY: type = adsk::Data::Member::kInt32; size = maxStringLength; break;
			case prt::Attributable::PT_FLOAT_ARRAY: type = adsk::Data::Member::kDouble; size = maxFloatArrayLength; break;
			case prt::Attributable::PT_STRING_ARRAY: type = adsk::Data::Member::kUInt8; size = maxStringLength; arrayLength = maxStringArrayLength; break;

			case prtx::Material::PT_TEXTURE: type = adsk::Data::Member::kUInt8; size = maxStringLength;  break;
			case prtx::Material::PT_TEXTURE_ARRAY: type = adsk::Data::Member::kUInt8; size = maxStringLength; arrayLength = maxStringArrayLength; break;

			default:
				break;
			}

			if (size > 0) {
				for (int i=0; i<arrayLength; i++) {
					size_t maxStringLengthTmp = maxStringLength;
					char* tmp = new char[maxStringLength];
					std::wstring keyToUse = key;
					if (i>0)
						keyToUse = key + std::to_wstring(i);
					prt::StringUtils::toOSNarrowFromUTF16(keyToUse.c_str(), tmp, &maxStringLengthTmp);
					fStructure->addMember(type, size, tmp);
					delete tmp;
				}
			}
		}

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




		char* tmp = new char[maxStringLength];
		const prtx::WStringVector&    keys = mat->getKeys();
		for (const auto& key : keys) {
			if (MATERIAL_ATTRIBUTE_BLACKLIST.count(key) > 0)
				continue;

			size_t maxStringLengthTmp = maxStringLength;
			prt::StringUtils::toOSNarrowFromUTF16(key.c_str(), tmp, &maxStringLengthTmp);

			if (!handle.setPositionByMemberName(tmp))
				continue;

			maxStringLengthTmp = maxStringLength;

			switch (mat->getType(key)) {
			case prt::Attributable::PT_BOOL: handle.asBoolean()[0] = mat->getBool(key);  break;
			case prt::Attributable::PT_FLOAT: handle.asDouble()[0] = mat->getFloat(key);  break;
			case prt::Attributable::PT_INT: handle.asInt32()[0] = mat->getInt(key);  break;

				//workaround: transporting string as uint8 array, because using asString crashes maya
			case prt::Attributable::PT_STRING:
				if (mat->getString(key).size()==0)
					break;
				checkStringLength(mat->getString(key), maxStringLength);
				prt::StringUtils::toOSNarrowFromUTF16(mat->getString(key).c_str(), (char*)handle.asUInt8(), &maxStringLengthTmp);
				break;
			case prt::Attributable::PT_BOOL_ARRAY:
				for (int i = 0; i < mat->getBoolArray(key).size() && i < maxStringLength; i++)
					handle.asBoolean()[i] = mat->getBoolArray(key)[i];
				break;
			case prt::Attributable::PT_INT_ARRAY:
				for (int i = 0; i < mat->getIntArray(key).size() && i < maxStringLength; i++)
					handle.asInt32()[i] = mat->getIntArray(key)[i];
				break;
			case prt::Attributable::PT_FLOAT_ARRAY:
				for (int i = 0; i < mat->getFloatArray(key).size() && i < maxFloatArrayLength; i++)
					handle.asDouble()[i] = mat->getFloatArray(key)[i];
				break;
			case prt::Attributable::PT_STRING_ARRAY: {
				for (int i = 0; i < mat->getStringArray(key).size() && i < maxStringLength; i++)
				{
					if (mat->getStringArray(key)[i].size() == 0)
						continue;

					if (i>0) {
						std::wstring keyToUse = key + std::to_wstring(i);
						maxStringLengthTmp = maxStringLength;
						prt::StringUtils::toOSNarrowFromUTF16(keyToUse.c_str(), tmp, &maxStringLengthTmp);
						handle.setPositionByMemberName(tmp+i);
					}

					maxStringLengthTmp = maxStringLength;
					checkStringLength(mat->getStringArray(key)[i], maxStringLength);
					prt::StringUtils::toOSNarrowFromUTF16(mat->getStringArray(key)[i].c_str(), (char*)handle.asUInt8(), &maxStringLengthTmp);
				}
				break;
			}
			case prtx::Material::PT_TEXTURE: {
				const auto& t = mat->getTexture(key);
				const std::wstring p = uriToPath(t);
				prt::StringUtils::toOSNarrowFromUTF16(p.c_str(), (char*)handle.asUInt8(), &maxStringLengthTmp);
				break;
			}
			case prtx::Material::PT_TEXTURE_ARRAY: {
				const auto& ta = mat->getTextureArray(key);
				prtx::WStringVector pa(ta.size());
				std::transform(ta.begin(), ta.end(), pa.begin(), uriToPath);
				for (int i = 0; i < mat->getTextureArray(key).size() && i < maxStringArrayLength; i++)
				{
					if (pa[i].size() == 0)
						continue;

					if (i > 0) {
						std::wstring keyToUse = key + std::to_wstring(i);
						maxStringLengthTmp = maxStringLength;
						prt::StringUtils::toOSNarrowFromUTF16(keyToUse.c_str(), tmp, &maxStringLengthTmp);
						handle.setPositionByMemberName(tmp);
					}

					maxStringLengthTmp = maxStringLength;
					checkStringLength(pa[i].c_str(), maxStringLength);
					prt::StringUtils::toOSNarrowFromUTF16(pa[i].c_str(), (char*)handle.asUInt8(), &maxStringLengthTmp);
				}
				break;
			}
			}
		}
		delete tmp;

		

		handle.setPositionByMemberName(gPRTMatMemberFaceStart.c_str());
		handle.asInt32()[0] = mShadingRanges[i * 2];

		handle.setPositionByMemberName(gPRTMatMemberFaceEnd.c_str());
		handle.asInt32()[0] = mShadingRanges[i * 2 + 1];

		newStream.setElement(i, handle);
	}

	outputMesh.setMetadata(newMetadata);
}

void MayaCallbacks::checkStringLength(std::wstring string, const size_t &maxStringLength)
{
	if (string.length() >= maxStringLength) {
		const std::wstring msg = L"Maximum texture path size is " + std::to_wstring(maxStringLength);
		prt::log(msg.c_str(), prt::LOG_ERROR);
	}
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
