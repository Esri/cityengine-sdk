/**
 * Esri CityEngine SDK Maya Plugin Example
 *
 * This example demonstrates the main functionality of the Procedural Runtime API.
 * Esri R&D Center Zurich, Switzerland
 *
 * See http://github.com/ArcGIS/esri-cityengine-sdk for instructions.
 */

#pragma once

#include "prtx/Encoder.h"
#include "prtx/EncoderFactory.h"
#include "prtx/EncoderInfoBuilder.h"
#include "prtx/Singleton.h"

#include "prt/ContentType.h"

#include <string>
#include <memory>


class MayaEncoder : public prtx::GeometryEncoder {
public:
	static const std::wstring ID;
	static const std::wstring NAME;
	static const std::wstring DESCRIPTION;

public:
	MayaEncoder(const std::wstring& id, const prt::AttributeMap* options, prt::Callbacks* callbacks);
	virtual ~MayaEncoder() { }

public:
	virtual void init(prtx::GenerateContext&);
	virtual void encode(prtx::GenerateContext& context, size_t initialShapeIndex);
	virtual void finish(prtx::GenerateContext&) { }

private:
	void convertGeometry(
			const std::wstring& cgbName,
			const prtx::GeometryPtrVector& geometries,
			const std::vector<prtx::MaterialPtrVector>& mat
	);
};

class MayaEncoderFactory : public prtx::EncoderFactory, public prtx::Singleton<MayaEncoderFactory> {
public:
	static MayaEncoderFactory* createInstance() {
		prtx::EncoderInfoBuilder encoderInfoBuilder;

		encoderInfoBuilder.setID(MayaEncoder::ID);
		encoderInfoBuilder.setName(MayaEncoder::NAME);
		encoderInfoBuilder.setDescription(MayaEncoder::DESCRIPTION);
		encoderInfoBuilder.setType(prt::CT_GEOMETRY);

		return new MayaEncoderFactory(encoderInfoBuilder.create());
	}

	MayaEncoderFactory(const prt::EncoderInfo* info) : prtx::EncoderFactory(info) { }
	virtual ~MayaEncoderFactory() { }

	virtual MayaEncoder* create(const prt::AttributeMap* options, prt::Callbacks* callbacks) const {
		return new MayaEncoder(getID(), options, callbacks);
	}
};
