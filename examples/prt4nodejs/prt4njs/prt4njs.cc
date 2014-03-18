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
#include "boost/scoped_array.hpp"

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


std::string toUTF8(const wchar_t* u16) {
	return boost::locale::conv::utf_to_utf<char>(u16);
}

std::string toUTF8(const std::wstring& u16) {
	return boost::locale::conv::utf_to_utf<char>(u16);
}

void toUTF16(const char* u8, std::wstring& u16) {
	u16.assign(boost::locale::conv::utf_to_utf<wchar_t>(u8));
}

template<typename T> v8::Local<v8::Value> toNJS(T v);

const char* AnnotationTypeNames[6] = { "void", "bool", "float", "string", "int", "unknown" };

template<> v8::Local<v8::Value> toNJS(char const* s) {
	return v8::Local<v8::String>(v8::String::New(s));
}

template<> v8::Local<v8::Value> toNJS(const std::string& s) {
	return v8::Local<v8::String>(v8::String::New(s.c_str()));
}

template<> v8::Local<v8::Value> toNJS(wchar_t const* s) {
	return v8::Local<v8::String>(v8::String::New(toUTF8(s).c_str()));
}

template<> v8::Local<v8::Value> toNJS(const std::wstring& s) {
	return v8::Local<v8::String>(v8::String::New(toUTF8(s).c_str()));
}

template<> v8::Local<v8::Value> toNJS(double d) {
	return v8::Local<v8::Number>(v8::Number::New(d));
}

template<> v8::Local<v8::Value> toNJS(bool d) {
	return v8::Local<v8::Number>(v8::Number::New(d));
}

template<> v8::Local<v8::Value> toNJS(int32_t i) {
	return v8::Local<v8::Integer>(v8::Integer::New(i));
}

template<> v8::Local<v8::Value> toNJS(prt::AnnotationArgumentType aat) {
	return toNJS(AnnotationTypeNames[aat]);
}

template<typename T> v8::Local<v8::Array> toNJS(T const* const a, size_t n) {
	v8::Local<v8::Array> njsArray = v8::Array::New(n);
	for (size_t i = 0; i < n; i++) {
		njsArray->Set(i, toNJS(a[i]));
	}
	return njsArray;
}

template<> v8::Local<v8::Value> toNJS(prt::AttributeMap const* am) {
	v8::Local<v8::Object> njsAM = v8::Object::New();
	size_t keyCount = 0;
	wchar_t const* const* const keys = am->getKeys(&keyCount);
	for (size_t k = 0; k < keyCount; k++) {
		const wchar_t* const key = keys[k];
		assert(key != 0);
		v8::Local<v8::Value> njsValue;
		switch(am->getType(key)) {
		case prt::Attributable::PT_STRING:
			njsValue = toNJS(am->getString(key));
			break;
		case prt::Attributable::PT_FLOAT:
			njsValue = toNJS(am->getFloat(key));
			break;
		case prt::Attributable::PT_BOOL:
			njsValue = toNJS(am->getBool(key));
			break;
		case prt::Attributable::PT_INT:
			njsValue = toNJS(am->getInt(key));
			break;
		case prt::Attributable::PT_STRING_ARRAY: {
			size_t n = 0;
			wchar_t const* const* a = am->getStringArray(key, &n);
			njsValue = toNJS(a, n);
			break;
		}
		case prt::Attributable::PT_INT_ARRAY: {
			size_t n = 0;
			int32_t const* a = am->getIntArray(key, &n);
			njsValue = toNJS(a, n);
			break;
		}
		case prt::Attributable::PT_FLOAT_ARRAY: {
			size_t n = 0;
			double const* a = am->getFloatArray(key, &n);
			njsValue = toNJS(a, n);
			break;
		}
		case prt::Attributable::PT_BOOL_ARRAY: {
			size_t n = 0;
			bool const* a = am->getBoolArray(key, &n);
			njsValue = toNJS(a, n);
			break;
		}
		default:
			break;
		}
		if (!njsValue.IsEmpty())
			njsAM->Set(v8::Local<v8::Value>::New(v8::String::New(toUTF8(key).c_str())), njsValue);
	}
	return njsAM;
}

