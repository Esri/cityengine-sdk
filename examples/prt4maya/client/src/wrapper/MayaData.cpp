/*
 * MayaData.cpp
 *
 *  Created on: Oct 12, 2012
 *      Author: shaegler
 */

#include "wrapper/MayaData.h"

//#define M_CHECK3(_stat_) {if(MS::kSuccess != _stat_) {throw std::runtime_error(StringUtils::printToString("err:%s %d\n", _stat_.errorString().asChar(), _stat_.statusCode()));}}

void MayaData::setVertices(double* vtx, size_t size) {
	mVertices.clear();
	for (size_t i = 0; i < size; i += 3)
		mVertices.append(vtx[i], vtx[i+1], vtx[i+2]);
}

void MayaData::setNormals(double* nrm, size_t size) {

}

void MayaData::setUVs(float* u, float* v, size_t size) {

}

void MayaData::setFaces(int* counts, size_t countsSize, int* connects, size_t connectsSize, int* tcConnects, size_t tcConnectsSize) {
	mCounts.clear();
	for (size_t i = 0; i < countsSize; ++i)
		mCounts.append(counts[i]);

	mConnects.clear();
	for (size_t i = 0; i < connectsSize; ++i)
		mConnects.append(connects[i]);
}

// maya api tutorial: http://ewertb.soundlinker.com/maya.php
void MayaData::createMesh() {
	static bool SETUPMATERIALS = false;

	std::cout << "--- MayaData::convertGeometry begin" << std::endl;

	// setup mesh
	MStatus stat;
	MDataHandle outputHandle = mData->outputValue(*mPlug, &stat);
	//M_CHECK3(stat);
	MObject oMesh = outputHandle.asMesh();

	MFnMeshData dataCreator;
	MObject newOutputData = dataCreator.create(&stat);
	//M_CHECK3(stat);

	MFnMesh meshFn;
	oMesh = meshFn.create(
		mVertices.length(),
		mCounts.length(),
		mVertices,
		mCounts,
		mConnects,
		newOutputData,
		&stat);
	//M_CHECK3(stat);

	if(SETUPMATERIALS) {
		mShadingGroups->clear();
		mShadingRanges->clear();
//
//		MPlugArray plugs;
//		bool isConnected = mPlug->connectedTo(plugs, false, true, &stat);
//		//M_CHECK3(stat);
////		prtspi::Log::trace("plug is connected: %d; %d plugs\n", isConnected, plugs.length());
//		if(plugs.length() > 0) {
//			// setup uvs + shader connections
//			if(tcConnects.length() > 0) {
//				MString layerName = "map1";
//				stat = meshFn.setUVs(tcsU, tcsV, &layerName);
//				//M_CHECK3(stat);
//
////				prtspi::Log::trace("tcConnect has size %d",  tcConnects.length());
//
//				int uvInd = 0;
//				int curFace = 0;
//				for(size_t gi = 0, size = geometries->size(); gi < size; ++gi) {
//					prtspi::IGeometry* geo = (prtspi::IGeometry*)geometries->get(gi);
//
//					MString texName;
//
//					prtspi::IMaterial* mat = geo->getMaterial();
//					if(mat->getTextureArray(L"diffuseMap")->size() > 0) {
//						std::wstring uri(mat->getTextureArray(L"diffuseMap")->get(0)->getName());
//						texName = MString(uri.substr(wcslen(URIUtils::SCHEME_FILE)).c_str());
//					}
//
//					const int faceCount   = (int)geo->getFaceCount();
//					const size_t tcsCount = geo->getUVCount();
//					const bool hasUVs     = tcsCount > 0;
//
////					prtspi::Log::trace("Material %d : hasUVs = %d, faceCount = %d, texName = '%s'\n", gi, hasUVs, faceCount, texName.asChar());
//
//					int startFace = curFace;
//
//					if(hasUVs) {
//						for(int i=0; i<faceCount; i++) {
//							for(int j=0; j<counts[curFace]; j++) {
//								stat = meshFn.assignUV(curFace, j,  tcConnects[uvInd++], &layerName);
//								M_CHECK3(stat);
//							}
//							curFace++;
//						}
//					}
//					else curFace += faceCount;
//
//					if(curFace - 1 > startFace) {
//						MString cmd("createShadingGroup(\"");
//						cmd += texName;
//						cmd += "\")";
//						MString result = MGlobal::executeCommandStringResult(cmd, false, false, &stat);
////						prtspi::Log::trace("mel cmd '%s' executed, result = '%s'", cmd.asChar(), result.asChar());
//
//						mShadingGroups->append(result);
//						mShadingRanges->append(startFace);
//						mShadingRanges->append(curFace - 1);
//					}
//
//					//M_CHECK3(stat);
//				}
//			}
//		} // if > 0 connections
	} // if SETUPMATERIALS


	stat = outputHandle.set(newOutputData);
	//M_CHECK3(stat);


	printf("    mayaVertices.length = %d", mVertices.length());
	printf("    mayaCounts.length   = %d", mCounts.length());
	printf("    mayaConnects.length = %d", mConnects.length());
}

