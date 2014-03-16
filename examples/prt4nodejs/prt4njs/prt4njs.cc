#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iterator>

#include "boost/filesystem.hpp"
#include "boost/foreach.hpp"
#include "boost/algorithm/string.hpp"
#include "boost/assign.hpp"
#include "boost/locale/encoding.hpp"
#include "boost/shared_ptr.hpp"

#include "prt/prt.h"
#include "prt/API.h"
#include "prt/Cache.h"
#include "prt/MemoryOutputCallbacks.h"
#include "prt/FlexLicParams.h"

#include "node.h"
#include "v8.h"


namespace {

// some file name definitions
const char*		FILE_FLEXNET_LIB		= "flexnet_prt";
const char* 	FILE_LOG				= "prt4njs.log";
const wchar_t*	FILE_CGA_ERROR			= L"CGAErrors.txt";
const wchar_t*	FILE_CGA_PRINT			= L"CGAPrint.txt";


// some encoder IDs
const wchar_t*	ENCODER_ID_CGA_ERROR	= L"com.esri.prt.core.CGAErrorEncoder";
const wchar_t*	ENCODER_ID_CGA_PRINT	= L"com.esri.prt.core.CGAPrintEncoder";
const wchar_t*	ENCODER_ID_OBJ			= L"com.esri.prt.codecs.OBJEncoder";


// default initial shape geometry (a quad)
namespace UnitQuad {
const double 	vertices[]				= { 0, 0, 0,  0, 0, 1,  1, 0, 1,  1, 0, 0 };
const size_t 	vertexCount				= 12;
const uint32_t	indices[]				= { 0, 1, 2, 3 };
const size_t 	indexCount				= 4;
const uint32_t 	faceCounts[]			= { 4 };
const size_t 	faceCountsCount			= 1;
}


namespace Loggers {

const std::wstring LEVELS[] = { L"trace", L"debug", L"info", L"warning", L"error", L"fatal" };

struct Logger {
	Logger(prt::LogLevel lvl) : mLogLevel(lvl) { }
	std::wstring prefix() const { return L"[" + LEVELS[mLogLevel] + L"] "; }
	prt::LogLevel mLogLevel;
};

// log to std streams
struct StreamLogger : public Logger {
	StreamLogger(std::wostream& out, prt::LogLevel lvl) : Logger(lvl), mOut(out) { }
	const StreamLogger& operator<<(std::wostream&(*x)(std::wostream&)) const { mOut << prefix() << x << std::endl; return *this; }
	template<typename T> const StreamLogger& operator<<(const T& x) const { mOut << prefix() << x << std::endl; return *this; }
	std::wostream& mOut;
};
template<> const StreamLogger& StreamLogger::operator<< <> (const std::string& x) const { mOut << prefix(); std::copy(x.begin(), x.end(), std::ostream_iterator<char, wchar_t>(mOut)); return *this; }

// log through the prt logger
struct PRTLogger : public Logger {
	PRTLogger(std::wostream&, prt::LogLevel lvl) : Logger(lvl) { }
	const PRTLogger& operator<<(std::wostream&(*x)(std::wostream&)) const {
		std::wostringstream wstr; wstr << x; prt::log(wstr.str().c_str(), mLogLevel); return *this;
	}
	template<typename T> const PRTLogger& operator<<(const T& x) const {
		std::wostringstream wstr; wstr << x; prt::log(wstr.str().c_str(), mLogLevel); return *this;
	}
};
template<> const PRTLogger& PRTLogger::operator<< <> (const std::string& x) const {
	std::wostringstream wstr;
	std::copy(x.begin(), x.end(), std::ostream_iterator<char, wchar_t>(wstr));
	prt::log(wstr.str().c_str(), mLogLevel);
	return *this;
}

} // namespace Loggers

// choose your logger
//typedef Loggers::StreamLogger LoggerType;
typedef Loggers::PRTLogger LoggerType;
const LoggerType LOG_DBG(std::wcout, prt::LOG_DEBUG);
const LoggerType LOG_INF(std::wcout, prt::LOG_INFO);
const LoggerType LOG_WRN(std::wcout, prt::LOG_WARNING);
const LoggerType LOG_ERR(std::wcout, prt::LOG_ERROR);


struct PRTContext {
	PRTContext(
			const std::string& prtRoot,
			const std::string& licType,
			const std::string& licHost,
			prt::LogLevel logLvl
	)
	: mPRTRoot(prtRoot)
	, mLogHandler(0)
	, mFileLogHandler(0)
	, mLicHandle(0)
	, mLicType(licType)
	, mLicHost(licHost)
	, mLogLevel(logLvl)
	, mCache(0)
	{
		init();
	}
	~PRTContext() {
		cleanup();
	}