template<> v8::Local<v8::Value> toNJS(prt::EncoderInfo const* info) {
	v8::Local<v8::Object> njsInfo = v8::Object::New();
	njsInfo->Set(v8::Local<v8::Value>::New(v8::String::New("id")), toNJS(info->getID()), v8::ReadOnly);

	prt::AttributeMap const* defOpts = 0;
	prt::Status status = info->createValidatedOptionsAndStates(0, &defOpts);
	assert(status == prt::STATUS_OK);
	njsInfo->Set(v8::Local<v8::Value>::New(v8::String::New("options")), toNJS(defOpts));
	defOpts->destroy();

	return njsInfo;
}

template<typename T> v8::Local<v8::Array> annotationsToNJS(T const* p) {
	const size_t numAnnots = p->getNumAnnotations();
	v8::Local<v8::Array> njsAnnots = v8::Array::New(numAnnots);
	for (size_t i = 0; i < numAnnots; i++) {
		prt::Annotation const* a = p->getAnnotation(i);

		const size_t numArgs = a->getNumArguments();
		v8::Local<v8::Array> njsArgs = v8::Array::New(numArgs);
		for (size_t ai = 0; ai < numArgs; ai++) {
			prt::AnnotationArgument const* arg = a->getArgument(ai);
			v8::Local<v8::Object> njsArg = v8::Object::New();
			njsArg->Set(v8::Local<v8::Value>::New(v8::String::New("type")), toNJS(arg->getType()), v8::ReadOnly);
			v8::Local<v8::Value> val;
			switch (arg->getType()) {
			case prt::AAT_BOOL:		val = v8::Local<v8::Boolean>::New(v8::Boolean::New(arg->getBool())); break;
			case prt::AAT_FLOAT:	val = v8::Number::New(arg->getFloat()); break;
			case prt::AAT_STR:		val = v8::String::New(toUTF8(arg->getStr()).c_str()); break;
			default: break;
			}
			njsArg->Set(v8::Local<v8::Value>::New(v8::String::New("val")), val, v8::ReadOnly);
			njsArgs->Set(ai, njsArg);
		}

		v8::Local<v8::Object> njsAnnot = v8::Object::New();
		njsAnnot->Set(v8::Local<v8::Value>::New(v8::String::New("name")), v8::Local<v8::Value>::New(v8::String::New(toUTF8(a->getName()).c_str())), v8::ReadOnly);
		njsAnnot->Set(v8::Local<v8::Value>::New(v8::String::New("args")), njsArgs, v8::ReadOnly);
		njsAnnots->Set(i, njsAnnot);


	}
	return njsAnnots;
}

v8::Local<v8::Object> toNJS(prt::RuleFileInfo::Entry const* entry) {
	const std::string name = boost::locale::conv::utf_to_utf<char>(entry->getName());
	std::vector<std::string> tokenized;
	boost::split(tokenized, name, boost::is_any_of("$"), boost::token_compress_on);

	const size_t numParams = entry->getNumParameters();
	v8::Local<v8::Array> njsRuleParams = v8::Array::New(numParams);
	for (size_t pi = 0; pi < numParams; pi++) {
		prt::RuleFileInfo::Parameter const* param = entry->getParameter(pi);

		v8::Local<v8::Object> njsParam = v8::Object::New();
		njsParam->Set(v8::Local<v8::Value>::New(v8::String::New("name")), v8::Local<v8::Value>::New(v8::String::New(toUTF8(param->getName()).c_str())), v8::ReadOnly);
		njsParam->Set(v8::Local<v8::Value>::New(v8::String::New("type")), toNJS(param->getType()), v8::ReadOnly);
		njsParam->Set(v8::Local<v8::Value>::New(v8::String::New("annots")), annotationsToNJS(param), v8::ReadOnly);
		njsRuleParams->Set(pi, njsParam);
	}

	v8::Local<v8::Object> njsRule = v8::Object::New();
	njsRule->Set(v8::Local<v8::Value>::New(v8::String::New("name")), v8::Local<v8::Value>::New(v8::String::New(tokenized[1].c_str())), v8::ReadOnly);
	njsRule->Set(v8::Local<v8::Value>::New(v8::String::New("style")), v8::Local<v8::Value>::New(v8::String::New(tokenized[0].c_str())), v8::ReadOnly);
	njsRule->Set(v8::Local<v8::Value>::New(v8::String::New("params")), njsRuleParams, v8::ReadOnly);
	njsRule->Set(v8::Local<v8::Value>::New(v8::String::New("annots")), annotationsToNJS(entry), v8::ReadOnly);

	return njsRule;
}

