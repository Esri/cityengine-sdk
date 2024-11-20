/**
 * CityEngine SDK CLI Example
 *
 * This example demonstrates the main functionality of the Procedural Runtime API.
 *
 * See README.md in https://github.com/Esri/cityengine-sdk for build instructions.
 *
 * Copyright (c) 2012-2024 Esri R&D Center Zurich
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

#include "logging.h"
#include "utils.h"

#include "prt/API.h"

#include <array>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace {

/**
 * commonly used constants
 */
const char* FILE_LOG = "prt4cmd.log";
const wchar_t* FILE_CGA_ERROR = L"CGAErrors.txt";
const wchar_t* FILE_CGA_PRINT = L"CGAPrint.txt";
const wchar_t* ENCODER_ID_CGA_ERROR = L"com.esri.prt.core.CGAErrorEncoder";
const wchar_t* ENCODER_ID_CGA_PRINT = L"com.esri.prt.core.CGAPrintEncoder";
const wchar_t* ENCODER_OPT_NAME = L"name";

/**
 * Helper struct to manage PRT lifetime (e.g. the prt::init() call)
 */
struct PRTContext {
	explicit PRTContext(const pcu::InputArgs& inputArgs) {
		// create a console and file logger and register them with PRT
		const std::filesystem::path fsLogPath = inputArgs.mInstallRootPath / FILE_LOG;
		mLogHandler.reset(prt::ConsoleLogHandler::create(prt::LogHandler::ALL, prt::LogHandler::ALL_COUNT));
		mFileLogHandler.reset(prt::FileLogHandler::create(prt::LogHandler::ALL, prt::LogHandler::ALL_COUNT,
		                                                  fsLogPath.wstring().c_str()));
		prt::addLogHandler(mLogHandler.get());
		prt::addLogHandler(mFileLogHandler.get());

		// setup paths for plugins, assume standard SDK layout as per README.md
		const std::filesystem::path rootPath = inputArgs.mInstallRootPath;
		const std::filesystem::path extPath = rootPath / "lib";

		// initialize PRT with the path to its extension libraries, the desired log level
		const std::wstring wExtPath = extPath.wstring();
		const std::array<const wchar_t*, 1> extPaths = {wExtPath.c_str()};
		mPRTHandle.reset(prt::init(extPaths.data(), extPaths.size(), (prt::LogLevel)inputArgs.mLogLevel));
	}

	~PRTContext() {
		// shutdown PRT
		mPRTHandle.reset();

		// remove loggers
		prt::removeLogHandler(mLogHandler.get());
		prt::removeLogHandler(mFileLogHandler.get());
	}

	explicit operator bool() const {
		return (bool)mPRTHandle;
	}

	pcu::ConsoleLogHandlerPtr mLogHandler;
	pcu::FileLogHandlerPtr mFileLogHandler;
	pcu::ObjectPtr mPRTHandle;
};

} // namespace

/**
 * the actual model generation
 */