	void init() {
		mLogHandler = prt::ConsoleLogHandler::create(prt::LogHandler::ALL, (size_t)6);
		prt::addLogHandler(mLogHandler);

		//		boost::filesystem::path fileLogPath = boost::filesystem::temp_directory_path() / FILE_LOG;
		//		mFileLogHandler = prt::FileLogHandler::create(prt::LogHandler::ALL, (size_t)6, fileLogPath.wstring().c_str());
		//		prt::addLogHandler(mFileLogHandler);

		if (mLogLevel <= prt::LOG_DEBUG) {
			LOG_DBG << "prt root: " << mPRTRoot;
			LOG_DBG << "lic type: " << mLicType;
			LOG_DBG << "lic host: " << mLicHost;
		}

		std::string prtLicLib =  mPRTRoot + "/bin/libflexnet_prt.dylib"; // TODO: FILE_FLEXNET_LIB

		prt::FlexLicParams flp;
		flp.mActLibPath	= prtLicLib.c_str();
		flp.mFeature	= mLicType.c_str();
		flp.mHostName	= mLicHost.c_str();

		std::wstring wRoot = boost::locale::conv::utf_to_utf<wchar_t>(mPRTRoot);
		std::wstring extPath = wRoot + L"/lib";

		prt::Status status = prt::STATUS_UNSPECIFIED_ERROR;
		const wchar_t* cExtPath = extPath.c_str();
		mLicHandle = prt::init(&cExtPath, 1, mLogLevel, &flp, &status);

		if (!good()) {
			LOG_ERR << "failed to initialize prt: " << prt::getStatusDescription(status);
		}

		mCache = prt::CacheObject::create(prt::CacheObject::CACHE_TYPE_DEFAULT);
	}

	void cleanup() {
		if (mCache)
			mCache->destroy();

		if (mLicHandle)
			mLicHandle->destroy();

		if (mLogHandler) {
			prt::removeLogHandler(mLogHandler);
			mLogHandler->destroy();
		}
		if (mFileLogHandler) {
			prt::removeLogHandler(mFileLogHandler);
			mFileLogHandler->destroy();
		}

		if (mLogLevel <= prt::LOG_DEBUG) {
			LOG_DBG << "destroyed prt context";
		}
	}

	bool good() const { return (mLicHandle != 0); }

	std::string				mPRTRoot;

	prt::ConsoleLogHandler*	mLogHandler;
	prt::FileLogHandler*	mFileLogHandler;

	prt::Object const * 	mLicHandle;
	std::string				mLicType;
	std::string				mLicHost;

	prt::LogLevel			mLogLevel;

	prt::CacheObject*		mCache;
};

PRTContext* prtCtx = 0;


const char* AnnotationTypeNames[6] = { "void", "bool", "float", "string", "int", "unknown" };

v8::Local<v8::String> toNJS(prt::AnnotationArgumentType aat) {
	v8::Local<v8::String> njsAAT = v8::String::New(AnnotationTypeNames[aat]);
//	njsAAT->Set(v8::String::NewSymbol(), v8::Int32::New(static_cast<int32_t>(aat)), v8::ReadOnly);
	return njsAAT;
}


} // namespace anonymous


