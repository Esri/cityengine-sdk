from libc.stdlib cimport malloc, free
from libc.string cimport memcpy
from cpython cimport array

cdef extern from "prt/prt_stdint.h":
	ctypedef unsigned int uint64_t
	ctypedef unsigned int uint32_t
	ctypedef signed int int32_t
	ctypedef unsigned int size_t

cdef extern from "SimpleWrapper.h":
	struct InitialShapeData:
		const char*	mName
		const char* mRuleFile
		const char* mStartRule
		int32_t		mRandomSeed
		double*		mVertexCoords
		size_t		mVertexCoordsCount
		uint32_t*	mIndices
		size_t      mIndicesCount
		uint32_t*	mFaceCounts
		size_t		mFaceCountsCount
	cdef int _prtInit(const char* prtPath, const char* licType)
	cdef int _prtGenerate(const char* rulePackage, InitialShapeData* initialShapes, size_t initialShapeCount, const char* encoderID, const char* outputPath)
	cdef const char* _prtGetStatusMessage(int status)
	cdef int _prtRelease()

def prtInit(prtPath, licType):
	return _prtInit(prtPath, licType)

def prtGenerate(rulePackage, initialShapes, encoderID, outputPath):
	isCount = len(initialShapes)
	cdef InitialShapeData* initialShapeDataArray = <InitialShapeData*>malloc(isCount * sizeof(InitialShapeData))
	
	i = 0
	cdef InitialShapeData isd
	cdef array.array arr
	for isName, isData in initialShapes.iteritems():
		isd.mName					= isName
		isd.mRuleFile				= isData["ruleFile"]
		isd.mStartRule				= isData["startRule"]
		isd.mRandomSeed				= isData["randomSeed"]
		
		arr = array.array('d', isData["vtxCoords"])
		isd.mVertexCoords			= <double*>malloc(len(arr) * sizeof(double))
		memcpy(isd.mVertexCoords, arr.data.as_doubles, len(arr) * sizeof(double))
		isd.mVertexCoordsCount		= len(arr)

		arr = array.array('i', isData["indices"])
		isd.mIndices				= <uint32_t*>malloc(len(arr) * sizeof(uint32_t))
		memcpy(isd.mIndices, arr.data.as_ints, len(arr) * sizeof(uint32_t))
		isd.mIndicesCount			= len(arr)

		arr = array.array('i', isData["faceCounts"])
		isd.mFaceCounts				= <uint32_t*>malloc(len(arr) * sizeof(uint32_t))
		memcpy(isd.mFaceCounts, arr.data.as_ints, len(arr) * sizeof(uint32_t))
		isd.mFaceCountsCount		= len(arr)

		memcpy(&initialShapeDataArray[i], &isd, sizeof(InitialShapeData));
		i += 1
	
	status = _prtGenerate(rulePackage, initialShapeDataArray, isCount, encoderID, outputPath)
	
	# TODO FREE ARRAYS
	#free(initialShapeDataArray)
	
	return status

def prtGetStatusMessage(status):
	return _prtGetStatusMessage(status)

def prtRelease():
	return _prtRelease()
