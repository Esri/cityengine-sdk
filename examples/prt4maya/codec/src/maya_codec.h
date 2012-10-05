/*
 * maya_codec.h
 *
 *  Created on: 21 Feb 2012
 *      Author: dec
 */

#ifndef MAYA_CODEC_H_
#define MAYA_CODEC_H_

class MPlug;
class MDataBlock;

struct MayaData {
	const MPlug*  mPlug;
	MDataBlock*   mData;
};

#endif /* MAYA_CODEC_H_ */