class NJSAnnotationArgument : public node::ObjectWrap {
public:
	static v8::Handle<v8::Value> create(prt::AnnotationArgument const* prtPtr) {
		v8::Local<v8::ObjectTemplate> tpl = v8::ObjectTemplate::New();
		tpl->SetInternalFieldCount(1);

		tpl->Set(v8::String::NewSymbol("getType"),	v8::FunctionTemplate::New(getType)->GetFunction());
		tpl->Set(v8::String::NewSymbol("getKey"),	v8::FunctionTemplate::New(getKey)->GetFunction());
		tpl->Set(v8::String::NewSymbol("getBool"),	v8::FunctionTemplate::New(getBool)->GetFunction());
		tpl->Set(v8::String::NewSymbol("getFloat"),	v8::FunctionTemplate::New(getFloat)->GetFunction());
		tpl->Set(v8::String::NewSymbol("getStr"),	v8::FunctionTemplate::New(getStr)->GetFunction());

		NJSAnnotationArgument* njsObj = new NJSAnnotationArgument(prtPtr);
		v8::Local<v8::Object> njsInstance = tpl->NewInstance();
		njsObj->Wrap(njsInstance);
		return njsInstance;
	}

	static v8::Handle<v8::Value> getType(const v8::Arguments& args) {
		v8::HandleScope scope;
		NJSAnnotationArgument* obj = node::ObjectWrap::Unwrap<NJSAnnotationArgument>(args.This());
		prt::AnnotationArgumentType aat = obj->mPtr->getType();
		return scope.Close(toNJS(aat));
	}

	static v8::Handle<v8::Value> getKey(const v8::Arguments& args) {
		v8::HandleScope scope;
		NJSAnnotationArgument* thisPtr = node::ObjectWrap::Unwrap<NJSAnnotationArgument>(args.This());
		const wchar_t* wKey = thisPtr->mPtr->getKey();
		const std::string key = boost::locale::conv::utf_to_utf<char>(wKey);
		return scope.Close(v8::String::New(key.c_str()));
	}

	static v8::Handle<v8::Value> getBool(const v8::Arguments& args) {
		v8::HandleScope scope;
		NJSAnnotationArgument* thisPtr = node::ObjectWrap::Unwrap<NJSAnnotationArgument>(args.This());
		return scope.Close(v8::Boolean::New(thisPtr->mPtr->getBool()));
	}

	static v8::Handle<v8::Value> getFloat(const v8::Arguments& args) {
		v8::HandleScope scope;
		NJSAnnotationArgument* thisPtr = node::ObjectWrap::Unwrap<NJSAnnotationArgument>(args.This());
		return scope.Close(v8::Number::New(thisPtr->mPtr->getFloat()));
	}

	static v8::Handle<v8::Value> getStr(const v8::Arguments& args) {
		v8::HandleScope scope;
		NJSAnnotationArgument* thisPtr = node::ObjectWrap::Unwrap<NJSAnnotationArgument>(args.This());
		const wchar_t* wval = thisPtr->mPtr->getStr();
		const std::string val = boost::locale::conv::utf_to_utf<char>(wval);
		return scope.Close(v8::String::New(val.c_str()));
	}

private:
	explicit NJSAnnotationArgument(prt::AnnotationArgument const* ptr) : mPtr(ptr) { }
	virtual ~NJSAnnotationArgument() { }

	prt::AnnotationArgument const* mPtr;
};


class NJSAnnotation : public node::ObjectWrap {
public:
	static v8::Handle<v8::Value> create(prt::Annotation const* prtPtr) {
		v8::Local<v8::ObjectTemplate> tpl = v8::ObjectTemplate::New();
		tpl->SetInternalFieldCount(1);

		tpl->Set(v8::String::NewSymbol("getName"),			v8::FunctionTemplate::New(getName)->GetFunction());
		tpl->Set(v8::String::NewSymbol("getNumArguments"),	v8::FunctionTemplate::New(getNumArguments)->GetFunction());
		tpl->Set(v8::String::NewSymbol("getArgument"),		v8::FunctionTemplate::New(getArgument)->GetFunction());

		NJSAnnotation* njsObj = new NJSAnnotation(prtPtr);
		v8::Local<v8::Object> njsInstance = tpl->NewInstance();
		njsObj->Wrap(njsInstance);
		return njsInstance;
	}

