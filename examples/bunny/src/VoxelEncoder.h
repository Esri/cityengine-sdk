#ifndef VOXELENCODER_H
#define VOXELENCODER_H

#include "prtx/Encoder.h"
#include "prtx/EncoderFactory.h"
#include "prtx/Singleton.h"

#include <string>


namespace prt {
	class AttributeMap;
}

namespace prtx {
	class GenerateContext;
}

class VoxelEncoder : public prtx::GeometryEncoder {
public:
	static const std::wstring ID;
	static const std::wstring NAME;
	static const std::wstring DESCRIPTION;

	VoxelEncoder(const std::wstring& id, const prt::AttributeMap* defaultOptions, prt::Callbacks* callbacks);
	virtual ~VoxelEncoder();

	virtual void init(prtx::GenerateContext& context);
	virtual void encode(prtx::GenerateContext& context, size_t initialShapeIndex);
	virtual void finish(prtx::GenerateContext& context);

private:
	int32_t mVoxelRes;
};

class VoxelEncoderFactory : public prtx::EncoderFactory, public prtx::Singleton<VoxelEncoderFactory> {
public:
	static VoxelEncoderFactory* createInstance();

	VoxelEncoderFactory(const prt::EncoderInfo* info) : prtx::EncoderFactory(info) { }
	virtual ~VoxelEncoderFactory() { }
	virtual VoxelEncoder* create(const prt::AttributeMap* defaultOptions, prt::Callbacks* callbacks) const {
		return new VoxelEncoder(getID(), defaultOptions, callbacks);
	}

};

#endif // VOXELENCODER_H