int main(int argc, char* argv[]) {
	try {
		// -- fetch command line args
		const pcu::InputArgs inputArgs(argc, argv);
		if (!inputArgs.readyToRumble())
			return static_cast<int>(inputArgs.mStatus);

		// -- initialize PRT via the above helper struct
		const PRTContext prtCtx(inputArgs);
		if (!prtCtx) {
			LOG_ERR << L"Failed to start PRT, aborting.";
			return EXIT_FAILURE;
		}

		// -- handle the info option (must happen after successful init)
		if (!inputArgs.mInfoFile.empty()) {
			const pcu::RunStatus s = pcu::codecInfoToXML(inputArgs.mInfoFile);
			return static_cast<int>(s);
		}

		// -- create resolve map based on rule package
		pcu::ResolveMapPtr resolveMap;
		if (!inputArgs.mRulePackageURI.empty()) {
			LOG_INF << "Using rule package " << inputArgs.mRulePackageURI << std::endl;

			prt::Status status = prt::STATUS_UNSPECIFIED_ERROR;
			resolveMap.reset(
			        prt::createResolveMap(pcu::toUTF16FromUTF8(inputArgs.mRulePackageURI).c_str(), nullptr, &status));
			if (resolveMap && (status == prt::STATUS_OK)) {
				LOG_DBG << "resolve map = " << pcu::objectToXML(resolveMap.get());
			}
			else {
				LOG_ERR << "getting resolve map from '" << inputArgs.mRulePackageURI << "' failed, aborting.";
				return EXIT_FAILURE;
			}
		}

		// -- create cache & callback
		pcu::FileOutputCallbacksPtr foc{prt::FileOutputCallbacks::create(inputArgs.mOutputPath.wstring().c_str())};
		pcu::CachePtr cache{prt::CacheObject::create(prt::CacheObject::CACHE_TYPE_DEFAULT)};

		// -- setup initial shape geometry
		pcu::InitialShapeBuilderPtr isb{prt::InitialShapeBuilder::create()};
		if (!inputArgs.mInitialShapeGeoURI.empty()) {
			LOG_DBG << L"trying to read initial shape geometry from " << inputArgs.mInitialShapeGeoURI;
			const prt::Status s = isb->resolveGeometry(pcu::toUTF16FromOSNarrow(inputArgs.mInitialShapeGeoURI).c_str(),
			                                           resolveMap.get(), cache.get());
			if (s != prt::STATUS_OK) {
				LOG_ERR << "could not resolve geometry from " << inputArgs.mInitialShapeGeoURI;
				return EXIT_FAILURE;
			}
		}
		else {
			isb->setGeometry(pcu::quad::vertices, pcu::quad::vertexCount, pcu::quad::indices, pcu::quad::indexCount,
			                 pcu::quad::faceCounts, pcu::quad::faceCountsCount);
		}

		// -- setup initial shape attributes
		int32_t seed = 666;
		const std::wstring shapeName = L"TheInitialShape";

		// ---- seed
		if (inputArgs.mInitialShapeAttrs) {
			if (inputArgs.mInitialShapeAttrs->hasKey(L"seed") &&
			    inputArgs.mInitialShapeAttrs->getType(L"seed") == prt::AttributeMap::PT_INT)
				seed = inputArgs.mInitialShapeAttrs->getInt(L"seed");
		}

		// ---- cgb
		prt::Status status = prt::STATUS_UNSPECIFIED_ERROR;
		const wchar_t* const cgbKey = resolveMap->findCGBKey(&status);
		if (cgbKey == nullptr || status != prt::STATUS_OK) {
			LOG_ERR << "getting CGB Key from '" << inputArgs.mRulePackageURI << "' failed, aborting.";
			return EXIT_FAILURE;
		}

		LOG_INF << "Using rule file " << cgbKey;

		// ---- start rule
		const wchar_t* const cgbURI = resolveMap->getString(cgbKey, &status);
		if(cgbURI == nullptr || status != prt::STATUS_OK) {
			LOG_ERR << "getting CGB URI from '" << inputArgs.mRulePackageURI << "' failed, aborting.";
			return EXIT_FAILURE;
		}

		pcu::RuleFileInfoPtr info{prt::createRuleFileInfo(cgbURI, cache.get(), &status)};
		if(!info || status != prt::STATUS_OK) {
			LOG_ERR << "getting rule file info from '" << inputArgs.mRulePackageURI << "' failed, aborting.";
			return EXIT_FAILURE;
		}

		const prt::RuleFileInfo::Entry* startRule = nullptr;
		for (size_t ri = 0; ri < info->getNumRules(); ri++) {
			const prt::RuleFileInfo::Entry* const ruleEntry = info->getRule(ri);
			if(ruleEntry->getNumParameters() > 0) continue;
			for (size_t ai = 0; ai < ruleEntry->getNumAnnotations(); ai++) {
				if (wcscmp(ruleEntry->getAnnotation(ai)->getName(), L"@StartRule") == 0) {
					startRule = ruleEntry;
					break;
				}
			}
			if (startRule != nullptr) break;
		}

		if (startRule == nullptr) {
			LOG_ERR << "getting start rule from '" << inputArgs.mRulePackageURI << "' failed, aborting.";
			return EXIT_FAILURE;
		}

		LOG_INF << "Using start rule " << startRule->getName();

		isb->setAttributes(cgbKey, startRule->getName(), seed, shapeName.c_str(),
		                   inputArgs.mInitialShapeAttrs.get(), resolveMap.get());

		// -- create initial shape
		const pcu::InitialShapePtr initialShape{isb->createInitialShapeAndReset()};
		const std::vector<const prt::InitialShape*> initialShapes = {initialShape.get()};

		// -- setup options for helper encoders
		const pcu::AttributeMapBuilderPtr optionsBuilder{prt::AttributeMapBuilder::create()};
		optionsBuilder->setString(ENCODER_OPT_NAME, FILE_CGA_ERROR);
		const pcu::AttributeMapPtr errOptions{optionsBuilder->createAttributeMapAndReset()};
		optionsBuilder->setString(ENCODER_OPT_NAME, FILE_CGA_PRINT);
		const pcu::AttributeMapPtr printOptions{optionsBuilder->createAttributeMapAndReset()};

		// -- validate & complete encoder options
		const pcu::AttributeMapPtr validatedEncOpts{
		        createValidatedOptions(pcu::toUTF16FromOSNarrow(inputArgs.mEncoderID), inputArgs.mEncoderOpts)};
		const pcu::AttributeMapPtr validatedErrOpts{createValidatedOptions(ENCODER_ID_CGA_ERROR, errOptions)};
		const pcu::AttributeMapPtr validatedPrintOpts{createValidatedOptions(ENCODER_ID_CGA_PRINT, printOptions)};

		// -- setup encoder IDs and corresponding options
		const std::wstring encoder = pcu::toUTF16FromOSNarrow(inputArgs.mEncoderID);
		const std::array<const wchar_t*, 3> encoders = {
		        encoder.c_str(),      // our desired encoder
		        ENCODER_ID_CGA_ERROR, // an encoder to redirect rule errors into CGAErrors.txt
		        ENCODER_ID_CGA_PRINT  // an encoder to redirect CGA print statements to CGAPrint.txt
		};
		const std::array<const prt::AttributeMap*, 3> encoderOpts = {validatedEncOpts.get(), validatedErrOpts.get(),
		                                                             validatedPrintOpts.get()};

		// -- THE GENERATE CALL
		const prt::Status genStat = prt::generate(initialShapes.data(), initialShapes.size(), nullptr, encoders.data(),
		                                          encoders.size(), encoderOpts.data(), foc.get(), cache.get(), nullptr);
		if (genStat != prt::STATUS_OK) {
			LOG_ERR << "prt::generate() failed with status: '" << prt::getStatusDescription(genStat) << "' (" << genStat
			        << ")";
		}

		return EXIT_SUCCESS;
	}
	catch (const std::exception& e) {
		std::cerr << "caught exception: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	catch (...) {
		std::cerr << "caught unknown exception. " << std::endl;
		return EXIT_FAILURE;
	}
}