	static v8::Handle<v8::Value> getName(const v8::Arguments& args) {
		v8::HandleScope scope;
		NJSAnnotation* thisPtr = node::ObjectWrap::Unwrap<NJSAnnotation>(args.This());

		const wchar_t* wName = thisPtr->mPtr->getName();
		const std::string name = boost::locale::conv::utf_to_utf<char>(wName);

		return scope.Close(v8::String::New(name.c_str()));
	}

	static v8::Handle<v8::Value> getNumArguments(const v8::Arguments& args) {
		v8::HandleScope scope;
		NJSAnnotation* thisPtr = node::ObjectWrap::Unwrap<NJSAnnotation>(args.This());
		return scope.Close(v8::Integer::New(thisPtr->mPtr->getNumArguments()));
	}

	static v8::Handle<v8::Value> getArgument(const v8::Arguments& args) {
		v8::HandleScope scope;
		NJSAnnotation* thisPtr = node::ObjectWrap::Unwrap<NJSAnnotation>(args.This());

		uint32_t idx = args[0]->ToUint32()->Value();
		prt::AnnotationArgument const* annArg = thisPtr->mPtr->getArgument(idx);

		return scope.Close(NJSAnnotationArgument::create(annArg));
	}

private:
	explicit NJSAnnotation(prt::Annotation const* ptr) : mPtr(ptr) { }
	virtual ~NJSAnnotation() { }

	prt::Annotation const* mPtr;
};


class NJSRuleFileInfoParameter : public node::ObjectWrap {
public:
	static v8::Handle<v8::Value> create(prt::RuleFileInfo::Parameter const* prtPtr) {
		v8::Local<v8::ObjectTemplate> tpl = v8::ObjectTemplate::New();
		tpl->SetInternalFieldCount(1);

		tpl->Set(v8::String::NewSymbol("getType"),			v8::FunctionTemplate::New(getType)->GetFunction());
		tpl->Set(v8::String::NewSymbol("getName"),			v8::FunctionTemplate::New(getName)->GetFunction());
		tpl->Set(v8::String::NewSymbol("getNumAnnotations"),v8::FunctionTemplate::New(getNumAnnotations)->GetFunction());
		tpl->Set(v8::String::NewSymbol("getAnnotation"),	v8::FunctionTemplate::New(getAnnotation)->GetFunction());

		NJSRuleFileInfoParameter* njsObj = new NJSRuleFileInfoParameter(prtPtr);
		v8::Local<v8::Object> njsInstance = tpl->NewInstance();
		njsObj->Wrap(njsInstance);
		return njsInstance;
	}

	static v8::Handle<v8::Value> getType(const v8::Arguments& args) {
		v8::HandleScope scope;
		NJSRuleFileInfoParameter* obj = node::ObjectWrap::Unwrap<NJSRuleFileInfoParameter>(args.This());
		prt::AnnotationArgumentType aat = obj->mPtr->getType();
		return scope.Close(toNJS(aat));
	}

	static v8::Handle<v8::Value> getName(const v8::Arguments& args) {
		v8::HandleScope scope;
		NJSRuleFileInfoParameter* obj = node::ObjectWrap::Unwrap<NJSRuleFileInfoParameter>(args.This());

		const wchar_t* wName = obj->mPtr->getName();
		const std::string name = boost::locale::conv::utf_to_utf<char>(wName);

		return scope.Close(v8::String::New(name.c_str()));
	}

	static v8::Handle<v8::Value> getNumAnnotations(const v8::Arguments& args) {
		v8::HandleScope scope;
		NJSRuleFileInfoParameter* obj = node::ObjectWrap::Unwrap<NJSRuleFileInfoParameter>(args.This());
		return scope.Close(v8::Integer::New(obj->mPtr->getNumAnnotations()));
	}

	static v8::Handle<v8::Value> getAnnotation(const v8::Arguments& args) {
		v8::HandleScope scope;
		NJSRuleFileInfoParameter* thisPtr = node::ObjectWrap::Unwrap<NJSRuleFileInfoParameter>(args.This());

		uint32_t idx = args[0]->ToUint32()->Value();
		prt::Annotation const* ann = thisPtr->mPtr->getAnnotation(idx);

		return scope.Close(NJSAnnotation::create(ann));
	}

private:
	explicit NJSRuleFileInfoParameter(prt::RuleFileInfo::Parameter const* entry) : mPtr(entry) { }
	virtual ~NJSRuleFileInfoParameter() { }

