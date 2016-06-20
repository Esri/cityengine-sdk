#pragma once

#include "prt/API.h"
#include "prt/FileOutputCallbacks.h"
#include "prt/LogHandler.h"

#include "boost/filesystem.hpp"

#include <memory>


namespace pcu { // prt4cmd utils

/**
 * helpers for prt object management
 */
struct PRTDestroyer {
	void operator()(prt::Object const* p) const {
		if (p)
			p->destroy();
	}
};

using ObjectPtr					= std::unique_ptr<const prt::Object, PRTDestroyer>;
using CachePtr					= std::unique_ptr<prt::CacheObject, PRTDestroyer>;
using ResolveMapPtr				= std::unique_ptr<const prt::ResolveMap, PRTDestroyer>;
using InitialShapePtr			= std::unique_ptr<const prt::InitialShape, PRTDestroyer>;
using InitialShapeBuilderPtr	= std::unique_ptr<prt::InitialShapeBuilder, PRTDestroyer>;
using AttributeMapPtr			= std::unique_ptr<const prt::AttributeMap, PRTDestroyer>;
using AttributeMapBuilderPtr	= std::unique_ptr<prt::AttributeMapBuilder, PRTDestroyer>;
using FileOutputCallbacksPtr	= std::unique_ptr<prt::FileOutputCallbacks, PRTDestroyer>;
using ConsoleLogHandlerPtr		= std::unique_ptr<prt::ConsoleLogHandler, PRTDestroyer>;
using FileLogHandlerPtr			= std::unique_ptr<prt::FileLogHandler, PRTDestroyer>;
using EncoderInfoPtr			= std::unique_ptr<const prt::EncoderInfo, PRTDestroyer>;
using DecoderInfoPtr			= std::unique_ptr<const prt::DecoderInfo, PRTDestroyer>;

/**
 * options helpers
 */
AttributeMapPtr createValidatedOptions(const wchar_t* encID, const AttributeMapPtr& unvalidatedOptions);

/**
 * command line argument helpers
 */
struct InputArgs {
	InputArgs(const std::wstring& defaultEncoderID) : mEncoderID(defaultEncoderID) { }

	boost::filesystem::path	mWorkDir;
	std::wstring			mEncoderID;
	AttributeMapPtr			mEncoderOpts;
	boost::filesystem::path	mOutputPath;
	std::string				mRulePackage;
	AttributeMapPtr			mInitialShapeAttrs;
	std::wstring			mInitialShapeGeo;
	int						mLogLevel;
	boost::filesystem::path	mInfoFile;
	std::string				mLicHost;
	std::string				mLicFeature;
};

bool initInputArgs(int argc, char *argv[], InputArgs& inputArgs);

/**
 * library helpers
 */
std::string getSharedLibraryPrefix();
std::string getSharedLibrarySuffix();

/**
 * string and URI helpers
 */
std::string  toOSNarrowFromUTF16(const std::wstring& osWString);
std::wstring toUTF16FromOSNarrow(const std::string& osString);
std::string  toUTF8FromOSNarrow (const std::string& osString);
std::wstring percentEncode      (const std::string& utf8String);
std::wstring toFileURI(const boost::filesystem::path& p);

/**
 * XML helpers
 */
std::string objectToXML(prt::Object const* obj);
void codecInfoToXML(InputArgs& inputArgs);

/**
 * default initial shape geometry (a quad)
 */
namespace quad {
const double 	vertices[]		= { 0, 0, 0,  0, 0, 1,  1, 0, 1,  1, 0, 0 };
const size_t 	vertexCount		= 12;
const uint32_t	indices[]		= { 0, 1, 2, 3 };
const size_t 	indexCount		= 4;
const uint32_t 	faceCounts[]	= { 4 };
const size_t 	faceCountsCount	= 1;
}

} // namespace utils
