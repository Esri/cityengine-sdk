/**
 * Esri CityEngine SDK CLI Example
 *
 * This example demonstrates the main functionality of the Procedural Runtime API.
 *
 * See README for build instructions.
 *
 * Written by Matthias Specht and Simon Haegler
 * Esri R&D Center Zurich, Switzerland
 *
 * Copyright 2016 Esri R&D Center Zurich
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

#ifdef _WIN32
#define _SCL_SECURE_NO_WARNINGS
#endif

#include "prt/prt.h"
#include "prt/API.h"
#include "prt/FileOutputCallbacks.h"
#include "prt/LogHandler.h"
#include "prt/ContentType.h"
#include "prt/FlexLicParams.h"
#include "prt/StringUtils.h"

#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"
#include "boost/algorithm/string.hpp"

#include <memory>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iterator>


/**
 * commonly used constants
 */
const char*		FILE_FLEXNET_LIB		= "flexnet_prt";
const char* 	FILE_LOG				= "prt4cmd.log";
const wchar_t*	FILE_CGA_ERROR			= L"CGAErrors.txt";
const wchar_t*	FILE_CGA_PRINT			= L"CGAPrint.txt";
const wchar_t*	ENCODER_ID_CGA_ERROR	= L"com.esri.prt.core.CGAErrorEncoder";
const wchar_t*	ENCODER_ID_CGA_PRINT	= L"com.esri.prt.core.CGAPrintEncoder";
const wchar_t*	ENCODER_ID_OBJ			= L"com.esri.prt.codecs.OBJEncoder";


/**
 * default initial shape geometry (a quad)
 */
namespace UnitQuad {
const double 	vertices[]		= { 0, 0, 0,  0, 0, 1,  1, 0, 1,  1, 0, 0 };
const size_t 	vertexCount		= 12;
const uint32_t	indices[]		= { 0, 1, 2, 3 };
const size_t 	indexCount		= 4;
const uint32_t 	faceCounts[]	= { 4 };
const size_t 	faceCountsCount	= 1;
}


/**
 * helpers for prt object management
 */
struct PRTDestroyer { void operator()(prt::Object const* p) const { if (p) p->destroy(); } };
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


/**
 * fwd declarations of helper functions
 */
std::string getSharedLibraryPrefix();
std::string getSharedLibrarySuffix();


/**
 * Helper struct to transport command line arguments
 */
typedef std::map<std::wstring, std::wstring> StringStringMap;
struct InputArgs {
	InputArgs() : mEncoderOpts(0), mInitialShapeAttrs(0) { }
	~InputArgs() {
		if (mEncoderOpts != 0) mEncoderOpts->destroy();
		if (mInitialShapeAttrs != 0) mInitialShapeAttrs->destroy();
	}
	
	boost::filesystem::path		mWorkDir;
	std::wstring				mEncoderID;
	const prt::AttributeMap*	mEncoderOpts;
	boost::filesystem::path		mOutputPath;
	std::string					mRulePackage;
	const prt::AttributeMap*	mInitialShapeAttrs;
	std::wstring				mInitialShapeGeo;
	int							mLogLevel;
	boost::filesystem::path		mInfoFile;
	std::string					mLicHost;
	std::string					mLicFeature;
};


/**
 * Helper struct to manage PRT lifetime and licensing
 */
