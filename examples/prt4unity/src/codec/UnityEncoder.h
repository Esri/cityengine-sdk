/**
 * Esri CityEngine SDK Unity Plugin Example
 *
 * This example demonstrates the main functionality of the Procedural Runtime API.
 * Esri R&D Center Zurich, Switzerland
 *
 * See http://github.com/ArcGIS/esri-cityengine-sdk for instructions.
 */

#ifndef UNITYENCODER_H_
#define UNITYENCODER_H_

#include <string>
#include <iostream>
#include <stdexcept>

#include "prt/ContentType.h"
#include "prt/InitialShape.h"

#include "prtx/ResolveMap.h"
#include "prtx/Encoder.h"
#include "prtx/EncoderFactory.h"
#include "prtx/EncoderInfoBuilder.h"
#include "prtx/PRTUtils.h"
#include "prtx/Singleton.h"
#include "prtx/EncodePreparator.h"

class IUnityOutputHandler;

class UnityEncoder : public prtx::GeometryEncoder {
public:
	static const std::wstring ID;
	static const std::wstring NAME;
	static const std::wstring DESCRIPTION;

public:
	UnityEncoder(const std::wstring& id, const prt::AttributeMap* options, prt::Callbacks* callbacks);
	virtual ~UnityEncoder();

public:
	virtual void init(prtx::GenerateContext& context);
	virtual void encode(prtx::GenerateContext& context, size_t initialShapeIndex);
	virtual void finish(prtx::GenerateContext& context);

private:
	void convertGeometry(const wchar_t* cgbName, const std::vector<prtx::EncodePreparator::FinalizedInstance>& instances, IUnityOutputHandler* unityOutput);
};


class UnityEncoderFactory : public prtx::EncoderFactory, public prtx::Singleton<UnityEncoderFactory> {
public:
	static UnityEncoderFactory* createInstance() {
		prtx::EncoderInfoBuilder encoderInfoBuilder;

		encoderInfoBuilder.setID(UnityEncoder::ID);
		encoderInfoBuilder.setName(UnityEncoder::NAME);
		encoderInfoBuilder.setDescription(UnityEncoder::DESCRIPTION);
		encoderInfoBuilder.setType(prt::CT_GEOMETRY);

		return new UnityEncoderFactory(encoderInfoBuilder.create());
	}

	UnityEncoderFactory(const prt::EncoderInfo* info) : prtx::EncoderFactory(info) { }
	virtual ~UnityEncoderFactory() { }

	virtual UnityEncoder* create(const prt::AttributeMap* options, prt::Callbacks* callbacks) const {
		return new UnityEncoder(getID(), options, callbacks);
	}
};


#endif /* UNITYENCODER_H_ */
