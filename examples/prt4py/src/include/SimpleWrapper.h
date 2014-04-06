#ifndef PRT4PY_SIMPLEWRAPPER_H_
#define PRT4PY_SIMPLEWRAPPER_H_

#include "prt/prt_stdint.h"

extern "C" {

struct InitialShapeData {
	const char*	mName;
	const char* mRuleFile;
	const char* mStartRule;
	int32_t		mRandomSeed;
	double*		mVertexCoords;
	size_t		mVertexCoordsCount;
	uint32_t*	mIndices;
	size_t      mIndicesCount;
	uint32_t*	mFaceCounts;
	size_t		mFaceCountsCount;
};
	
int			_prtInit(const char* prtPath, const char* licType);
int			_prtGenerate(const char* rulePackage, InitialShapeData* initialShapes, size_t initialShapeCount, const char* encoderID, const char* outputPath);
const char*	_prtGetStatusMessage(int status);
int			_prtRelease();

} // extern "C"

#endif