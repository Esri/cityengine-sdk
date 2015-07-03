/**
 * Esri CityEngine SDK Custom STL Encoder Example
 *
 * This example demonstrates the usage of the PRTX interface
 * to write custom encoders.
 *
 * See README.md in http://github.com/ArcGIS/esri-cityengine-sdk for build instructions.
 *
 * Written by Simon Haegler
 * Esri R&D Center Zurich, Switzerland
 *
 * Copyright 2015 Esri R&D Center Zurich
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

#ifndef PRT_EXAMPLE_STLENCODER_H_
#define PRT_EXAMPLE_STLENCODER_H_

#include <string>

#include "prt/AttributeMap.h"
#include "prt/Callbacks.h"

#include "prtx/prtx.h"
#include "prtx/Encoder.h"
#include "prtx/EncodePreparator.h"
#include "prtx/EncoderFactory.h"
#include "prtx/Singleton.h"


// forward declare some classes to reduce header inclusion
namespace prtx {
class GenerateContext;
}

class STLEncoder : public prtx::GeometryEncoder {
public:
	static const std::wstring ID;
	static const std::wstring NAME;
	static const std::wstring DESCRIPTION;

	STLEncoder(const std::wstring& id, const prt::AttributeMap* defaultOptions, prt::Callbacks* callbacks);
	virtual ~STLEncoder();

	virtual void init(prtx::GenerateContext& context);
	virtual void encode(prtx::GenerateContext& context, size_t initialShapeIndex);
	virtual void finish(prtx::GenerateContext& context);

private:
	prtx::DefaultNamePreparator	mNamePreparator;
	prtx::EncodePreparatorPtr	mEncodePreparator;
};


class STLEncoderFactory : public prtx::EncoderFactory, public prtx::Singleton<STLEncoderFactory> {
public:
	static STLEncoderFactory* createInstance();

	STLEncoderFactory(const prt::EncoderInfo* info) : prtx::EncoderFactory(info) { }
	virtual ~STLEncoderFactory() { }
	virtual STLEncoder* create(const prt::AttributeMap* defaultOptions, prt::Callbacks* callbacks) const {
		return new STLEncoder(getID(), defaultOptions, callbacks);
	}

};


#endif /* PRT_EXAMPLE_STLENCODER_H_ */
