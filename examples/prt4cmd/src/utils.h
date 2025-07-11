/**
 * CityEngine SDK CLI Example
 *
 * This example demonstrates the main functionality of the Procedural Runtime API.
 *
 * See README.md in https://github.com/Esri/cityengine-sdk for build instructions.
 *
 * Copyright (c) 2012-2025 Esri R&D Center Zurich
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

#include "prt/API.h"
#include "prt/FileOutputCallbacks.h"
#include "prt/LogHandler.h"

#include <cstdlib>
#include <filesystem>
#include <memory>
#include <string>

namespace pcu { // prt4cmd utils

enum class RunStatus : uint8_t { DONE = EXIT_SUCCESS, FAILED = EXIT_FAILURE, CONTINUE = 2 };

/**
 * helpers for prt object management
 */
struct PRTDestroyer {
	void operator()(const prt::Object* p) const {
		if (p)
			p->destroy();
	}
};

using ObjectPtr = std::unique_ptr<const prt::Object, PRTDestroyer>;
using CachePtr = std::unique_ptr<prt::CacheObject, PRTDestroyer>;
using ResolveMapPtr = std::unique_ptr<const prt::ResolveMap, PRTDestroyer>;
using InitialShapePtr = std::unique_ptr<const prt::InitialShape, PRTDestroyer>;
using InitialShapeBuilderPtr = std::unique_ptr<prt::InitialShapeBuilder, PRTDestroyer>;
using AttributeMapPtr = std::unique_ptr<const prt::AttributeMap, PRTDestroyer>;
using AttributeMapBuilderPtr = std::unique_ptr<prt::AttributeMapBuilder, PRTDestroyer>;
using FileOutputCallbacksPtr = std::unique_ptr<prt::FileOutputCallbacks, PRTDestroyer>;
using ConsoleLogHandlerPtr = std::unique_ptr<prt::ConsoleLogHandler, PRTDestroyer>;
using FileLogHandlerPtr = std::unique_ptr<prt::FileLogHandler, PRTDestroyer>;
using EncoderInfoPtr = std::unique_ptr<const prt::EncoderInfo, PRTDestroyer>;
using DecoderInfoPtr = std::unique_ptr<const prt::DecoderInfo, PRTDestroyer>;
using RuleFileInfoPtr = std::unique_ptr<const prt::RuleFileInfo, PRTDestroyer>;

/**
 * prt encoder options helpers
 */
AttributeMapPtr createValidatedOptions(const std::wstring& encID, const AttributeMapPtr& unvalidatedOptions);

/**
 * string and URI helpers
 */
using URI = std::string;

std::string toOSNarrowFromUTF16(const std::wstring& osWString);
std::wstring toUTF16FromOSNarrow(const std::string& osString);
std::wstring toUTF16FromUTF8(const std::string& utf8String);
std::string toUTF8FromOSNarrow(const std::string& osString);
std::string percentEncode(const std::string& utf8String);
URI toFileURI(const std::string& p);

/**
 * XML helpers
 */
std::string objectToXML(const prt::Object* obj);
RunStatus codecInfoToXML(const std::filesystem::path& infoFilePath);

/**
 * default initial shape geometry (a quad)
 */
namespace quad {
const double vertices[] = {0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0};
const size_t vertexCount = 12;
const uint32_t indices[] = {0, 1, 2, 3};
const size_t indexCount = 4;
const uint32_t faceCounts[] = {4};
const size_t faceCountsCount = 1;
} // namespace quad

inline std::ostream& operator<<(std::ostream& out, const std::filesystem::path& p) {
	out << p.generic_string();
	return out;
}

inline std::wostream& operator<<(std::wostream& out, const std::filesystem::path& p) {
	out << p.generic_wstring();
	return out;
}

/**
 * command line argument helper
 */
struct InputArgs {
	InputArgs(int argc, char* argv[]);
	bool readyToRumble() const {
		return (mStatus == RunStatus::CONTINUE);
	}

	std::filesystem::path mInstallRootPath;
	std::filesystem::path mOutputPath;
	std::string mEncoderID;
	AttributeMapPtr mEncoderOpts;
	URI mRulePackageURI;
	AttributeMapPtr mInitialShapeAttrs;
	URI mInitialShapeGeoURI;
	int mLogLevel;
	std::filesystem::path mInfoFile;
	RunStatus mStatus;
};

} // namespace pcu