	prt::RuleFileInfo::Parameter const* mPtr;
};


class NJSRuleFileInfoEntry : public node::ObjectWrap {
public:
	static v8::Handle<v8::Value> create(prt::RuleFileInfo::Entry const* entry) {
		v8::Local<v8::ObjectTemplate> tpl = v8::ObjectTemplate::New();
		tpl->SetInternalFieldCount(1);

		tpl->Set(v8::String::NewSymbol("getReturnType"),	v8::FunctionTemplate::New(getReturnType)->GetFunction());
		tpl->Set(v8::String::NewSymbol("getName"),			v8::FunctionTemplate::New(getName)->GetFunction());
		tpl->Set(v8::String::NewSymbol("getNumParameters"),	v8::FunctionTemplate::New(getNumParameters)->GetFunction());
		tpl->Set(v8::String::NewSymbol("getParameter"),		v8::FunctionTemplate::New(getParameter)->GetFunction());
		tpl->Set(v8::String::NewSymbol("getNumAnnotations"),v8::FunctionTemplate::New(getNumAnnotations)->GetFunction());
		tpl->Set(v8::String::NewSymbol("getAnnotation"),	v8::FunctionTemplate::New(getAnnotation)->GetFunction());

		NJSRuleFileInfoEntry* njsEntry = new NJSRuleFileInfoEntry(entry);
		v8::Local<v8::Object> njsInstance = tpl->NewInstance();
		njsEntry->Wrap(njsInstance);
		return njsInstance;
	}

	static v8::Handle<v8::Value> getReturnType(const v8::Arguments& args) {
		v8::HandleScope scope;
		NJSRuleFileInfoEntry* obj = node::ObjectWrap::Unwrap<NJSRuleFileInfoEntry>(args.This());

		prt::AnnotationArgumentType aat = obj->mPtr->getReturnType();

		return scope.Close(toNJS(aat));
	}

	static v8::Handle<v8::Value> getName(const v8::Arguments& args) {
		v8::HandleScope scope;
		NJSRuleFileInfoEntry* obj = node::ObjectWrap::Unwrap<NJSRuleFileInfoEntry>(args.This());

		const wchar_t* wName = obj->mPtr->getName();
		const std::string name = boost::locale::conv::utf_to_utf<char>(wName);

		return scope.Close(v8::String::New(name.c_str()));
	}

	static v8::Handle<v8::Value> getNumParameters(const v8::Arguments& args) {
		v8::HandleScope scope;
		NJSRuleFileInfoEntry* obj = node::ObjectWrap::Unwrap<NJSRuleFileInfoEntry>(args.This());
		return scope.Close(v8::Integer::New(obj->mPtr->getNumParameters()));
	}

	static v8::Handle<v8::Value> getParameter(const v8::Arguments& args) {
		v8::HandleScope scope;
		NJSRuleFileInfoEntry* obj = node::ObjectWrap::Unwrap<NJSRuleFileInfoEntry>(args.This());

		uint32_t idx = args[0]->ToUint32()->Value();
		prt::RuleFileInfo::Parameter const* param = obj->mPtr->getParameter(idx);

		return scope.Close(NJSRuleFileInfoParameter::create(param));
	}

	static v8::Handle<v8::Value> getNumAnnotations(const v8::Arguments& args) {
		v8::HandleScope scope;
		NJSRuleFileInfoEntry* obj = node::ObjectWrap::Unwrap<NJSRuleFileInfoEntry>(args.This());
		return scope.Close(v8::Integer::New(obj->mPtr->getNumAnnotations()));
	}

