#ifndef VOXELCALLBACKS_H
#define VOXELCALLBACKS_H

#include "prt/Callbacks.h"


class VoxelCallbacks : public prt::Callbacks {
public:
	virtual void updateVoxels(const uint8_t* voxels, size_t voxelsSize) = 0;
	virtual void updateStatistics(uint32_t leafCount, uint32_t nonLeafCount, double leafAttrProp) = 0;

	virtual prt::Status generateError(size_t isIndex, prt::Status status, const wchar_t* message) { return prt::STATUS_OK; }
	virtual prt::Status assetError(size_t isIndex, prt::CGAErrorLevel level, const wchar_t* key, const wchar_t* uri, const wchar_t* message) { return prt::STATUS_OK; }
	virtual prt::Status cgaError(size_t isIndex, int32_t shapeID, prt::CGAErrorLevel level, int32_t methodId, int32_t pc, const wchar_t* message) { return prt::STATUS_OK; }
	virtual prt::Status cgaPrint(size_t isIndex, int32_t shapeID, const wchar_t* txt) { return prt::STATUS_OK; }
	virtual prt::Status cgaReportBool(size_t isIndex, int32_t shapeID, const wchar_t* key, bool value) { return prt::STATUS_OK; }
	virtual prt::Status cgaReportFloat(size_t isIndex, int32_t shapeID, const wchar_t* key, double value) { return prt::STATUS_OK; }
	virtual prt::Status cgaReportString(size_t isIndex, int32_t shapeID, const wchar_t* key, const wchar_t* value) { return prt::STATUS_OK; }
	virtual prt::Status attrBool(size_t isIndex, int32_t shapeID, const wchar_t* key, bool value) { return prt::STATUS_OK; }
	virtual prt::Status attrFloat(size_t isIndex, int32_t shapeID, const wchar_t* key, double value) { return prt::STATUS_OK; }
	virtual prt::Status attrString(size_t isIndex, int32_t shapeID, const wchar_t* key, const wchar_t* value) { return prt::STATUS_OK; }
};

#endif //VOXELCALLBACKS_H
