/**
 * CityEngine SDK Custom STL Encoder Example
 *
 * This example demonstrates the usage of the PRTX interface
 * to write custom encoders.
 *
 * See README.md in https://github.com/Esri/cityengine-sdk for build instructions.
 *
 * Written by Simon Haegler
 * Esri R&D Center Zurich, Switzerland
 *
 * Copyright 2012-2022 (c) Esri R&D Center Zurich
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

#include "prtx/prtx.h"
#include "prtx/Encoder.h"
#include "prtx/EncodePreparator.h"
#include "prtx/EncoderFactory.h"
#include "prtx/Singleton.h"

#include "prt/AttributeMap.h"
#include "prt/Callbacks.h"

#include <string>


// forward declare some classes to reduce header inclusion
namespace prtx {
class GenerateContext;
}

class STLEncoder : public prtx::GeometryEncoder {
public:
	static const std::wstring ID;
	static const std::wstring NAME;
	static const std::wstring DESCRIPTION;

	using prtx::GeometryEncoder::GeometryEncoder; // re-use parent constructor

	STLEncoder(const STLEncoder&) = delete;
	STLEncoder(STLEncoder&&) = delete;
	STLEncoder& operator=(STLEncoder&) = delete;
	virtual ~STLEncoder() = default;

	virtual void init(prtx::GenerateContext& context) override;
	virtual void encode(prtx::GenerateContext& context, size_t initialShapeIndex) override;
	virtual void finish(prtx::GenerateContext& context) override;

private:
	prtx::DefaultNamePreparator mNamePreparator;
	prtx::EncodePreparatorPtr   mEncodePreparator;
};


class STLEncoderFactory : public prtx::EncoderFactory, public prtx::Singleton<STLEncoderFactory> {
public:
	static STLEncoderFactory* createInstance();

	STLEncoderFactory(const prt::EncoderInfo* info) : prtx::EncoderFactory(info) { }
	STLEncoderFactory(const STLEncoderFactory&) = delete;
	STLEncoderFactory(STLEncoderFactory&&) = delete;
	STLEncoderFactory& operator=(STLEncoderFactory&) = delete;
	virtual ~STLEncoderFactory() = default;

	virtual STLEncoder* create(const prt::AttributeMap* defaultOptions, prt::Callbacks* callbacks) const override {
		return new STLEncoder(getID(), defaultOptions, callbacks);
	}

};
