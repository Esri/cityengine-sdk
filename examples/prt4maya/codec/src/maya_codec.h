/*
 * maya_codec.h
 *
 *  Created on: 21 Feb 2012
 *      Author: dec
 */

#ifndef MAYA_CODEC_H_
#define MAYA_CODEC_H_

#include "maya/MFloatPointArray.h"
#include "maya/MIntArray.h"
#include "maya/MFloatArray.h"

struct MayaMatData {
	std::string texName;
	int faceCount;
	bool hasUVs;
};

struct MayaData {
	MFloatPointArray* vertices;
	MIntArray*        counts;
	MIntArray*        connects;
	MFloatArray*      tcsU;
	MFloatArray*      tcsV;
	MIntArray*        tcConnects;

	MayaMatData*      materials;
	int               materialCount;
	void (*destroy)(struct MayaData *);
};

#endif /* MAYA_CODEC_H_ */