	static v8::Handle<v8::Value> getAnnotation(const v8::Arguments& args) {
		v8::HandleScope scope;
		NJSRuleFileInfoEntry* thisPtr = node::ObjectWrap::Unwrap<NJSRuleFileInfoEntry>(args.This());

		uint32_t idx = args[0]->ToUint32()->Value();
		prt::Annotation const* ann = thisPtr->mPtr->getAnnotation(idx);

		return scope.Close(NJSAnnotation::create(ann));
	}

private:
	explicit NJSRuleFileInfoEntry(prt::RuleFileInfo::Entry const* entry) : mPtr(entry) { }
	virtual ~NJSRuleFileInfoEntry() { }

	prt::RuleFileInfo::Entry const* mPtr;
};


class NJSRuleFileInfo : public node::ObjectWrap {
public:
	static v8::Handle<v8::Value> create(prt::RuleFileInfo const* info) {
		v8::Local<v8::ObjectTemplate> tpl = v8::ObjectTemplate::New();
		tpl->SetInternalFieldCount(1);

		tpl->Set(v8::String::NewSymbol("getNumAttributes"), v8::FunctionTemplate::New(getNumAttributes)->GetFunction());
		tpl->Set(v8::String::NewSymbol("getAttribute"), v8::FunctionTemplate::New(getAttribute)->GetFunction());
		tpl->Set(v8::String::NewSymbol("getNumRules"), v8::FunctionTemplate::New(getNumRules)->GetFunction());
		tpl->Set(v8::String::NewSymbol("getRule"), v8::FunctionTemplate::New(getRule)->GetFunction());
		tpl->Set(v8::String::NewSymbol("getNumAnnotations"), v8::FunctionTemplate::New(getNumAnnotations)->GetFunction());

		NJSRuleFileInfo* njsInfo = new NJSRuleFileInfo(info);

		v8::Local<v8::Object> njsInfoObj = tpl->NewInstance();
		njsInfo->Wrap(njsInfoObj);
		return njsInfoObj;
	}

	static v8::Handle<v8::Value> getNumAttributes(const v8::Arguments& args) {
		v8::HandleScope scope;
		NJSRuleFileInfo* obj = node::ObjectWrap::Unwrap<NJSRuleFileInfo>(args.This());
		return scope.Close(v8::Integer::New(obj->mPRTObject->getNumAttributes()));
	}

	static v8::Handle<v8::Value> getAttribute(const v8::Arguments& args) {
		v8::HandleScope scope;
		NJSRuleFileInfo* obj = node::ObjectWrap::Unwrap<NJSRuleFileInfo>(args.This());
		uint32_t ruleIdx = args[0]->ToUint32()->Value();
		prt::RuleFileInfo::Entry const* attr = obj->mPRTObject->getAttribute(ruleIdx);
		return scope.Close(NJSRuleFileInfoEntry::create(attr));
	}

	static v8::Handle<v8::Value> getNumRules(const v8::Arguments& args) {
		v8::HandleScope scope;
		NJSRuleFileInfo* obj = node::ObjectWrap::Unwrap<NJSRuleFileInfo>(args.This());
		return scope.Close(v8::Integer::New(obj->mPRTObject->getNumRules()));
	}

	static v8::Handle<v8::Value> getRule(const v8::Arguments& args) {
		v8::HandleScope scope;
		NJSRuleFileInfo* obj = node::ObjectWrap::Unwrap<NJSRuleFileInfo>(args.This());
		uint32_t ruleIdx = args[0]->ToUint32()->Value();
		prt::RuleFileInfo::Entry const* entry = obj->mPRTObject->getRule(ruleIdx);
		return scope.Close(NJSRuleFileInfoEntry::create(entry));
	}

	static v8::Handle<v8::Value> getNumAnnotations(const v8::Arguments& args) {
		v8::HandleScope scope;
		NJSRuleFileInfo* obj = node::ObjectWrap::Unwrap<NJSRuleFileInfo>(args.This());
		return scope.Close(v8::Integer::New(obj->mPRTObject->getNumAnnotations()));
	}

private:
	explicit NJSRuleFileInfo(prt::RuleFileInfo const* info) : mPRTObject(info) { }
	virtual ~NJSRuleFileInfo() { mPRTObject->destroy(); }

	prt::RuleFileInfo const* mPRTObject;
};