struct PRTContext {
	PRTContext(const InputArgs& inputArgs) {
		// -- create a console and file logger and register them with PRT
		boost::filesystem::path fsLogPath = inputArgs.mWorkDir / FILE_LOG;
		mLogHandler.reset(prt::ConsoleLogHandler::create(prt::LogHandler::ALL, prt::LogHandler::ALL_COUNT));
		mFileLogHandler.reset(prt::FileLogHandler::create(prt::LogHandler::ALL, prt::LogHandler::ALL_COUNT, fsLogPath.wstring().c_str()));
		prt::addLogHandler(mLogHandler.get());
		prt::addLogHandler(mFileLogHandler.get());

		// -- setup paths for plugins and licensing, assume standard sdk layout
		boost::filesystem::path rootPath = inputArgs.mWorkDir;
		boost::filesystem::path extPath = rootPath / "lib";
		boost::filesystem::path fsFlexLib = rootPath / "bin" / (getSharedLibraryPrefix() + FILE_FLEXNET_LIB + getSharedLibrarySuffix());
		std::string flexLib = fsFlexLib.string();

		// -- setup the licensing information
		prt::FlexLicParams flp;
		flp.mActLibPath = flexLib.c_str();
		flp.mFeature = inputArgs.mLicFeature.c_str();
		flp.mHostName = inputArgs.mLicHost.c_str();

		// -- initialize PRT with the path to its extension libraries, the desired log level and the licensing data
		std::wstring wExtPath = extPath.wstring();
		std::array<const wchar_t*, 1> extPaths = { wExtPath.c_str() };
		mLicHandle.reset(prt::init(extPaths.data(), extPaths.size(), (prt::LogLevel)inputArgs.mLogLevel, &flp));
	}

	~PRTContext() {
		// release PRT license
		mLicHandle.reset();

		// -- remove loggers
		prt::removeLogHandler(mLogHandler.get());
		prt::removeLogHandler(mFileLogHandler.get());
	}

	ConsoleLogHandlerPtr	mLogHandler;
	FileLogHandlerPtr		mFileLogHandler;
	ObjectPtr				mLicHandle;
};


/**
 * Logging Helpers
 */
namespace Loggers {

struct Logger { };

// log to std streams
const std::wstring LEVELS[] = { L"trace", L"debug", L"info", L"warning", L"error", L"fatal" };
template<prt::LogLevel L> struct StreamLogger : public Logger {
	StreamLogger(std::wostream& out = std::wcout) : Logger(), mOut(out) { mOut << prefix(); }
	virtual ~StreamLogger() { mOut << std::endl; }
	StreamLogger<L>& operator<<(std::wostream&(*x)(std::wostream&)) { mOut << x; return *this; }
	StreamLogger<L>& operator<<(const std::string& x) { std::copy(x.begin(), x.end(), std::ostream_iterator<char, wchar_t>(mOut)); return *this; }
	template<typename T> StreamLogger<L>& operator<<(const T& x) { mOut << x; return *this; }
	static std::wstring prefix() { return L"[" + LEVELS[L] + L"] "; }
	std::wostream& mOut;
};

// log through the prt logger
template<prt::LogLevel L> struct PRTLogger : public Logger {
	PRTLogger() : Logger() { }
	virtual ~PRTLogger() { prt::log(wstr.str().c_str(), L); }
	PRTLogger<L>& operator<<(std::wostream&(*x)(std::wostream&)) { wstr << x;  return *this; }
	PRTLogger<L>& operator<<(const std::string& x) {
		std::copy(x.begin(), x.end(), std::ostream_iterator<char, wchar_t>(wstr));
		return *this;
	}
	template<typename T> PRTLogger<L>& operator<<(const T& x) { wstr << x; return *this; }
	std::wostringstream wstr;
};

// choose your logger (PRTLogger or StreamLogger)
#define LT PRTLogger

typedef LT<prt::LOG_DEBUG>		_LOG_DBG;
typedef LT<prt::LOG_INFO>		_LOG_INF;
typedef LT<prt::LOG_WARNING>	_LOG_WRN;
typedef LT<prt::LOG_ERROR>		_LOG_ERR;

} // namespace Loggers

#define LOG_DBG Loggers::_LOG_DBG()
#define LOG_INF Loggers::_LOG_INF()
#define LOG_WRN Loggers::_LOG_WRN()
#define LOG_ERR Loggers::_LOG_ERR()


/**
 * more forward declarations of helper functions
 */