std::wstring fromNJS(const v8::Local<v8::String>& njs) {
	std::wstring ws;
	toUTF16(*v8::String::Utf8Value(njs), ws);
	return ws;
}

std::wstring fromNJS(const v8::Local<v8::Object>& obj, const std::string& field) {
	v8::Local<v8::String> njsField = v8::Local<v8::String>::Cast(obj->Get(toNJS<const std::string&>(field)));
	return fromNJS(njsField);
}

prt::AttributeMap const* fromNJS(const v8::Local<v8::Object>& njsObj) {
	prt::AttributeMapBuilder* isb = prt::AttributeMapBuilder::create();

	v8::Local<v8::Array> njsKeys = njsObj->GetPropertyNames();
	for (size_t i = 0; i < njsKeys->Length(); i++) {
		v8::Local<v8::String> njsKey = njsKeys->Get(i)->ToString();
		std::wstring key = fromNJS(njsKey);
		v8::Local<v8::Value> v = njsObj->Get(njsKey);
		if (v->IsString())
			isb->setString(key.c_str(), fromNJS(v->ToString()).c_str());
		else if (v->IsNumber())
			isb->setFloat(key.c_str(), v->NumberValue());
		else if (v->IsBoolean())
			isb->setBool(key.c_str(), v->BooleanValue());
		else if (v->IsInt32())
			isb->setInt(key.c_str(), v->Int32Value());
		else if (v->IsArray()) {
			v8::Local<v8::Array> njsArray = v8::Local<v8::Array>::Cast(v);
			if (njsArray->Length() > 0) {
				if (njsArray->Get(0)->IsString()) {
					boost::scoped_array<const wchar_t*> ptrs(new const wchar_t*[njsArray->Length()]);
					std::vector<std::wstring> strings(njsArray->Length());
					for (size_t ai = 0; ai < njsArray->Length(); ai++) {
						strings[ai] = fromNJS(njsArray->Get(ai)->ToString());
						ptrs[ai] = strings[ai].c_str();
					}
					isb->setStringArray(key.c_str(), ptrs.get(), strings.size());
				}
				else if (njsArray->Get(0)->IsNumber()) {
					boost::scoped_array<double> data(new double[njsArray->Length()]);
					for (size_t ai = 0; ai < njsArray->Length(); ai++) {
						data[ai] = njsArray->Get(ai)->NumberValue();
					}
					isb->setFloatArray(key.c_str(), data.get(), njsArray->Length());
				}
				else if (njsArray->Get(0)->IsBoolean()) {
					boost::scoped_array<bool> data(new bool[njsArray->Length()]);
					for (size_t ai = 0; ai < njsArray->Length(); ai++) {
						data[ai] = njsArray->Get(ai)->BooleanValue();
					}
					isb->setBoolArray(key.c_str(), data.get(), njsArray->Length());
				}
				else if (njsArray->Get(0)->IsInt32()) {
					boost::scoped_array<int32_t> data(new int32_t[njsArray->Length()]);
					for (size_t ai = 0; ai < njsArray->Length(); ai++) {
						data[ai] = njsArray->Get(ai)->Int32Value();
					}
					isb->setIntArray(key.c_str(), data.get(), njsArray->Length());
				}
			}
		}
	}

	prt::AttributeMap const* am = isb->createAttributeMap();
	isb->destroy();
	return am;
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


v8::Handle<v8::Value> njsShutdown(const v8::Arguments& args) {
	v8::HandleScope scope;

	delete prtCtx;
	prtCtx = 0;

	return scope.Close(v8::Undefined());
}


v8::Handle<v8::Value> njsRuleInfoAdv(const v8::Arguments& args) {
	v8::String::Utf8Value rpkURI(args[0]->ToString());
	v8::HandleScope scope;

	LOG_DBG << "ruleInfo for " << *rpkURI;
	prt::Status status = prt::STATUS_UNSPECIFIED_ERROR;
	const std::wstring wRPKUri = boost::locale::conv::utf_to_utf<wchar_t>(*rpkURI);
	prt::RuleFileInfo const* info = prt::createRuleFileInfo(wRPKUri.c_str(), prtCtx->mCache, &status);

	return scope.Close(NJSRuleFileInfo::create(info));
}


v8::Handle<v8::Value> njsGetRuleInfo(const v8::Arguments& args) {
	v8::String::Utf8Value rpkURI(args[0]->ToString());
	v8::Local<v8::Function> cb = v8::Local<v8::Function>::Cast(args[1]);
	v8::HandleScope scope;

	prt::Status status = prt::STATUS_UNSPECIFIED_ERROR;
	const std::wstring wRPKUri = boost::locale::conv::utf_to_utf<wchar_t>(*rpkURI);
	prt::RuleFileInfo const* info = prt::createRuleFileInfo(wRPKUri.c_str(), prtCtx->mCache, &status);

	const size_t numRules = info->getNumRules();
	v8::Local<v8::Array> njsRules = v8::Array::New(numRules);
	for (size_t ri = 0; ri < numRules; ri++) {
		prt::RuleFileInfo::Entry const* rule = info->getRule(ri);
		njsRules->Set(ri, toNJS(rule));
	}

	const size_t numAttributes = info->getNumAttributes();
	v8::Local<v8::Array> njsAttrs = v8::Array::New(numAttributes);
	for (size_t ai = 0; ai < numAttributes; ai++) {
		prt::RuleFileInfo::Entry const* attr = info->getAttribute(ai);
		njsAttrs->Set(ai, toNJS(attr));
	}

	info->destroy();

	v8::Local<v8::Object> njsInfo = v8::Object::New();
	njsInfo->Set(toNJS("rules"), njsRules, v8::ReadOnly);
	njsInfo->Set(toNJS("attributes"), njsAttrs, v8::ReadOnly);

	const unsigned argc = 1;
	v8::Local<v8::Value> argv[argc] = { njsInfo };
	cb->Call(v8::Context::GetCurrent()->Global(), argc, argv);
	return scope.Close(v8::Local<v8::Integer>(v8::Integer::New(static_cast<uint64_t>(status))));
}


namespace {
const std::wstring ID_DELIM = L";";
}


v8::Handle<v8::Value> njsListEncoderIDs(const v8::Arguments& args) {
	v8::Local<v8::Function> cb = v8::Local<v8::Function>::Cast(args[0]);
	v8::HandleScope scope;

	LOG_DBG << "njsListEncoderIDs";

	const size_t is = 1024;
	size_t s = is;
	boost::scoped_array<wchar_t> idStr(new wchar_t[is]);
	prt::Status status = prt::listEncoderIds(idStr.get(), &s);
	if (s > is) {
		idStr.reset(new wchar_t[s]);
		status = prt::listEncoderIds(idStr.get(), &s);
	}

	std::wstring idStr_(idStr.get());
	LOG_DBG << s << idStr_;

	std::vector<std::wstring> ids;
	boost::trim_right_if(idStr_, boost::is_any_of(ID_DELIM));
	boost::split(ids, idStr_, boost::is_any_of(ID_DELIM), boost::token_compress_on);

	const size_t numIDs = ids.size();
	v8::Local<v8::Array> njsEncIDs = v8::Array::New(numIDs);
	for (size_t i = 0; i < numIDs; i++) {
		njsEncIDs->Set(i, toNJS<const std::wstring&>(ids[i]));
	}

	const unsigned argc = 1;
	v8::Local<v8::Value> argv[argc] = { njsEncIDs };
	cb->Call(v8::Context::GetCurrent()->Global(), argc, argv);
	return scope.Close(v8::Local<v8::Integer>(v8::Integer::New(static_cast<uint64_t>(status))));
}


v8::Handle<v8::Value> njsGetEncoderInfo(const v8::Arguments& args) {
	v8::String::Utf8Value njsEncID(args[0]->ToString());
	v8::Local<v8::Function> cb = v8::Local<v8::Function>::Cast(args[1]);
	v8::HandleScope scope;

	prt::Status status = prt::STATUS_UNSPECIFIED_ERROR;
	const std::wstring wEncID = boost::locale::conv::utf_to_utf<wchar_t>(*njsEncID);
	prt::EncoderInfo const* info = prt::createEncoderInfo(wEncID.c_str(), &status);
	v8::Local<v8::Value> njsInfo = toNJS(info);
	info->destroy();

	const unsigned argc = 1;
	v8::Local<v8::Value> argv[argc] = { njsInfo };
	cb->Call(v8::Context::GetCurrent()->Global(), argc, argv);
	return scope.Close(v8::Local<v8::Integer>(v8::Integer::New(static_cast<uint64_t>(status))));
}


v8::Handle<v8::Value> njsGenerate(const v8::Arguments& args) {
	v8::Local<v8::Array> njsInitialShapes	= v8::Local<v8::Array>::Cast(args[0]);
	v8::Local<v8::Object> njsEncInfo		= v8::Local<v8::Object>::Cast(args[1]);
	v8::Local<v8::Object> cbObj				= v8::Local<v8::Object>::Cast(args[2]);
	v8::Local<v8::Function> cbGenEnd = v8::Local<v8::Function>::Cast(cbObj->Get(toNJS("generateEnd")));
	v8::HandleScope scope;

	prt::MemoryOutputCallbacks* moh = prt::MemoryOutputCallbacks::create();
	{
		std::vector<const prt::InitialShape*> initialShapes;
		initialShapes.reserve(njsInitialShapes->Length());
		prt::InitialShapeBuilder* isb = prt::InitialShapeBuilder::create();

		for (size_t isIdx = 0, numInitialShapes = njsInitialShapes->Length(); isIdx < numInitialShapes; isIdx++) {
			v8::Local<v8::Object> njsIS = v8::Local<v8::Object>::Cast(njsInitialShapes->Get(isIdx));

			std::wstring rpkURI = fromNJS(njsIS, "rpk");
			std::wstring uid = fromNJS(njsIS, "uid");
			std::wstring ruleFile = fromNJS(njsIS, "ruleFile");
			std::wstring startRule = fromNJS(njsIS, "startRule");

			v8::Local<v8::Object> njsAttributes = v8::Local<v8::Object>::Cast(njsIS->Get(toNJS("attributes")));
			prt::AttributeMap const* attributes = fromNJS(njsAttributes);

			prt::Status status = prt::STATUS_UNSPECIFIED_ERROR;
			const prt::ResolveMap* resolveMap = prt::createResolveMap(rpkURI.c_str(), 0, &status);

			v8::Local<v8::Array> njsVertices = v8::Local<v8::Array>::Cast(njsIS->Get(toNJS("vertices")));
			const size_t numVertices = njsVertices->Length();
			std::vector<double> vertices(numVertices);
			for (size_t vi = 0; vi < numVertices; vi++)
				vertices[vi] = njsVertices->Get(vi)->NumberValue();

			v8::Local<v8::Array> njsFaces = v8::Local<v8::Array>::Cast(njsIS->Get(toNJS("faces")));
			std::vector<uint32_t> faceIndices, faceCounts;
			const size_t numFaceIndices = njsFaces->Length();
			faceIndices.reserve(3*numFaceIndices); // estimate
			for (size_t fi = 0; fi < numFaceIndices; fi++) {
				v8::Local<v8::Array> njsFace = v8::Local<v8::Array>::Cast(njsFaces->Get(fi));
				for (size_t i = 0; i < njsFaces->Length(); i++) {
					faceIndices.push_back(njsFace->Get(i)->Uint32Value());
				}
				faceCounts.push_back(njsFaces->Length());
			}
			isb->setGeometry(
					&vertices[0], vertices.size(),
					&faceIndices[0], faceIndices.size(),
					&faceCounts[0], faceCounts.size()
			);

			isb->setAttributes(
					ruleFile.c_str(),
					startRule.c_str(),
					isb->computeSeed(),
					uid.c_str(),
					attributes,
					resolveMap
			);

			initialShapes.push_back(isb->createInitialShapeAndReset());
		}
		isb->destroy();

		std::wstring encID = fromNJS(njsEncInfo, "id");
		LOG_DBG << encID;
		v8::Local<v8::Object> njsEncOpts = v8::Local<v8::Object>::Cast(njsEncInfo->Get(toNJS("options")));
		prt::AttributeMap const* encOpts = fromNJS(njsEncOpts);

		prt::Status status = prt::STATUS_UNSPECIFIED_ERROR;
		prt::EncoderInfo const* encInfo = prt::createEncoderInfo(encID.c_str(), &status);
		const prt::AttributeMap* validEncOpts = 0;
		encInfo->createValidatedOptionsAndStates(encOpts, &validEncOpts);
		encInfo->destroy();

		const prt::AttributeMap* encoderOpts[] = {
				validEncOpts
				//			, validatedErrOpts
				//			, validatedPrintOpts
		};
		std::vector<const wchar_t*> encoders = boost::assign::list_of(encID.c_str());

		prt::Status stat = prt::generate(
				&initialShapes[0], initialShapes.size(),
				0,
				&encoders[0], encoders.size(),
				encoderOpts, moh, prtCtx->mCache, 0
		);
		if(stat != prt::STATUS_OK) {
			LOG_ERR << "prt::generate() failed with status: '" << prt::getStatusDescription(stat) << "' (" << stat << ")";
		}

		validEncOpts->destroy();
		for(size_t i=0; i<initialShapes.size(); i++)
			initialShapes[i]->destroy();

		v8::Local<v8::Object> result = v8::Local<v8::Object>(v8::Object::New());
		result->Set(toNJS("encoderInfo"), njsEncInfo);

		for (size_t bi = 0; bi < moh->getNumBlocks(); bi++) {
			size_t blockSize = 0;
			const uint8_t* blockData = moh->getBlock(bi, &blockSize);

			//result->Set(toNJS("data"), njsEncInfo);
		}

		const unsigned argc = 1;
		v8::Local<v8::Value> argv[argc] = { result };
		cbGenEnd->Call(v8::Context::GetCurrent()->Global(), argc, argv);
	}
	moh->destroy();

	//	return scope.Close(v8::Local<v8::Integer>(v8::Integer::New(static_cast<uint64_t>(status))));
	return scope.Close(v8::Undefined());
}


void init(v8::Handle<v8::Object> exports) {
	exports->Set(v8::String::NewSymbol("init"), 				v8::FunctionTemplate::New(njsInit)->GetFunction());
	exports->Set(v8::String::NewSymbol("shutdown"),				v8::FunctionTemplate::New(njsShutdown)->GetFunction());
	exports->Set(v8::String::NewSymbol("getRuleInfo"),			v8::FunctionTemplate::New(njsGetRuleInfo)->GetFunction());
	exports->Set(v8::String::NewSymbol("listEncoderIDs"),		v8::FunctionTemplate::New(njsListEncoderIDs)->GetFunction());
	exports->Set(v8::String::NewSymbol("getEncoderInfo"),		v8::FunctionTemplate::New(njsGetEncoderInfo)->GetFunction());
	exports->Set(v8::String::NewSymbol("generate"),				v8::FunctionTemplate::New(njsGenerate)->GetFunction());
}

NODE_MODULE(prt4njs, init);
