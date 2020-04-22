/**
 * Esri CityEngine SDK Custom STL Decoder Example
 *
 * This example demonstrates the usage of the PRTX interface
 * to write custom decoders.
 *
 * See README.md in https://github.com/Esri/esri-cityengine-sdk for build instructions.
 *
 * Written by Simon Haegler
 * Esri R&D Center Zurich, Switzerland
 *
 * Copyright 2012-2020 (c) Esri R&D Center Zurich
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

#include "prtx/Decoder.h"
#include "prtx/DecoderFactory.h"
#include "prtx/Singleton.h"

#include <string>


class STLDecoder : public prtx::GeometryDecoder {
public:
    STLDecoder() = default;
	STLDecoder(const STLDecoder&) = delete;
	STLDecoder(STLDecoder&&) = delete;
	STLDecoder& operator=(STLDecoder&) = delete;
	virtual ~STLDecoder() = default;

	virtual void decode(
			prtx::ContentPtrVector& results,
			std::istream&           stream,
			prt::Cache*             cache,
			const std::wstring&     key,
			prtx::ResolveMap const* resolveMap,
			std::wstring&           warnings
	) override;
};


class STLDecoderFactory : public prtx::DecoderFactory, public prtx::Singleton<STLDecoderFactory> {
public:
    static STLDecoderFactory* createInstance();

    STLDecoderFactory();
	STLDecoderFactory(const STLDecoderFactory&) = delete;
	STLDecoderFactory(STLDecoderFactory&&) = delete;
	STLDecoderFactory& operator=(STLDecoderFactory&) = delete;
	virtual ~STLDecoderFactory() = default;

    virtual STLDecoder* create() const override;

    virtual const std::wstring& getID() const override;
    virtual const std::wstring& getName() const override;
    virtual const std::wstring& getDescription() const override;
    virtual prt::ContentType getContentType() const override;
};
