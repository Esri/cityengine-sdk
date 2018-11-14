/**
 * Esri CityEngine SDK CLI Example
 *
 * This example demonstrates the main functionality of the Procedural Runtime API.
 *
 * See README_<platform>.md for build instructions.
 *
 * Copyright (c) 2012-2017 Esri R&D Center Zurich
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

#include "utils.h"
#include "logging.h"

#include "prt/prt.h"
#include "prt/API.h"
#include "prt/ContentType.h"

#include <string>
#include <vector>
#include <iterator>
#include <functional>
#include <array>
#include <cstdlib>
#include <cstdio>
#include <iostream>


namespace {

/**
 * commonly used constants
 */
const char*    FILE_LOG             = "prt4cmd.log";
const wchar_t* FILE_CGA_ERROR       = L"CGAErrors.txt";
const wchar_t* FILE_CGA_PRINT       = L"CGAPrint.txt";
const wchar_t* ENCODER_ID_CGA_ERROR = L"com.esri.prt.core.CGAErrorEncoder";
const wchar_t* ENCODER_ID_CGA_PRINT = L"com.esri.prt.core.CGAPrintEncoder";
const wchar_t* ENCODER_OPT_NAME     = L"name";

/**
 * Helper struct to manage PRT lifetime (e.g. the prt::init() call)
 */
struct PRTContext {
	PRTContext(const pcu::InputArgs& inputArgs) {
		// create a console and file logger and register them with PRT
		const pcu::Path fsLogPath = inputArgs.mWorkDir / FILE_LOG;
		mLogHandler.reset(prt::ConsoleLogHandler::create(prt::LogHandler::ALL, prt::LogHandler::ALL_COUNT));
		mFileLogHandler.reset(prt::FileLogHandler::create(prt::LogHandler::ALL, prt::LogHandler::ALL_COUNT, fsLogPath.native_wstring().c_str()));
		prt::addLogHandler(mLogHandler.get());
		prt::addLogHandler(mFileLogHandler.get());

		// setup paths for plugins, assume standard SDK layout as per README.md
		const pcu::Path rootPath = inputArgs.mWorkDir;
		const pcu::Path extPath = rootPath / "lib";

		// initialize PRT with the path to its extension libraries, the desired log level
		const std::wstring wExtPath = extPath.native_wstring();
		const std::array<const wchar_t*, 1> extPaths = { wExtPath.c_str() };
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
	pcu::FileLogHandlerPtr    mFileLogHandler;
	pcu::ObjectPtr            mPRTHandle;
};

} // namespace


/**
 * the actual model generation
 */
