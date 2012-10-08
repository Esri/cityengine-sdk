/*
 * maya_codec.h
 *
 *  Created on: 21 Feb 2012
 *      Author: dec
 */

#ifndef MAYA_CODEC_H_
#define MAYA_CODEC_H_

#include "maya/MIntArray.h"
#include "maya/MStringArray.h"
#include "maya/MPlug.h"
#include "maya/MDataBlock.h"


struct MayaData {
	const MPlug*  mPlug;
	MDataBlock*   mData;
	MStringArray* mShadingGroups;
	MIntArray*    mShadingRanges;
};

#endif /* MAYA_CODEC_H_ */

