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
#include "prt/FlexLicParams.h"

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
const char*    FILE_FLEXNET_LIB     = "flexnet_prt";
const char*    FILE_LOG             = "prt4cmd.log";
const wchar_t* FILE_CGA_ERROR       = L"CGAErrors.txt";
const wchar_t* FILE_CGA_PRINT       = L"CGAPrint.txt";
const wchar_t* ENCODER_ID_CGA_ERROR = L"com.esri.prt.core.CGAErrorEncoder";
const wchar_t* ENCODER_ID_CGA_PRINT = L"com.esri.prt.core.CGAPrintEncoder";
const wchar_t* ENCODER_OPT_NAME     = L"name";

/**
 * Helper struct to manage PRT lifetime (e.g. the prt::init() call) and licensing
 */
struct PRTContext {
	PRTContext(const pcu::InputArgs& inputArgs) {
		// create a console and file logger and register them with PRT
		pcu::Path fsLogPath = inputArgs.mWorkDir / FILE_LOG;
		mLogHandler.reset(prt::ConsoleLogHandler::create(prt::LogHandler::ALL, prt::LogHandler::ALL_COUNT));
		mFileLogHandler.reset(prt::FileLogHandler::create(prt::LogHandler::ALL, prt::LogHandler::ALL_COUNT, fsLogPath.native_wstring().c_str()));
		prt::addLogHandler(mLogHandler.get());
		prt::addLogHandler(mFileLogHandler.get());

		// setup paths for plugins and licensing, assume standard SDK layout as per README.md
		pcu::Path rootPath = inputArgs.mWorkDir;
		pcu::Path extPath = rootPath / "lib";
		std::string fsFlexLibBaseName = pcu::getSharedLibraryPrefix() + FILE_FLEXNET_LIB + pcu::getSharedLibrarySuffix();
		pcu::Path fsFlexLib = rootPath / "bin" / fsFlexLibBaseName;
		std::string flexLib = fsFlexLib.native_string();

		// setup the licensing information
		prt::FlexLicParams flp;
		flp.mActLibPath = flexLib.c_str();
		flp.mFeature    = inputArgs.mLicFeature.c_str();
		flp.mHostName   = inputArgs.mLicHost.c_str();

		// initialize PRT with the path to its extension libraries, the desired log level and licensing data
		const std::wstring wExtPath = extPath.native_wstring();
		std::array<const wchar_t*, 1> extPaths = { wExtPath.c_str() };
		mLicHandle.reset(prt::init(extPaths.data(), extPaths.size(), (prt::LogLevel)inputArgs.mLogLevel, &flp));
	}

	~PRTContext() {
		// release PRT license
		mLicHandle.reset();

		// remove loggers
		prt::removeLogHandler(mLogHandler.get());
		prt::removeLogHandler(mFileLogHandler.get());
	}

	explicit operator bool() const {
		return (bool)mLicHandle;
	}

	pcu::ConsoleLogHandlerPtr mLogHandler;
	pcu::FileLogHandlerPtr    mFileLogHandler;
	pcu::ObjectPtr            mLicHandle;
};

} // namespace


/**
 * the actual model generation
 */
int main (int argc, char *argv[]) {
	try {
		// -- fetch command line args
		pcu::InputArgs inputArgs(argc, argv);
		if (!inputArgs.readyToRumble())
			return static_cast<int>(inputArgs.mStatus);

		// -- initialize PRT via the above helper struct
		PRTContext prtCtx(inputArgs);
		if (!prtCtx) {
			LOG_ERR << L"failed to get a CityEngine license, bailing out.";
			return EXIT_FAILURE;
		}

        // handle the info option (must happen after successful init)
        if (!inputArgs.mInfoFile.empty()) {
            pcu::RunStatus s = pcu::codecInfoToXML(inputArgs.mInfoFile);
            return static_cast<int>(s);
        }

		// -- create resolve map based on rule package
		pcu::ResolveMapPtr resolveMap;
		if (!inputArgs.mRulePackage.empty()) {
			LOG_INF << "Using rule package " << inputArgs.mRulePackage << std::endl;

			std::string u8rpkURI = pcu::toFileURI(inputArgs.mRulePackage);
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
			prt::Status s = isb->resolveGeometry(pcu::toUTF16FromOSNarrow(inputArgs.mInitialShapeGeo).c_str(), resolveMap.get(), cache.get());
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
		std::wstring ruleFile  = L"bin/rule.cgb";
		std::wstring startRule = L"default$init";
		int32_t      seed      = 666;
		std::wstring shapeName = L"TheInitialShape";

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
		pcu::InitialShapePtr initialShape{isb->createInitialShapeAndReset()};
		std::vector<const prt::InitialShape*> initialShapes = { initialShape.get() };

		// -- setup options for helper encoders
		pcu::AttributeMapBuilderPtr optionsBuilder{prt::AttributeMapBuilder::create()};
		optionsBuilder->setString(ENCODER_OPT_NAME, FILE_CGA_ERROR);
		pcu::AttributeMapPtr errOptions{optionsBuilder->createAttributeMapAndReset()};
		optionsBuilder->setString(ENCODER_OPT_NAME, FILE_CGA_PRINT);
		pcu::AttributeMapPtr printOptions{optionsBuilder->createAttributeMapAndReset()};

		// -- validate & complete encoder options
		pcu::AttributeMapPtr validatedEncOpts{createValidatedOptions(pcu::toUTF16FromOSNarrow(inputArgs.mEncoderID), inputArgs.mEncoderOpts)};
		pcu::AttributeMapPtr validatedErrOpts{createValidatedOptions(ENCODER_ID_CGA_ERROR, errOptions)};
		pcu::AttributeMapPtr validatedPrintOpts{createValidatedOptions(ENCODER_ID_CGA_PRINT, printOptions)};

		// -- setup encoder IDs and corresponding options
		std::wstring encoder = pcu::toUTF16FromOSNarrow(inputArgs.mEncoderID);
		std::array<const wchar_t*,3> encoders = {
				encoder.c_str(),      // our desired encoder
				ENCODER_ID_CGA_ERROR, // an encoder to redirect rule errors into CGAErrors.txt
				ENCODER_ID_CGA_PRINT  // an encoder to redirect CGA print statements to CGAPrint.txt
		};
		std::array<const prt::AttributeMap*,3> encoderOpts = { validatedEncOpts.get(), validatedErrOpts.get(), validatedPrintOpts.get() };

		// -- THE GENERATE CALL
		prt::Status genStat = prt::generate(
				initialShapes.data(), initialShapes.size(), nullptr,
				encoders.data(), encoders.size(), encoderOpts.data(),
				foc.get(), cache.get(), nullptr
		);
		if (genStat != prt::STATUS_OK) {
			LOG_ERR << "prt::generate() failed with status: '" << prt::getStatusDescription(genStat) << "' (" << genStat << ")";
		}

		return EXIT_SUCCESS;
	}
	catch (std::exception& e) {
		std::cerr << "caught exception: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	catch (...) {
		std::cerr << "caught unknown exception. " << std::endl;
		return EXIT_FAILURE;
	}
}