int main(int argc, char *argv[]) {
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
		if (!inputArgs.mRulePackage.empty()) {
			LOG_INF << "Using rule package " << inputArgs.mRulePackage << std::endl;

			const std::string u8rpkURI = pcu::toFileURI(inputArgs.mRulePackage);
			prt::Status status = prt::STATUS_UNSPECIFIED_ERROR;
			resolveMap.reset(prt::createResolveMap(pcu::toUTF16FromUTF8(u8rpkURI).c_str(), nullptr, &status));
			if (resolveMap && (status == prt::STATUS_OK)) {
				LOG_DBG << "resolve map = " << pcu::objectToXML(resolveMap.get());
			}
			else {
				LOG_ERR << "getting resolve map from '" << inputArgs.mRulePackage << "' failed, aborting.";
				return EXIT_FAILURE;
			}
		}

		// -- create cache & callback
		pcu::FileOutputCallbacksPtr foc{prt::FileOutputCallbacks::create(inputArgs.mOutputPath.native_wstring().c_str())};
		pcu::CachePtr cache{prt::CacheObject::create(prt::CacheObject::CACHE_TYPE_DEFAULT)};

		// -- setup initial shape geometry
		pcu::InitialShapeBuilderPtr isb{prt::InitialShapeBuilder::create()};
		if (!inputArgs.mInitialShapeGeo.empty()) {
			LOG_DBG << L"trying to read initial shape geometry from " << inputArgs.mInitialShapeGeo;
			const prt::Status s = isb->resolveGeometry(pcu::toUTF16FromOSNarrow(inputArgs.mInitialShapeGeo).c_str(), resolveMap.get(), cache.get());
			if (s != prt::STATUS_OK) {
                LOG_ERR << "could not resolve geometry from " << inputArgs.mInitialShapeGeo;
                return EXIT_FAILURE;
            }
		}
		else {
			isb->setGeometry(
				pcu::quad::vertices, pcu::quad::vertexCount,
				pcu::quad::indices, pcu::quad::indexCount,
				pcu::quad::faceCounts, pcu::quad::faceCountsCount
			);
		}

		// -- setup initial shape attributes
		std::wstring       ruleFile  = L"bin/rule.cgb";
		std::wstring       startRule = L"default$init";
		int32_t            seed      = 666;
		const std::wstring shapeName = L"TheInitialShape";

		if (inputArgs.mInitialShapeAttrs) {
			if (inputArgs.mInitialShapeAttrs->hasKey(L"ruleFile") &&
				inputArgs.mInitialShapeAttrs->getType(L"ruleFile") == prt::AttributeMap::PT_STRING)
				ruleFile = inputArgs.mInitialShapeAttrs->getString(L"ruleFile");
			if (inputArgs.mInitialShapeAttrs->hasKey(L"startRule") &&
				inputArgs.mInitialShapeAttrs->getType(L"startRule") == prt::AttributeMap::PT_STRING)
				startRule = inputArgs.mInitialShapeAttrs->getString(L"startRule");
			if (inputArgs.mInitialShapeAttrs->hasKey(L"seed") &&
				inputArgs.mInitialShapeAttrs->getType(L"seed") == prt::AttributeMap::PT_INT)
				seed = inputArgs.mInitialShapeAttrs->getInt(L"seed");
		}

		isb->setAttributes(
				ruleFile.c_str(),
				startRule.c_str(),
				seed,
				shapeName.c_str(),
				inputArgs.mInitialShapeAttrs.get(),
				resolveMap.get()
		);

		// -- create initial shape
		const pcu::InitialShapePtr initialShape{isb->createInitialShapeAndReset()};
		const std::vector<const prt::InitialShape*> initialShapes = { initialShape.get() };

		// -- setup options for helper encoders
		const pcu::AttributeMapBuilderPtr optionsBuilder{prt::AttributeMapBuilder::create()};
		optionsBuilder->setString(ENCODER_OPT_NAME, FILE_CGA_ERROR);
		const pcu::AttributeMapPtr errOptions{optionsBuilder->createAttributeMapAndReset()};
		optionsBuilder->setString(ENCODER_OPT_NAME, FILE_CGA_PRINT);
		const pcu::AttributeMapPtr printOptions{optionsBuilder->createAttributeMapAndReset()};

		// -- validate & complete encoder options
		const pcu::AttributeMapPtr validatedEncOpts{createValidatedOptions(pcu::toUTF16FromOSNarrow(inputArgs.mEncoderID), inputArgs.mEncoderOpts)};
		const pcu::AttributeMapPtr validatedErrOpts{createValidatedOptions(ENCODER_ID_CGA_ERROR, errOptions)};
		const pcu::AttributeMapPtr validatedPrintOpts{createValidatedOptions(ENCODER_ID_CGA_PRINT, printOptions)};

		// -- setup encoder IDs and corresponding options
		const std::wstring encoder = pcu::toUTF16FromOSNarrow(inputArgs.mEncoderID);
		const std::array<const wchar_t*,3> encoders = {
				encoder.c_str(),      // our desired encoder
				ENCODER_ID_CGA_ERROR, // an encoder to redirect rule errors into CGAErrors.txt
				ENCODER_ID_CGA_PRINT  // an encoder to redirect CGA print statements to CGAPrint.txt
		};
		const std::array<const prt::AttributeMap*,3> encoderOpts = { validatedEncOpts.get(), validatedErrOpts.get(), validatedPrintOpts.get() };

		// -- THE GENERATE CALL
		const prt::Status genStat = prt::generate(
				initialShapes.data(), initialShapes.size(), nullptr,
				encoders.data(), encoders.size(), encoderOpts.data(),
				foc.get(), cache.get(), nullptr
		);
		if (genStat != prt::STATUS_OK) {
			LOG_ERR << "prt::generate() failed with status: '" << prt::getStatusDescription(genStat) << "' (" << genStat << ")";
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