v8::Handle<v8::Value> njsInit(const v8::Arguments& args) {
	v8::String::Utf8Value	prtRoot(args[0]->ToString());
	v8::String::Utf8Value	licType(args[1]->ToString());
	v8::String::Utf8Value	licHost(args[2]->ToString());
	uint32_t				logLvl =args[3]->ToUint32()->Value();
	v8::HandleScope scope;

	prtCtx = new PRTContext(*prtRoot, *licType, *licHost, static_cast<prt::LogLevel>(logLvl));

	v8::Local<v8::Integer> initResult = v8::Integer::New(-1);
	if (prtCtx->good())
		initResult = v8::Integer::New(0);
	return scope.Close(initResult);
}


v8::Handle<v8::Value> njsCleanup(const v8::Arguments& args) {
	v8::HandleScope scope;

	delete prtCtx;
	prtCtx = 0;

	LOG_DBG << "cleaned up";
	return scope.Close(v8::Undefined());
}


v8::Handle<v8::Value> njsRuleInfo(const v8::Arguments& args) {
	v8::String::Utf8Value rpkURI(args[0]->ToString());
	v8::HandleScope scope;

	LOG_DBG << "ruleInfo for " << *rpkURI;
	prt::Status status = prt::STATUS_UNSPECIFIED_ERROR;
	const std::wstring wRPKUri = boost::locale::conv::utf_to_utf<wchar_t>(*rpkURI);
	prt::RuleFileInfo const* info = prt::createRuleFileInfo(wRPKUri.c_str(), prtCtx->mCache, &status);

	return scope.Close(NJSRuleFileInfo::create(info));
}


