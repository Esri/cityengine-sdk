/**
 * Esri CityEngine SDK Maya Plugin Example
 *
 * This example demonstrates the main functionality of the Procedural Runtime API.
 *
 * See README.md in https://github.com/Esri/esri-cityengine-sdk for build instructions.
 *
 * Copyright (c) 2012-2019 Esri R&D Center Zurich
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "prtx/Encoder.h"
#include "prtx/EncoderFactory.h"
#include "prtx/EncoderInfoBuilder.h"
#include "prtx/Singleton.h"

#include "prt/ContentType.h"

#include <string>
#include <memory>

// Forward declarations
namespace prtx {

class Geometry;
typedef std::shared_ptr<Geometry> GeometryPtr;		///< shared Geometry pointer
typedef std::vector<GeometryPtr>  GeometryPtrVector;		///< vector of shared Geometry pointers

class Material;
typedef std::shared_ptr<Material> MaterialPtr;			///< shared Material pointer
typedef std::vector<MaterialPtr>  MaterialPtrVector;		///< vector of shared Material pointers

} // prtx


class MayaEncoder : public prtx::GeometryEncoder {
public:
	static const std::wstring ID;
	static const std::wstring NAME;
	static const std::wstring DESCRIPTION;

public:
	MayaEncoder(const std::wstring& id, const prt::AttributeMap* options, prt::Callbacks* callbacks);
	virtual ~MayaEncoder() { }

public:
	virtual void init(prtx::GenerateContext&) override;
	virtual void encode(prtx::GenerateContext& context, size_t initialShapeIndex) override;
	virtual void finish(prtx::GenerateContext&) override { }

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

	virtual MayaEncoder* create(const prt::AttributeMap* options, prt::Callbacks* callbacks) const override {
		return new MayaEncoder(getID(), options, callbacks);
	}
};
