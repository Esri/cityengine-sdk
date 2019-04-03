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

#include "maya/MStatus.h"
#include "maya/MFloatPointArray.h"

#include "prt/Object.h"
#include "prt/AttributeMap.h"
#include "prt/InitialShape.h"
#include "prt/RuleFileInfo.h"
#include "prt/EncoderInfo.h"
#include "prt/OcclusionSet.h"

#include <memory>
#include <cstdint>
#include <cmath>
#include <vector>
#include <algorithm>

#if defined(_MSC_VER) && (_MSC_VER <= 1700)
#   include <cfloat>
#endif


#define DO_DBG 1
#define MCHECK(_stat_) {if(MS::kSuccess != _stat_) { prtu::dbg("maya err at %s:%d: %s %d\n", __FILE__, __LINE__, _stat_.errorString().asChar(), _stat_.statusCode());}}

struct PRTDestroyer {
	void operator()(prt::Object const* p) {
		if (p) p->destroy();
	}
};

using ObjectUPtr = std::unique_ptr<const prt::Object, PRTDestroyer>;
using InitialShapeNOPtrVector = std::vector<const prt::InitialShape*>;
using AttributeMapNOPtrVector = std::vector<const prt::AttributeMap*>;
using CacheObjectUPtr = std::unique_ptr<prt::CacheObject, PRTDestroyer>;
using AttributeMapUPtr = std::unique_ptr<const prt::AttributeMap, PRTDestroyer>;
using AttributeMapVector = std::vector<AttributeMapUPtr>;
using AttributeMapBuilderUPtr = std::unique_ptr<prt::AttributeMapBuilder, PRTDestroyer>;
using AttributeMapBuilderVector = std::vector<AttributeMapBuilderUPtr>;
using InitialShapeBuilderUPtr = std::unique_ptr<prt::InitialShapeBuilder, PRTDestroyer>;
using InitialShapeBuilderVector = std::vector<InitialShapeBuilderUPtr>;
using ResolveMapUPtr = std::unique_ptr<const prt::ResolveMap, PRTDestroyer>;
using ResolveMapBuilderUPtr = std::unique_ptr<prt::ResolveMapBuilder, PRTDestroyer>;
using RuleFileInfoUPtr = std::unique_ptr<const prt::RuleFileInfo, PRTDestroyer>;
using EncoderInfoUPtr = std::unique_ptr<const prt::EncoderInfo, PRTDestroyer>;
using OcclusionSetUPtr = std::unique_ptr<prt::OcclusionSet, PRTDestroyer>;

namespace prtu {

	template<typename C, typename D>
	std::vector<const C*> toPtrVec(const std::vector<std::unique_ptr<C, D>>& sv) {
		std::vector<const C*> pv(sv.size());
		std::transform(sv.begin(), sv.end(), pv.begin(), [](const std::unique_ptr<C, D>& s) { return s.get(); });
		return pv;
	}

	const char* filename(const char* path);

	void dbg(const char* fmt, ...);
	void wdbg(const wchar_t* fmt, ...);

	template<typename C> C getDirSeparator();
	template<> char getDirSeparator();
	template<> wchar_t getDirSeparator();

	int fromHex(wchar_t c);
	wchar_t toHex(int i);
	void toHex(wchar_t* color, double r, double g, double b);
	MString toCleanId(const MString& name);

	int32_t computeSeed(MFloatPointArray& vertices);
	int32_t computeSeed(const double* vertices, size_t count);

	inline bool isnan(double d) {
#if defined(_MSC_VER) && (_MSC_VER <= 1700)
		return ::isnan(d);
#else
		return std::isnan(d);
#endif
	}

} // namespace prtu