std::string  toOSNarrowFromUTF16(const std::wstring& osWString);
std::wstring toUTF16FromOSNarrow(const std::string& osString);
std::string  toUTF8FromOSNarrow (const std::string& osString);
std::wstring percentEncode      (const std::string& utf8String);
bool initInputArgs(int argc, char *argv[], InputArgs& inputArgs);
const prt::AttributeMap* createValidatedOptions(const wchar_t* encID, const prt::AttributeMap* unvalidatedOptions);
std::string objectToXML(prt::Object const* obj);
void codecInfoToXML(InputArgs& inputArgs);
std::wstring toFileURI(const boost::filesystem::path& p);


/**
 * finally, the actual model generation
 */
int main (int argc, char *argv[]) {
	try {
		// -- fetch command line args
		InputArgs inputArgs;
		if (!initInputArgs(argc, argv, inputArgs))
			return EXIT_FAILURE;

		// -- initialize PRT via a helper struct
		PRTContext prtCtx(inputArgs);
		if (!prtCtx.mLicHandle) {
			LOG_ERR << L"failed to get a CityEngine license, bailing out.";
			return EXIT_FAILURE;
		}

		// -- optionally handle the "codec info" command line switch and exit
		if (!inputArgs.mInfoFile.empty()) {
			codecInfoToXML(inputArgs);
			return EXIT_SUCCESS;
		}

		// -- setup output path for file callbacks
		if (!boost::filesystem::exists(inputArgs.mOutputPath)) {
			LOG_ERR << L"output path '" << inputArgs.mOutputPath << L"' does not exist, cannot continue.";
			return EXIT_FAILURE;
		}

		// -- create resolve map based on rule package
		ResolveMapPtr resolveMap;
		if (!inputArgs.mRulePackage.empty()) {
			if (inputArgs.mLogLevel <= prt::LOG_INFO) std::cout << "Using rule package " << inputArgs.mRulePackage << std::endl;

			std::wstring rpkURI = toFileURI(inputArgs.mRulePackage);
			prt::Status status = prt::STATUS_UNSPECIFIED_ERROR;
			resolveMap.reset(prt::createResolveMap(rpkURI.c_str(), nullptr, &status));
			if(status != prt::STATUS_OK) {
				LOG_ERR << "getting resolve map from '" << inputArgs.mRulePackage << "' failed, aborting.";
				return EXIT_FAILURE;
			}

			LOG_DBG << "resolve map = " << objectToXML(resolveMap.get());
		}

		// -- create cache & callback
		FileOutputCallbacksPtr foc{prt::FileOutputCallbacks::create(inputArgs.mOutputPath.wstring().c_str())};
		CachePtr cache{prt::CacheObject::create(prt::CacheObject::CACHE_TYPE_DEFAULT)};

		// -- setup initial shape geometry
		InitialShapeBuilderPtr isb{prt::InitialShapeBuilder::create()};
		if (!inputArgs.mInitialShapeGeo.empty()) {
			LOG_DBG << L"trying to read initial shape geometry from " << inputArgs.mInitialShapeGeo;
			isb->resolveGeometry(inputArgs.mInitialShapeGeo.c_str(), resolveMap.get(), cache.get());
		}
		else {
			isb->setGeometry(
					UnitQuad::vertices,
					UnitQuad::vertexCount,
					UnitQuad::indices,
					UnitQuad::indexCount,
					UnitQuad::faceCounts,
					UnitQuad::faceCountsCount
			);
		}

		// -- setup initial shape attributes
		std::wstring shapeName	= L"TheInitialShape";

		std::wstring ruleFile = L"bin/rule.cgb";
		if (inputArgs.mInitialShapeAttrs->hasKey(L"ruleFile") && inputArgs.mInitialShapeAttrs->getType(L"ruleFile") == prt::AttributeMap::PT_STRING)
			ruleFile = inputArgs.mInitialShapeAttrs->getString(L"ruleFile");

		std::wstring startRule = L"default$init";
		if (inputArgs.mInitialShapeAttrs->hasKey(L"startRule") && inputArgs.mInitialShapeAttrs->getType(L"startRule") == prt::AttributeMap::PT_STRING)
			startRule = inputArgs.mInitialShapeAttrs->getString(L"startRule");

		int32_t seed = 666;
		if (inputArgs.mInitialShapeAttrs->hasKey(L"seed") && inputArgs.mInitialShapeAttrs->getType(L"seed") == prt::AttributeMap::PT_INT)
			seed = inputArgs.mInitialShapeAttrs->getInt(L"seed");

		isb->setAttributes(
				ruleFile.c_str(),
				startRule.c_str(),
				seed,
				shapeName.c_str(),
				inputArgs.mInitialShapeAttrs,
				resolveMap.get()
		);

		// -- create initial shape
		InitialShapePtr initialShape{isb->createInitialShapeAndReset()};
		std::vector<const prt::InitialShape*> initialShapes = { initialShape.get() };

		// -- setup options for helper encoders
		AttributeMapBuilderPtr optionsBuilder{prt::AttributeMapBuilder::create()};
		optionsBuilder->setString(L"name", FILE_CGA_ERROR);
		AttributeMapPtr errOptions{optionsBuilder->createAttributeMapAndReset()};
		optionsBuilder->setString(L"name", FILE_CGA_PRINT);
		AttributeMapPtr printOptions{optionsBuilder->createAttributeMapAndReset()};

		// -- validate & complete encoder options
		AttributeMapPtr validatedEncOpts{createValidatedOptions(inputArgs.mEncoderID.c_str(), inputArgs.mEncoderOpts)};
		AttributeMapPtr validatedErrOpts{createValidatedOptions(ENCODER_ID_CGA_ERROR, errOptions.get())};
		AttributeMapPtr validatedPrintOpts{createValidatedOptions(ENCODER_ID_CGA_PRINT, printOptions.get())};

		// -- setup encoder IDs and corresponding options
		std::array<const wchar_t*,3> encoders = {
				inputArgs.mEncoderID.c_str(),	// our desired encoder
				ENCODER_ID_CGA_ERROR,			// an encoder to redirect rule errors into CGAErrors.txt
				ENCODER_ID_CGA_PRINT			// an encoder to redirect CGA print statements to CGAPrint.txt
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


/**
 * Helper to convert a list of "<key>:<type>=<value>" strings into an prt::AttributeMap
 */
const prt::AttributeMap* createAttributeMapFromTypedKeyValues(const std::vector<std::wstring>& args) {
	prt::AttributeMapBuilder* bld = prt::AttributeMapBuilder::create();
	for (const std::wstring& a: args) {
		std::vector<std::wstring> splits;
		boost::split(splits, a, boost::algorithm::is_any_of(":="), boost::token_compress_on);
		if (splits.size() == 3) {
			if (splits[1] == L"string") {
				bld->setString(splits[0].c_str(), splits[2].c_str());
			}
			else if (splits[1] == L"float") {
				double d;
				std::wistringstream istr(splits[2]);
				istr >> d;
				if (!istr.fail())
					bld->setFloat(splits[0].c_str(), d);
				else
					std::wcerr << L"cannot set float attribute " << splits[0] << std::endl;
			}
			else if (splits[1] == L"int") {
				int32_t v;
				std::wistringstream istr(splits[2]);
				istr >> v;
				if (!istr.fail())
					bld->setInt(splits[0].c_str(), v);
				else
					std::wcerr << L"cannot set int attribute " << splits[0] << std::endl;
			}
			else if (splits[1] == L"bool") {
				bool v;
				std::wistringstream istr(splits[2]);
				istr >> v;
				if (!istr.fail())
					bld->setBool(splits[0].c_str(), v);
				else
					std::wcerr << L"cannot set bool attribute " << splits[0] << std::endl;
			}
		}
		else
			std::wcout << L"warning: ignored key/value item: " << a << std::endl;
	}
	const prt::AttributeMap* am = bld->createAttributeMapAndReset();
	bld->destroy();
	return am;
}


/**
 * Parse the command line arguments and setup the inputArgs struct.
 */
bool initInputArgs(int argc, char *argv[], InputArgs& inputArgs) {
	// determine current path
	boost::filesystem::path executablePath = boost::filesystem::system_complete(boost::filesystem::path(argv[0]));
	inputArgs.mWorkDir = executablePath.parent_path().parent_path();

	std::vector<std::wstring> argShapeAttributes, argEncOpts;

	boost::program_options::options_description desc("Available Options");
	desc.add_options()("help,h", "This very help screen.")("version,v", "Show CityEngine SDK version.");
	desc.add_options()(
		"log-level,l",
		boost::program_options::value<int>(&inputArgs.mLogLevel)->default_value(2),
					   "Set log filter level: 1 = debug, 2 = info, 3 = warning, 4 = error, 5 = fatal, >5 = no output"
	);
	desc.add_options()(
		"output,o",
		boost::program_options::value<boost::filesystem::path>(&inputArgs.mOutputPath)->default_value(inputArgs.mWorkDir / "output"),
					   "Set the output path for the callbacks."
	);
	desc.add_options()(
		"encoder,e",
		boost::program_options::wvalue<std::wstring>(&inputArgs.mEncoderID)->default_value(ENCODER_ID_OBJ, toOSNarrowFromUTF16(ENCODER_ID_OBJ)), "The encoder ID, e.g. 'com.esri.prt.codecs.OBJEncoder'.");
	desc.add_options()(
		"rule-package,p",
		boost::program_options::value<std::string>(&inputArgs.mRulePackage),
					   "Set the rule package path."
	);
	desc.add_options()(
		"shape-attr,a",
		boost::program_options::wvalue<std::vector<std::wstring> >(&argShapeAttributes),
					   "Set a initial shape attribute (syntax is <name>:<type>=<value>, type = {string,float,int,bool})."
	);
	desc.add_options()(
		"shape-geo,g",
		boost::program_options::wvalue<std::wstring>(&inputArgs.mInitialShapeGeo),
					   "(Optional) Path to a file with shape geometry");
	desc.add_options()(
		"encoder-option,z",
		boost::program_options::wvalue<std::vector<std::wstring> >(&argEncOpts),
					   "Set a encoder option (syntax is <name>:<type>=<value>, type = {string,float,int,bool})."
	);
	desc.add_options()(
		"info,i",
		boost::program_options::value<boost::filesystem::path>(&inputArgs.mInfoFile), "Write XML Extension Information to file"
	);
	desc.add_options()(
		"license-server,s",
		boost::program_options::value<std::string>(&inputArgs.mLicHost),
					   "License Server Host Name, example: 27000@myserver.example.com"
	);
	desc.add_options()(
		"license-feature,f",
		boost::program_options::value<std::string>(&inputArgs.mLicFeature),
					   "License Feature to use, one of CityEngBasFx, CityEngBas, CityEngAdvFx, CityEngAdv"
	);
	
	boost::program_options::positional_options_description posDesc;
	posDesc.add("shape-ref", -1);
	
	boost::program_options::variables_map vm;
	boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(desc).positional(posDesc).run(), vm);
	boost::program_options::notify(vm);
	
	if (argc == 1 || vm.count("help")) {
		std::cout << desc << std::endl;
		return false;
	}
	
	if (vm.count("version")) {
		std::cout << prt::getVersion()->mFullName << std::endl;
		return false;
	}

	// make sure the path to the initial shape geometry is a valid URI
	if (!inputArgs.mInitialShapeGeo.empty()) {
		boost::filesystem::path isGeoPath = toOSNarrowFromUTF16(inputArgs.mInitialShapeGeo); // workaround for boost 1.41
		if (boost::filesystem::exists(isGeoPath)) {
			inputArgs.mInitialShapeGeo = toFileURI(isGeoPath);
		}
		else {
			std::wcerr << L"path to initial shape uri is not valid: " << inputArgs.mInitialShapeGeo << std::endl;
			return false;
		}
	}
	
	inputArgs.mInitialShapeAttrs = createAttributeMapFromTypedKeyValues(argShapeAttributes);
	inputArgs.mEncoderOpts = createAttributeMapFromTypedKeyValues(argEncOpts);
	
	return true;
}


std::string toOSNarrowFromUTF16(const std::wstring& osWString) {
	std::vector<char> temp(osWString.size());
	size_t size = temp.size();
	prt::Status status = prt::STATUS_OK;
	prt::StringUtils::toOSNarrowFromUTF16(osWString.c_str(), &temp[0], &size, &status);
	if(size > temp.size()) {
		temp.resize(size);
		prt::StringUtils::toOSNarrowFromUTF16(osWString.c_str(), &temp[0], &size, &status);
	}
	return std::string(&temp[0]);
}


std::wstring toUTF16FromOSNarrow(const std::string& osString) {
	std::vector<wchar_t> temp(osString.size());
	size_t size = temp.size();
	prt::Status status = prt::STATUS_OK;
	prt::StringUtils::toUTF16FromOSNarrow(osString.c_str(), &temp[0], &size, &status);
	if(size > temp.size()) {
		temp.resize(size);
		prt::StringUtils::toUTF16FromOSNarrow(osString.c_str(), &temp[0], &size, &status);
	}
	return std::wstring(&temp[0]);
}

std::string toUTF8FromOSNarrow(const std::string& osString) {
	std::wstring utf16String = toUTF16FromOSNarrow(osString);
	std::vector<char> temp(utf16String.size());
	size_t size = temp.size();
	prt::Status status = prt::STATUS_OK;
	prt::StringUtils::toUTF8FromUTF16(utf16String.c_str(), &temp[0], &size, &status);
	if(size > temp.size()) {
		temp.resize(size);
		prt::StringUtils::toUTF8FromUTF16(utf16String.c_str(), &temp[0], &size, &status);
	}
	return std::string(&temp[0]);
}

std::wstring percentEncode(const std::string& utf8String) {
	std::vector<char> temp(2*utf8String.size());
	size_t size = temp.size();
	prt::Status status = prt::STATUS_OK;
	prt::StringUtils::percentEncode(utf8String.c_str(), &temp[0], &size, &status);
	if(size > temp.size()) {
		temp.resize(size);
		prt::StringUtils::percentEncode(utf8String.c_str(), &temp[0], &size, &status);
	}

	std::vector<wchar_t> u16temp(temp.size());
	size = u16temp.size();
	prt::StringUtils::toUTF16FromUTF8(&temp[0], &u16temp[0], &size, &status);
	if(size > u16temp.size()) {
		u16temp.resize(size);
		prt::StringUtils::toUTF16FromUTF8(&temp[0], &u16temp[0], &size, &status);
	}

	return std::wstring(&u16temp[0]);
}


std::string objectToXML(prt::Object const* obj) {
	if (obj == 0)
		throw std::invalid_argument("object pointer is not valid");
	const size_t siz = 4096;
	size_t actualSiz = siz;
	std::string buffer(siz, ' ');
	obj->toXML((char*)&buffer[0], &actualSiz);
	buffer.resize(actualSiz-1); // ignore terminating 0
	if(siz < actualSiz)
		obj->toXML((char*)&buffer[0], &actualSiz);
	return buffer;
}


void codecInfoToXML(InputArgs& inputArgs) {
	std::vector<std::wstring> encIDs, decIDs;
	
	const size_t siz = 1024;
	size_t actualSiz = siz;
	std::vector<wchar_t> ids(siz, L' ');
	std::wstring idsStr;
	
	prt::listEncoderIds((wchar_t*)&ids[0], &actualSiz);
	ids.resize(actualSiz);
	if(siz < actualSiz)
		prt::listEncoderIds((wchar_t*)&ids[0], &actualSiz);
	idsStr = &ids[0];
	boost::trim_if(idsStr, boost::is_any_of("; "));
	boost::split(encIDs, idsStr, boost::is_any_of(L";"), boost::token_compress_on);
	
	prt::listDecoderIds((wchar_t*)&ids[0], &actualSiz);
	ids.resize(actualSiz);
	if(siz < actualSiz)
		prt::listDecoderIds((wchar_t*)&ids[0], &actualSiz);
	idsStr = &ids[0];
	boost::trim_if(idsStr, boost::is_any_of("; "));
	boost::split(decIDs, idsStr, boost::is_any_of(L";"), boost::token_compress_on);
	
	std::ofstream xml(inputArgs.mInfoFile.string().c_str());
	xml << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n\n";
	
	xml << "<Codecs build=\"" << prt::getVersion()->mVersion
	<< "\" buildDate=\"" << prt::getVersion()->mBuildDate
	<< "\" buildConfig=\"" << prt::getVersion()->mBuildConfig
	<< "\">\n";
	
	xml << "<Encoders>\n";
	for (const std::wstring& encID: encIDs) {
		prt::Status s = prt::STATUS_UNSPECIFIED_ERROR;
		const prt::EncoderInfo* encInfo = prt::createEncoderInfo(encID.c_str(), &s);
		if (s == prt::STATUS_OK && encInfo != 0)
			xml << objectToXML(encInfo) << std::endl;
		else
			std::wcout << L"encoder not found for ID: " << encID << std::endl;
		encInfo->destroy();
	}
	xml << "</Encoders>\n";
	
	xml << "<Decoders>\n";
	for (const std::wstring& decID: decIDs) {
		prt::Status s = prt::STATUS_UNSPECIFIED_ERROR;
		const prt::DecoderInfo* decInfo = prt::createDecoderInfo(decID.c_str(), &s);
		if (s == prt::STATUS_OK && decInfo != 0)
			xml << objectToXML(decInfo) << std::endl;
		else
			std::wcout << L"decoder not found for ID: " << decID << std::endl;
		decInfo->destroy();
	}
	xml << "</Decoders>\n";
	
	xml << "</Codecs>\n";
	xml.close();
}


std::wstring toFileURI(const boost::filesystem::path& p) {
#ifdef _WIN32
	static const std::wstring schema = L"file:/";
#else
	static const std::wstring schema = L"file:";
#endif

	std::string utf8Path = toUTF8FromOSNarrow(p.generic_string());
	std::wstring pecString = percentEncode(utf8Path);
	return schema + pecString;
}


const prt::AttributeMap* createValidatedOptions(const wchar_t* encID, const prt::AttributeMap* unvalidatedOptions) {
	const prt::EncoderInfo* encInfo = prt::createEncoderInfo(encID);
	const prt::AttributeMap* validatedOptions = 0;
	const prt::AttributeMap* optionStates = 0;
	encInfo->createValidatedOptionsAndStates(unvalidatedOptions, &validatedOptions, &optionStates);
	optionStates->destroy();
	encInfo->destroy();
	return validatedOptions;
}


std::string getSharedLibraryPrefix() {
	#if defined(_WIN32)
	return "";
	#elif defined(__APPLE__)
	return "lib";
	#elif defined(__linux__)
	return "lib";
	#else
	#	error unsupported build platform
	#endif
}


std::string getSharedLibrarySuffix() {
	#if defined(_WIN32)
	return ".dll";
	#elif defined(__APPLE__)
	return ".dylib";
	#elif defined(__linux__)
	return ".so";
	#else
	#	error unsupported build platform
	#endif
}

