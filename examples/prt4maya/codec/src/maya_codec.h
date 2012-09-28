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

struct MayaData {
	MFloatPointArray* vertices;
	MIntArray*        counts;
	MIntArray*        connects;
	void (*destroy)(struct MayaData *);
};

#endif /* MAYA_CODEC_H_ */