v8::Handle<v8::Value> njsGenerate(const v8::Arguments& args) {
	v8::String::Utf8Value rpkURI(args[0]->ToString());
	v8::HandleScope scope;

//	const prt::ResolveMap* assetsMap = 0;
//	if (!inputArgs.mRulePackage.empty()) {
//		if (inputArgs.mLogLevel <= prt::LOG_INFO) std::cout << "Using rule package " << inputArgs.mRulePackage.string() << std::endl;
//
//		std::wstring rpkURI = toFileURI<wchar_t>(/*inputArgs.mWorkDir / */ inputArgs.mRulePackage); // legacy workaround for old boost
//		prt::Status status = prt::STATUS_UNSPECIFIED_ERROR;
//		assetsMap = prt::createResolveMap(rpkURI.c_str(), 0, &status);
//		if(status != prt::STATUS_OK) {
//			LOG_ERR << "getting resolve map from '" << inputArgs.mRulePackage.string() << "' failed, aborting.";
//			runCtx.cleanup();
//			return 1;
//		}
//
//		LOG_DBG << "resolve map = " << objectToXML(assetsMap);
//	}
//
//	prt::MemoryOutputCallbacks* moh = prt::MemoryOutputCallbacks::create();
//	{
//
//		// -- setup initial shape
//		prt::InitialShapeBuilder* isb = prt::InitialShapeBuilder::create();
//		if (!inputArgs.mInitialShapeGeo.empty()) {
//			LOG_DBG << L"trying to read initial shape geometry from " << inputArgs.mInitialShapeGeo;
//			isb->resolveGeometry(inputArgs.mInitialShapeGeo.c_str(), assetsMap, cache);
//		}
//		else {
//			isb->setGeometry(
//					UnitQuad::vertices,
//					UnitQuad::vertexCount,
//					UnitQuad::indices,
//					UnitQuad::indexCount,
//					UnitQuad::faceCounts,
//					UnitQuad::faceCountsCount,
//					0,
//					0
//			);
//		}
//
//		// -- setup initial shape attributes
//		std::wstring shapeName	= L"TheInitialShape";
//
//		std::wstring ruleFile = L"bin/rule.cgb";
//		if (inputArgs.mInitialShapeAttrs->hasKey(L"ruleFile") && inputArgs.mInitialShapeAttrs->getType(L"ruleFile") == prt::AttributeMap::PT_STRING)
//			ruleFile = inputArgs.mInitialShapeAttrs->getString(L"ruleFile");
//
//		std::wstring startRule = L"default$init";
//		if (inputArgs.mInitialShapeAttrs->hasKey(L"startRule") && inputArgs.mInitialShapeAttrs->getType(L"startRule") == prt::AttributeMap::PT_STRING)
//			startRule = inputArgs.mInitialShapeAttrs->getString(L"startRule");
//
//		int32_t seed = isb->computeSeed();
//		if (inputArgs.mInitialShapeAttrs->hasKey(L"seed") && inputArgs.mInitialShapeAttrs->getType(L"seed") == prt::AttributeMap::PT_INT)
//			seed = inputArgs.mInitialShapeAttrs->getInt(L"seed");
//
//		isb->setAttributes(
//				ruleFile.c_str(),
//				startRule.c_str(),
//				seed,
//				shapeName.c_str(),
//				inputArgs.mInitialShapeAttrs,
//				assetsMap
//		);
//
//		// -- create initial shape
//		std::vector<const prt::InitialShape*> initialShapes = boost::assign::list_of(isb->createInitialShapeAndReset());
//		isb->destroy();
//
//		// -- setup options for helper encoders
//		prt::AttributeMapBuilder* optionsBuilder = prt::AttributeMapBuilder::create();
//		optionsBuilder->setString(L"name", FILE_CGA_ERROR);
//		const prt::AttributeMap* errOptions = optionsBuilder->createAttributeMapAndReset();
//		optionsBuilder->setString(L"name", FILE_CGA_PRINT);
//		const prt::AttributeMap* printOptions = optionsBuilder->createAttributeMapAndReset();
//		optionsBuilder->destroy();
//
//		// -- validate & complete encoder options
//		const prt::AttributeMap* validatedEncOpts = createValidatedOptions(inputArgs.mEncoderID.c_str(), inputArgs.mEncoderOpts);
//		const prt::AttributeMap* validatedErrOpts = createValidatedOptions(ENCODER_ID_CGA_ERROR, errOptions);
//		const prt::AttributeMap* validatedPrintOpts = createValidatedOptions(ENCODER_ID_CGA_PRINT, printOptions);
//
//		// -- THE GENERATE CALL
//		const prt::AttributeMap* encoderOpts[] = { validatedEncOpts, validatedErrOpts, validatedPrintOpts };
//		const wchar_t* encoders[] = {
//				inputArgs.mEncoderID.c_str(),	// our desired encoder
//				ENCODER_ID_CGA_ERROR,			// an encoder to redirect rule errors into CGAErrors.txt
//				ENCODER_ID_CGA_PRINT			// an encoder to redirect CGA print statements to CGAPrint.txt
//		};
//		prt::Status stat = prt::generate(&initialShapes[0], initialShapes.size(), 0, encoders, 3, encoderOpts, foh, cache, 0);
//		if(stat != prt::STATUS_OK) {
//			LOG_ERR << "prt::generate() failed with status: '" << prt::getStatusDescription(stat) << "' (" << stat << ")";
//		}
//
//		// -- cleanup
//		errOptions->destroy();
//		printOptions->destroy();
//		validatedEncOpts->destroy();
//		validatedErrOpts->destroy();
//		validatedPrintOpts->destroy();
//		for(size_t i=0; i<initialShapes.size(); i++)
//			initialShapes[i]->destroy();
//		if(assetsMap) assetsMap->destroy();
//
//	}
//	moh->destroy();

	return scope.Close(v8::Undefined());
}


void init(v8::Handle<v8::Object> exports) {
	exports->Set(v8::String::NewSymbol("init"), 	v8::FunctionTemplate::New(njsInit)->GetFunction());
	exports->Set(v8::String::NewSymbol("cleanup"),	v8::FunctionTemplate::New(njsCleanup)->GetFunction());
	exports->Set(v8::String::NewSymbol("ruleInfo"),	v8::FunctionTemplate::New(njsRuleInfo)->GetFunction());
	exports->Set(v8::String::NewSymbol("generate"),	v8::FunctionTemplate::New(njsGenerate)->GetFunction());
}

NODE_MODULE(prt4njs, init);
