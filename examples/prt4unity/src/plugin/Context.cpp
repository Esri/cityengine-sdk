/**
 * Esri CityEngine SDK Unity Plugin Example
 *
 * This example demonstrates the main functionality of the Procedural Runtime API.
 * Esri R&D Center Zurich, Switzerland
 *
 * See http://github.com/ArcGIS/esri-cityengine-sdk for instructions.
 */

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include "Context.h"
#include "Utilities.h"
#include "IUnityOutputHandler.h"

#include <codecvt>
#include <map>
#include <cassert>
#include <unordered_map>

#include "prt/FlexLicParams.h"
#include "prt/LogHandler.h"
#include "prt/RuleFileInfo.h"


// constants
namespace {
	static const wchar_t* FILE_PREFIX      = L"file://";
	static const wchar_t  SEPERATOR        = L'\\';
	static const wchar_t* FLEXNET_LIB      = L"flexnet_prt.dll";

	// encoder IDs
	static const wchar_t* ENCODER_ID_UNITY = L"com.esri.prt.examples.UnityEncoder";
	static const wchar_t* ENCODER_ID_ATTR  = L"com.esri.prt.core.AttributeEvalEncoder";

	// annotations
	static const wchar_t* ANNOT_START_RULE = L"@StartRule";
	static const wchar_t* ANNOT_RANGE      = L"@Range";
	static const wchar_t* ANNOT_COLOR      = L"@Color";
	static const wchar_t* ANNOT_DIR        = L"@Directory";
	static const wchar_t* ANNOT_FILE       = L"@File";

	static const wchar_t* NULL_KEY         = L"#NULL#";

	// log settings
	static const bool ENABLE_LOG_CONSOLE	= true;
	static const bool ENABLE_LOG_FILE		= true;
}


/*
 *	Output handler, receiving the attributes from the AttributeEvalEncoder (stored in the handler object) and the meshes and materials from the UnityEncoder (directly stored in the provided Context)
 */
class Context::OutputHandler : public IUnityOutputHandler {
public:
	struct AttributeHolder {
		enum Type {
			TYPE_INVALID,
			TYPE_BOOL,
			TYPE_FLOAT,
			TYPE_STRING
		} mType;

		union {
			bool mBool;
			double mFloat;
		};
		std::wstring mString;

		AttributeHolder()
			: mType(TYPE_INVALID) {}
	};

private:
	std::map<std::wstring, AttributeHolder> mAttrs;			// attributes (name + value) returned by AttributeEvalEncoder
	Context& mContext;										// context receiving the meshes and materials
	const wchar_t* mSpecialMaterial;						// name of special material for which a separate mesh should be returned (or multiple if exceeding Unity's per-mesh vertex limit)

public:
	OutputHandler(Context& ctx, const wchar_t* specialMaterial)
		: mContext(ctx)
		, mSpecialMaterial(specialMaterial) {}

public:		// inherited prt::Callbacks interface
	prt::Status generateError(size_t /*isIndex*/, prt::Status /*status*/, const wchar_t* message) {
		std::wstring msg(L"GENERATE ERROR: "); msg.append(message);
		Context::postUnityLogMessage(msg.c_str(), prt::LOG_ERROR);
		return prt::STATUS_OK;
	}
	prt::Status assetError(size_t /*isIndex*/, prt::CGAErrorLevel /*level*/, const wchar_t* /*key*/, const wchar_t* /*uri*/, const wchar_t* message) {
		std::wstring msg(L"ASSET ERROR: "); msg.append(message);
		Context::postUnityLogMessage(msg.c_str(), prt::LOG_ERROR);
		return prt::STATUS_OK;
	}
	prt::Status cgaError(size_t /*isIndex*/, int32_t /*shapeID*/, prt::CGAErrorLevel /*level*/, int32_t /*methodId*/, int32_t /*pc*/, const wchar_t* message) {
		std::wstring msg(L"CGA ERROR: "); msg.append(message);
		Context::postUnityLogMessage(msg.c_str(), prt::LOG_ERROR);
		return prt::STATUS_OK;
	}
	prt::Status cgaPrint(size_t /*isIndex*/, int32_t /*shapeID*/, const wchar_t* /*txt*/) {
		return prt::STATUS_OK;
	}
	prt::Status cgaReportBool(size_t /*isIndex*/, int32_t /*shapeID*/, const wchar_t* /*key*/, bool /*value*/) {
		return prt::STATUS_OK;
	}
	prt::Status cgaReportFloat(size_t /*isIndex*/, int32_t /*shapeID*/, const wchar_t* /*key*/, double /*value*/) {
		return prt::STATUS_OK;
	}
	prt::Status cgaReportString(size_t /*isIndex*/, int32_t /*shapeID*/, const wchar_t* /*key*/, const wchar_t* /*value*/) {
		return prt::STATUS_OK;
	}
	prt::Status attrBool(size_t isIndex, int32_t shapeID, const wchar_t* key, bool value);
	prt::Status attrFloat(size_t isIndex, int32_t shapeID, const wchar_t* key, double value);
	prt::Status attrString(size_t isIndex, int32_t shapeID, const wchar_t* key, const wchar_t* value);

public:		// IUnityOutputHandler interface
	void newMesh(const wchar_t* name, size_t numVertices, const float* vertices, const float* normals, const float* texcoords, size_t numSubMeshes, const size_t* numIndices, const uint32_t* indices, const uint32_t* materials);
	void newMaterial(uint32_t id, const wchar_t* name);
	void matSetColor(uint32_t id, float r, float g, float b);
	void matSetDiffuseTexture(uint32_t id, const wchar_t* tex);

	const wchar_t* getSpecialMaterial() {
		return mSpecialMaterial;
	}

public:		// accessors
	const std::map<std::wstring, AttributeHolder>& getAttrs() const {
		return mAttrs;
	}
};

prt::Status Context::OutputHandler::attrBool(size_t /*isIndex*/, int32_t /*shapeID*/, const wchar_t* key, bool value) {
	AttributeHolder& attr = mAttrs[key];
	attr.mBool = value;
	attr.mType = AttributeHolder::TYPE_BOOL;
	return prt::STATUS_OK;
}

prt::Status Context::OutputHandler::attrFloat(size_t /*isIndex*/, int32_t /*shapeID*/, const wchar_t* key, double value) {
	AttributeHolder& attr = mAttrs[key];
	attr.mFloat = value;
	attr.mType = AttributeHolder::TYPE_FLOAT;
	return prt::STATUS_OK;
}

prt::Status Context::OutputHandler::attrString(size_t /*isIndex*/, int32_t /*shapeID*/, const wchar_t* key, const wchar_t* value) {
	AttributeHolder& attr = mAttrs[key];
	attr.mString.assign(value);
	attr.mType = AttributeHolder::TYPE_STRING;
	return prt::STATUS_OK;
}

void Context::OutputHandler::newMesh(const wchar_t* name, size_t numVertices, const float* vertices, const float* normals, const float* texcoords, size_t numSubMeshes, const size_t* numIndices, const uint32_t* indices, const uint32_t* materials) {
	// create a new mesh
	Mesh* mesh = new Mesh();
	mContext.mMeshes.push_back(mesh);
	if(name)
		mesh->mName.assign(name);

	// set vertex positions, normals and texture coordinates
	mesh->mVertices.assign(vertices, vertices + numVertices * 3);
	if(normals)
		mesh->mNormals.assign(normals, normals + numVertices * 3);
	if(texcoords)
		mesh->mTexcoords.assign(texcoords, texcoords + numVertices * 2);

	// set indices and sub-meshes
	mesh->mSubMeshes.resize(numSubMeshes);
	size_t totalNumIndices = 0;
	for(size_t i = 0; i < numSubMeshes; i++)
		totalNumIndices += numIndices[i];
	mesh->mIndices.resize(totalNumIndices);
	uint32_t firstIndex = 0;
	std::vector<uint32_t>::iterator itIndex = mesh->mIndices.begin();
	for(size_t i = 0; i < numSubMeshes; i++) {
		Mesh::SubMesh& subMesh = mesh->mSubMeshes[i];
		subMesh.mFirstIndex = firstIndex;
		subMesh.mNumIndices = numIndices[i];
		std::copy(indices, indices + subMesh.mNumIndices, itIndex);
		indices += subMesh.mNumIndices;
		itIndex += subMesh.mNumIndices;
		firstIndex += subMesh.mNumIndices;
		subMesh.mMaterial = materials[i];
	}
}

void Context::OutputHandler::newMaterial(uint32_t id, const wchar_t* name) {
	if(id >= mContext.mMaterials.size())
		mContext.mMaterials.resize(id + 1, NULL);
	Material*& material = mContext.mMaterials[id];
	if(material)
		delete material;
	material = new Material();
	material->mName.assign(name);
}

void Context::OutputHandler::matSetColor(uint32_t id, float r, float g, float b) {
	if(id < mContext.mMaterials.size()) {
		Material* material = mContext.mMaterials[id];
		material->mColor[0] = r;
		material->mColor[1] = g;
		material->mColor[2] = b;
	}
}

void Context::OutputHandler::matSetDiffuseTexture(uint32_t id, const wchar_t* tex) {
	if(id < mContext.mMaterials.size()) {
		Material* material = mContext.mMaterials[id];
		material->mDiffuseTexture.assign(tex);
	}
}


/*
 *	Context
 */

// static variables
UnityLogHandler*     Context::theLogHandler     = NULL;
prt::FileLogHandler* Context::theFileLogHandler = NULL;
const prt::Object*   Context::theLicHandle      = NULL;
prt::CacheObject*    Context::theCache          = NULL;

Context::Context()
	: mUnityEncOpts(NULL)
	, mAttrEncOpts(NULL)
	, mResolveMap(NULL)
	, mRuleFileInfo(NULL)
{
	// create validated options for used encoders
	{
		const prt::EncoderInfo* encInfo = prt::createEncoderInfo(ENCODER_ID_UNITY);
		if(encInfo == NULL) {
			std::wstring msg(L"Unable to locate "); msg.append(ENCODER_ID_UNITY);
			::MessageBoxW(NULL, msg.c_str(), L"prt4unity", MB_OK | MB_ICONERROR);
		} else {
			encInfo->createValidatedOptionsAndStates(NULL, &mUnityEncOpts, NULL);
			encInfo->destroy();
		}
	}
	{
		const prt::EncoderInfo* encInfo = prt::createEncoderInfo(ENCODER_ID_ATTR);
		if(encInfo == NULL) {
			std::wstring msg(L"Unable to locate "); msg.append(ENCODER_ID_ATTR);
			::MessageBoxW(NULL, msg.c_str(), L"prt4unity", MB_OK | MB_ICONERROR);
		} else {
			encInfo->createValidatedOptionsAndStates(NULL, &mAttrEncOpts, NULL);
			encInfo->destroy();
		}
	}
}

Context::~Context() {
	clearAttributes();
	clearMeshes();
	clearMaterials();
	SafeDestroy(mUnityEncOpts);
	SafeDestroy(mAttrEncOpts);
	SafeDestroy(mResolveMap);
	SafeDestroy(mRuleFileInfo);
}

// deletes all attributes
void Context::clearAttributes() {
	for(std::vector<Attribute*>::iterator it = mAttributes.begin(), itEnd = mAttributes.end(); it != itEnd; ++it) {
		switch((*it)->mType) {
			case Attribute::ATTR_BOOL:
				delete static_cast<BoolAttribute*>(*it);
				break;
			case Attribute::ATTR_FLOAT:
				delete static_cast<FloatAttribute*>(*it);
				break;
			case Attribute::ATTR_STRING:
			case Attribute::ATTR_DIRECTORY:
			case Attribute::ATTR_COLOR:
				delete static_cast<StringAttribute*>(*it);
				break;
			case Attribute::ATTR_ENUM_BOOL:
			case Attribute::ATTR_ENUM_FLOAT:
			case Attribute::ATTR_ENUM_STRING:
				delete static_cast<EnumAttribute*>(*it);
				break;
			case Attribute::ATTR_FILE:
				delete static_cast<FileAttribute*>(*it);
				break;
		}
	}
	mAttributes.clear();
}

// deletes all meshes
void Context::clearMeshes() {
	for(std::vector<Mesh*>::iterator it = mMeshes.begin(), itEnd = mMeshes.end(); it != itEnd; ++it)
		delete *it;
	mMeshes.clear();
}

// deletes all materials
void Context::clearMaterials() {
	for(std::vector<Material*>::iterator it = mMaterials.begin(), itEnd = mMaterials.end(); it != itEnd; ++it)
		delete *it;
	mMaterials.clear();
}

// returns path to dynamically loaded PRT DLLs
std::wstring Context::getPrtLibRoot() {
	HMODULE hModule = NULL;
	GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCTSTR)prt::init, &hModule);

	wchar_t dllPath[_MAX_PATH];
	if(!::GetModuleFileNameW(hModule, dllPath, _countof(dllPath)))
		return L"<error>";

	wchar_t drive[8];
	wchar_t dir[_MAX_PATH];
	_wsplitpath_s(dllPath, drive, _countof(drive), dir, _countof(dir), NULL, 0, NULL, 0);

	std::wstring root(drive);
	root.append(dir);
	root.append(L"prt");
	root.push_back(SEPERATOR);
	return root;
}

// returns path to this plug-in DLL
std::wstring Context::getPluginRoot() {
	HMODULE hModule = NULL;
	GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCTSTR)Context::getPluginRoot, &hModule);

	wchar_t dllPath[_MAX_PATH];
	if(!::GetModuleFileNameW(hModule, dllPath, _countof(dllPath)))
		return L"<error>";

	wchar_t drive[8];
	wchar_t dir[_MAX_PATH];
	_wsplitpath_s(dllPath, drive, _countof(drive), dir, _countof(dir), NULL, 0, NULL, 0);

	std::wstring root(drive);
	root.append(dir);
	return root;
}

bool Context::initialize(const char* licFeature, const char* licHost) {
	const std::wstring root = getPrtLibRoot();

	// set up console log handler
	if(ENABLE_LOG_CONSOLE) {
		theLogHandler = new UnityLogHandler(prt::LogHandler::ALL, prt::LogHandler::ALL_COUNT);
		prt::addLogHandler(theLogHandler);
	}

	// set up file log handler
	if(ENABLE_LOG_FILE) {
		std::wstring logPath = getPluginRoot() + L"prt4unity.log";
		theFileLogHandler = prt::FileLogHandler::create(prt::LogHandler::ALL, prt::LogHandler::ALL_COUNT, logPath.c_str());
		prt::addLogHandler(theFileLogHandler);
	}

	// set up licensing information
	if(licFeature == NULL || strcmp(licFeature, "CityEngBas") != 0 && strcmp(licFeature, "CityEngBasFx") != 0 && strcmp(licFeature, "CityEngAdv") != 0 && strcmp(licFeature, "CityEngAdvFx") != 0) {
		std::wstring msg(L"Invalid license feature ");
		if(licFeature != NULL) {
			msg.push_back(L'\'');
			msg.append(std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(licFeature));
			msg.push_back(L'\'');
		} else
			msg.append(L"(null)");
		msg.append(L"; must be 'CityEngBas', 'CityEngBasFx', 'CityEngAdv', or 'CityEngAdvFx'");
		::MessageBoxW(NULL, msg.c_str(), L"prt4unity", MB_OK | MB_ICONERROR);
		return false;
	}

	if((strcmp(licFeature, "CityEngBas") == 0 || strcmp(licFeature, "CityEngAdv") == 0) && (licHost == NULL || licHost[0] == 0)) {
		std::wstring msg(L"License type '");
		msg.append(std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(licFeature));
		msg.append(L"' requires a license server hostname (<port>@<host>, e.g. 27000@flexnet.host.com)");
		::MessageBoxW(NULL, msg.c_str(), L"prt4unity", MB_OK | MB_ICONERROR);
		return false;
	}

	const std::wstring flexLib = root + FLEXNET_LIB;
	const std::string flexLibPath = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(flexLib);
	prt::FlexLicParams flp;
	flp.mActLibPath = flexLibPath.c_str();
	flp.mFeature = licFeature;
	flp.mHostName = licHost != NULL ? licHost : "";

	// initialize PRT with the path to its extension libraries, the desired log level and the licensing data
	prt::Status status;
	const wchar_t* prtPlugIns[1] = { root.c_str() };
	theLicHandle = prt::init(prtPlugIns, _countof(prtPlugIns), prt::LOG_INFO, &flp, &status);
	if(status != prt::STATUS_OK)
		return false;

	// create cache
	theCache = prt::CacheObject::create(prt::CacheObject::CACHE_TYPE_DEFAULT);
	return true;
}

void Context::uninitialize() {
	if(ENABLE_LOG_CONSOLE && theLogHandler) {
		prt::removeLogHandler(theLogHandler);
		delete theLogHandler;
		theLogHandler = NULL;
	}
	if(ENABLE_LOG_FILE && theFileLogHandler) {
		prt::removeLogHandler(theFileLogHandler);
		theFileLogHandler->destroy();
		theFileLogHandler = NULL;
	}
	SafeDestroy(theLicHandle);
	SafeDestroy(theCache);
}

// sets callback for forwarding log messages to Unity
void Context::setLogMessageCallback(UnityLogHandler::LogMessageCallback callback) {
	if(theLogHandler != NULL)
		theLogHandler->setCallback(callback);
}

// posts a log message directly to Unity using the provided callback
void Context::postUnityLogMessage(const wchar_t* message, uint32_t level) {
	if(theLogHandler) {
		UnityLogHandler::LogMessageCallback callback = theLogHandler->getCallback();
		if(callback)
			callback(message, level);
	}
}

// loads a rule package and makes it current
bool Context::setRulePackage(const wchar_t* filename, const wchar_t* unpackPath) {
	// discard data for previous rule package
	mRuleFiles.clear();
	mStartRules.clear();
	clearAttributes();
	clearMeshes();
	clearMaterials();
	SafeDestroy(mResolveMap);
	SafeDestroy(mRuleFileInfo);

	// create resolve map based on rule package
	std::wstring url(FILE_PREFIX); url.append(filename);
	prt::Status status = prt::STATUS_UNSPECIFIED_ERROR;
	mResolveMap = prt::createResolveMap(url.c_str(), unpackPath, &status);
	if(status != prt::STATUS_OK)
		return false;

	// query and store names of rule files contained in rule package
	size_t nKeys;
	const wchar_t* const* keys = mResolveMap->getKeys(&nKeys);
	const wchar_t* cgb = L".cgb";
	for(size_t k = 0; k < nKeys; k++) {
		const wchar_t* key = keys[k];
		const size_t len = std::char_traits<wchar_t>::length(key);
		if(len > 4 && std::char_traits<wchar_t>::compare(cgb, key + len - 4, 4) == 0)
			mRuleFiles.push_back(std::wstring(key, len));
	}

	return true;
}

// returns the names of all rule files contained in the current rule package
const std::vector<std::wstring>& Context::getRuleFiles() const {
	return mRuleFiles;
}

// sets the current rule file
bool Context::setRuleFile(size_t index) {
	if(index >= mRuleFiles.size())
		return false;
	const std::wstring& ruleFile = mRuleFiles[index];

	// discard data for previous rule file
	mStartRules.clear();
	clearAttributes();
	clearMeshes();
	clearMaterials();
	SafeDestroy(mRuleFileInfo);
	mRuleFileIndex = index;

	// query and store all start rules contained in rule file
	std::vector<const prt::RuleFileInfo::Entry*> annotStartRules;
	std::vector<const prt::RuleFileInfo::Entry*> noArgRules;

	prt::Status status = prt::STATUS_UNSPECIFIED_ERROR;
	mRuleFileInfo = prt::createRuleFileInfo(mResolveMap->getString(ruleFile.c_str()), 0, &status);
	if(status != prt::STATUS_OK)
		return false;

	for(size_t r = 0; r < mRuleFileInfo->getNumRules(); r++) {
		if(mRuleFileInfo->getRule(r)->getNumParameters() > 0)
			continue;
		bool startRule = false;
		for(size_t a = 0; a < mRuleFileInfo->getRule(r)->getNumAnnotations(); a++) {
			if(!(wcscmp(mRuleFileInfo->getRule(r)->getAnnotation(a)->getName(), ANNOT_START_RULE))) {
				annotStartRules.push_back(mRuleFileInfo->getRule(r));
				startRule = true;
				break;
			}
		}
		if(!startRule)
			noArgRules.push_back(mRuleFileInfo->getRule(r));
	}

	mStartRules.swap(annotStartRules.size() > 0 ? annotStartRules : noArgRules);
	return true;
}

// returns the names of all start rules contained in the current rule file
const std::vector<const prt::RuleFileInfo::Entry*>& Context::getStartRules() const {
	return mStartRules;
}

namespace {
	// default initial shape geometry (a quad) for querying rule attributes
	static const double   UnitQuad_vertices[]   = { 0, 0, 0,  0, 0, 1,  1, 0, 1,  1, 0, 0 };
	static const uint32_t UnitQuad_indices[]    = { 0, 1, 2, 3 };
	static const uint32_t UnitQuad_faceCounts[] = { 4 };
}

// sets the current start rule
bool Context::setStartRule(size_t index) {
	if(index >= mStartRules.size())
		return false;
	const prt::RuleFileInfo::Entry* ruleInfo = mStartRules[index];

	// discard data for previous start rule
	clearAttributes();
	clearMeshes();
	clearMaterials();
	mStartRuleIndex = index;

	ScopedObject<prt::AttributeMapBuilder> aBuilder(prt::AttributeMapBuilder::create());
	ScopedObject<const prt::AttributeMap> attrs(aBuilder->createAttributeMap());

	// create dummy initial shape for querying attributes
	ScopedObject<prt::InitialShapeBuilder> isb(prt::InitialShapeBuilder::create());
	isb->setGeometry(UnitQuad_vertices, _countof(UnitQuad_vertices),
	                 UnitQuad_indices, _countof(UnitQuad_indices),
	                 UnitQuad_faceCounts, _countof(UnitQuad_faceCounts));
	isb->setAttributes(mRuleFiles[mRuleFileIndex].c_str(), ruleInfo->getName(), isb->computeSeed(), L"", attrs.get(), mResolveMap);
	ScopedObject<const prt::InitialShape> shape(isb->createInitialShapeAndReset());

	// generate, using the attribute evaluation encoder
	const prt::InitialShape* initialShapes[] = { shape.get() };
	OutputHandler outputHandler(*this, NULL);
	prt::Status status = prt::generate(initialShapes, _countof(initialShapes), 0, &ENCODER_ID_ATTR, 1, &mAttrEncOpts, &outputHandler, theCache, NULL);

	// query and store attributes
	for(size_t i = 0; i < mRuleFileInfo->getNumAttributes(); i++) {
		const prt::RuleFileInfo::Entry* attrInfo = mRuleFileInfo->getAttribute(i);
		if(attrInfo->getNumParameters() != 0)
			continue;

		const std::wstring name(attrInfo->getName());

		// retrieve value from generate output
		const std::map<std::wstring, OutputHandler::AttributeHolder>::const_iterator itValue = outputHandler.getAttrs().find(name);
		if(itValue == outputHandler.getAttrs().end())
			continue;

		// create and add specialized attribute object, taking annotations into account
		switch(attrInfo->getReturnType()) {
			case prt::AAT_BOOL: {
				assert(itValue->second.mType == OutputHandler::AttributeHolder::TYPE_BOOL);
				const bool value = itValue->second.mBool;
				size_t a;
				for(a = 0; a < attrInfo->getNumAnnotations(); a++) {
					const prt::Annotation* an = attrInfo->getAnnotation(a);
					if(wcscmp(an->getName(), ANNOT_RANGE) == 0) {
						addEnumAttribute(name, an, value);
						goto setGroup;
					}
				}
				addBoolAttribute(name, value);
				break;
			}

			case prt::AAT_FLOAT: {
				assert(itValue->second.mType == OutputHandler::AttributeHolder::TYPE_FLOAT);
				const double value = itValue->second.mFloat;
				double min = std::numeric_limits<double>::quiet_NaN();
				double max = std::numeric_limits<double>::quiet_NaN();
				for(size_t a = 0; a < attrInfo->getNumAnnotations(); a++) {
					const prt::Annotation* an = attrInfo->getAnnotation(a);
					if(wcscmp(an->getName(), ANNOT_RANGE) == 0) {
						if(an->getNumArguments() == 2 && an->getArgument(0)->getType() == prt::AAT_FLOAT && an->getArgument(1)->getType() == prt::AAT_FLOAT) {
							min = an->getArgument(0)->getFloat();
							max = an->getArgument(1)->getFloat();
							break;
						} else {
							addEnumAttribute(name, an, value);
							goto setGroup;
						}
					}
				}
				addFloatAttribute(name, value, min, max);
				break;
			}

			case prt::AAT_STR: {
				assert(itValue->second.mType == OutputHandler::AttributeHolder::TYPE_STRING);
				const std::wstring& value = itValue->second.mString;
				for(size_t a = 0; a < attrInfo->getNumAnnotations(); a++) {
					const prt::Annotation* an = attrInfo->getAnnotation(a);
					if(wcscmp(an->getName(), ANNOT_RANGE) == 0) {
						addEnumAttribute(name, an, value);
						goto setGroup;
					} else if(wcscmp(an->getName(), ANNOT_COLOR) == 0) {
						addColorAttribute(name, value);
						goto setGroup;
					} else if(wcscmp(an->getName(), ANNOT_DIR) == 0) {
						addDirectoryAttribute(name, value);
						goto setGroup;
					} else if(wcscmp(an->getName(), ANNOT_FILE) == 0) {
						std::wstring ext;
						for(size_t arg = 0; arg < an->getNumArguments(); arg++) {
							if(an->getArgument(arg)->getType() == prt::AAT_STR) {
								ext += L"*.";
								ext += an->getArgument(arg)->getStr();
								if(!ext.empty())
									ext.push_back(';');
							}
						}
						addFileAttribute(name, value, ext);
						goto setGroup;
					}
				}
				if(value.size() == 7 && value.front() == L'#')
					addColorAttribute(name, value);
				else
					addStringAttribute(name, value);
				break;
			}

			default:
				continue;
		}

	setGroup:
		;
	}

	return true;
}


//////	Private member functions for adding attributes

void Context::addBoolAttribute(const std::wstring& name, bool value) {
	BoolAttribute* attr = new BoolAttribute();
	mAttributes.push_back(attr);
	attr->mType = Attribute::ATTR_BOOL;
	attr->mName = name;
	attr->mValue = value;
}

void Context::addFloatAttribute(const std::wstring& name, double value, double minValue, double maxValue) {
	FloatAttribute* attr = new FloatAttribute();
	mAttributes.push_back(attr);
	attr->mType = Attribute::ATTR_FLOAT;
	attr->mName = name;
	attr->mValue = value;
	attr->mMin = minValue;
	attr->mMax = maxValue;
}

void Context::addStringAttribute(const std::wstring& name, const std::wstring& value) {
	StringAttribute* attr = new StringAttribute();
	mAttributes.push_back(attr);
	attr->mType = Attribute::ATTR_STRING;
	attr->mName = name;
	attr->mValue = value;
}

void Context::addDirectoryAttribute(const std::wstring& name, const std::wstring& value) {
	StringAttribute* attr = new StringAttribute();
	mAttributes.push_back(attr);
	attr->mType = Attribute::ATTR_DIRECTORY;
	attr->mName = name;
	attr->mValue = value;
}

void Context::addColorAttribute(const std::wstring& name, const std::wstring& value) {
	StringAttribute* attr = new StringAttribute();
	mAttributes.push_back(attr);
	attr->mType = Attribute::ATTR_COLOR;
	attr->mName = name;
	attr->mValue = value;
}

void Context::addFileAttribute(const std::wstring& name, const std::wstring& value, const std::wstring& ext) {
	FileAttribute* attr = new FileAttribute();
	mAttributes.push_back(attr);
	attr->mType = Attribute::ATTR_FILE;
	attr->mName = name;
	attr->mValue = value;
	attr->mExt = ext;
}

void Context::addEnumAttribute(const std::wstring& name, const prt::Annotation* annotation, bool value) {
	uint32_t index = 0;
	for(size_t i = 0; i < annotation->getNumArguments(); i++) {
		if(annotation->getArgument(i)->getBool() == value) {
			index = i;
			break;
		}
	}
	addEnumAttribute(Attribute::ATTR_ENUM_BOOL, name, annotation, index);
}

void Context::addEnumAttribute(const std::wstring& name, const prt::Annotation* annotation, double value) {
	uint32_t index = 0;
	for(size_t i = 0; i < annotation->getNumArguments(); i++) {
		if(annotation->getArgument(i)->getFloat() == value) {
			index = i;
			break;
		}
	}
	addEnumAttribute(Attribute::ATTR_ENUM_FLOAT, name, annotation, index);
}

void Context::addEnumAttribute(const std::wstring& name, const prt::Annotation* annotation, const std::wstring& value) {
	uint32_t index = 0;
	for(size_t i = 0; i < annotation->getNumArguments(); i++) {
		if(annotation->getArgument(i)->getStr() == value) {
			index = i;
			break;
		}
	}
	addEnumAttribute(Attribute::ATTR_ENUM_STRING, name, annotation, index);
}

void Context::addEnumAttribute(Attribute::Type type, const std::wstring& name, const prt::Annotation* annotation, uint32_t index) {
	EnumAttribute* attr = new EnumAttribute();
	mAttributes.push_back(attr);
	attr->mType = type;
	attr->mName = name;
	attr->mAnnotation = annotation;
	attr->mIndex = index;
}


//////	Public member functions for querying the attributes

size_t Context::getAttributeCount() const {
	return mAttributes.size();
}

Context::Attribute::Type Context::getAttributeType(size_t attrIndex) const {
	if(attrIndex >= mAttributes.size())
		return Attribute::ATTR_INVALID;
	return mAttributes[attrIndex]->mType;
}

const wchar_t* Context::getAttributeName(size_t attrIndex) const {
	if(attrIndex >= mAttributes.size())
		return NULL;
	return mAttributes[attrIndex]->mName.c_str();
}

bool Context::getBoolAttributeValue(size_t attrIndex, bool& value) const {
	if(attrIndex >= mAttributes.size())
		return false;
	const Attribute* attr = mAttributes[attrIndex];
	if((attr->mType & Attribute::ATTR_BOOL) == 0)
		return false;
	value = static_cast<const BoolAttribute*>(attr)->mValue;
	return true;
}

bool Context::getFloatAttributeValue(size_t attrIndex, double& value, double& minValue, double& maxValue) const {
	if(attrIndex >= mAttributes.size())
		return false;
	const Attribute* attr = mAttributes[attrIndex];
	if((attr->mType & Attribute::ATTR_FLOAT) == 0)
		return false;
	const FloatAttribute* floatAttr = static_cast<const FloatAttribute*>(attr);
	value = floatAttr->mValue;
	minValue = floatAttr->mMin;
	maxValue = floatAttr->mMax;
	return true;
}

bool Context::getStringAttributeValue(size_t attrIndex, const wchar_t*& value) const {
	if(attrIndex >= mAttributes.size())
		return false;
	const Attribute* attr = mAttributes[attrIndex];
	if((attr->mType & Attribute::ATTR_STRING) == 0)
		return false;
	value = static_cast<const StringAttribute*>(attr)->mValue.c_str();
	return true;
}

bool Context::getDirectoryAttributeValue(size_t attrIndex, const wchar_t*& value) const {
	if(attrIndex >= mAttributes.size())
		return false;
	const Attribute* attr = mAttributes[attrIndex];
	if((attr->mType & Attribute::ATTR_DIRECTORY) == 0)
		return false;
	value = static_cast<const StringAttribute*>(attr)->mValue.c_str();
	return true;
}

bool Context::getFileAttributeValue(size_t attrIndex, const wchar_t*& value, const wchar_t*& ext) const {
	if(attrIndex >= mAttributes.size())
		return false;
	const Attribute* attr = mAttributes[attrIndex];
	if((attr->mType & Attribute::ATTR_FILE) == 0)
		return false;
	const FileAttribute* fileAttr = static_cast<const FileAttribute*>(attr);
	value = fileAttr->mValue.c_str();
	ext = fileAttr->mExt.c_str();
	return true;
}

bool Context::getColorAttributeValue(size_t attrIndex, const wchar_t*& value) const {
	if(attrIndex >= mAttributes.size())
		return false;
	const Attribute* attr = mAttributes[attrIndex];
	if((attr->mType & Attribute::ATTR_COLOR) == 0)
		return false;
	value = static_cast<const StringAttribute*>(attr)->mValue.c_str();
	return true;
}

bool Context::getEnumAttributeValue(size_t attrIndex, uint32_t& value) const {
	if(attrIndex >= mAttributes.size())
		return false;
	const Attribute* attr = mAttributes[attrIndex];
	if((attr->mType & Attribute::ATTR_ENUM) == 0)
		return false;
	value = static_cast<const EnumAttribute*>(attr)->mIndex;
	return true;
}

size_t Context::getEnumAttributeFieldCount(size_t attrIndex) const {
	if(attrIndex >= mAttributes.size())
		return 0;
	const Attribute* attr = mAttributes[attrIndex];
	if((attr->mType & Attribute::ATTR_ENUM) == 0)
		return 0;
	return static_cast<const EnumAttribute*>(attr)->mAnnotation->getNumArguments();
}

bool Context::getEnumAttributeField(size_t attrIndex, size_t fieldIndex, const wchar_t*& fieldKey) const {
	if(attrIndex >= mAttributes.size())
		return false;
	const Attribute* attr = mAttributes[attrIndex];
	if((attr->mType & Attribute::ATTR_ENUM) == 0)
		return false;
	const prt::Annotation* an = static_cast<const EnumAttribute*>(attr)->mAnnotation;
	if(fieldIndex >= an->getNumArguments())
		return false;

	const prt::AnnotationArgument* arg = an->getArgument(fieldIndex);
	const wchar_t* key = arg->getKey();
	if(wcscmp(key, NULL_KEY) == 0)
		key = arg->getStr();
	fieldKey = key;
	return true;
}


//////	Public member functions for setting the values of attributes

bool Context::setBoolAttributeValue(size_t attrIndex, bool value) {
	if(attrIndex >= mAttributes.size())
		return false;
	Attribute* attr = mAttributes[attrIndex];
	if((attr->mType & Attribute::ATTR_BOOL) == 0)
		return false;
	static_cast<BoolAttribute*>(attr)->mValue = value;
	return true;
}

bool Context::setFloatAttributeValue(size_t attrIndex, double value) {
	if(attrIndex >= mAttributes.size())
		return false;
	Attribute* attr = mAttributes[attrIndex];
	if((attr->mType & Attribute::ATTR_FLOAT) == 0)
		return false;
	static_cast<FloatAttribute*>(attr)->mValue = value;
	return true;
}

bool Context::setStringAttributeValue(size_t attrIndex, const wchar_t* value) {
	if(attrIndex >= mAttributes.size())
		return false;
	Attribute* attr = mAttributes[attrIndex];
	if((attr->mType & Attribute::ATTR_STRING) == 0)
		return false;
	std::wstring& attrValue = static_cast<StringAttribute*>(attr)->mValue;
	if(value == NULL)
		attrValue.clear();
	else
		attrValue.assign(value);
	return true;
}

bool Context::setDirectoryAttributeValue(size_t attrIndex, const wchar_t* value) {
	if(attrIndex >= mAttributes.size())
		return false;
	Attribute* attr = mAttributes[attrIndex];
	if((attr->mType & Attribute::ATTR_DIRECTORY) == 0)
		return false;
	std::wstring& attrValue = static_cast<StringAttribute*>(attr)->mValue;
	if(value == NULL)
		attrValue.clear();
	else
		attrValue.assign(value);
	return true;
}

bool Context::setFileAttributeValue(size_t attrIndex, const wchar_t* value) {
	if(attrIndex >= mAttributes.size())
		return false;
	Attribute* attr = mAttributes[attrIndex];
	if((attr->mType & Attribute::ATTR_FILE) == 0)
		return false;
	std::wstring& attrValue = static_cast<StringAttribute*>(attr)->mValue;
	if(value == NULL)
		attrValue.clear();
	else
		attrValue.assign(value);
	return true;
}

bool Context::setColorAttributeValue(size_t attrIndex, const wchar_t* value) {
	if(attrIndex >= mAttributes.size())
		return false;
	Attribute* attr = mAttributes[attrIndex];
	if((attr->mType & Attribute::ATTR_COLOR) == 0)
		return false;
	std::wstring& attrValue = static_cast<StringAttribute*>(attr)->mValue;
	if(value == NULL)
		attrValue.clear();
	else
		attrValue.assign(value);
	return true;
}

bool Context::setEnumAttributeValue(size_t attrIndex, uint32_t value) {
	if(attrIndex >= mAttributes.size())
		return false;
	Attribute* attr = mAttributes[attrIndex];
	if((attr->mType & Attribute::ATTR_ENUM) == 0)
		return false;
	static_cast<EnumAttribute*>(attr)->mIndex = value;
	return true;
}


// helper for merging adjacent coplanar triangles to a single polygon (currently does not support holes)
namespace {
	struct TriFace {
		uint32_t mIndices[3];			// vertex indices
		float mNormal[3];				// face normal
		uint32_t mAdjacentFace[3];		// [i]: face adjacent to edge v[i] -- v[(i+1)%3], with v[i] = vertices[mIndices[i]]
		bool mVisited;					// flag whether triangle has already been processed
	};

	uint64_t makeEdgeId(uint32_t a, uint32_t b) {
		return uint64_t(a) | (uint64_t(b) << 32);
	}

	uint32_t getFirstEdgeVertex(uint64_t id) {
		return uint32_t(id);
	}

	uint32_t getSecondEdgeVertex(uint64_t id) {
		return uint32_t(id >> 32);
	}

	uint32_t makeFaceId(uint32_t f, uint32_t e) {
		return (f << 2) | e;
	}

	uint32_t getFaceIndex(uint32_t id) {
		return id >> 2;
	}

	uint32_t getEdgeIndex(uint32_t id) {
		return id & 0x3;
	}

	void mergeAdjacentTriangles(const float* vertices, uint32_t numVertices, const uint32_t* triIndices, uint32_t numTriIndices, std::vector<uint32_t>& indices, std::vector<uint32_t>& faceCounts) {
		// create intermediate triangle representation and determine adjacency relationships
		typedef std::unordered_multimap<uint64_t, uint32_t> HalfEdgeFacesMap;
		HalfEdgeFacesMap halfEdgeFaces;
		std::vector<TriFace> triangles;
		const uint32_t numTris = numTriIndices / 3;
		triangles.resize(numTris);
		std::vector<TriFace>::iterator itTri = triangles.begin();
		uint32_t numFaces = 0;
		for(uint32_t i = 0; i < numTris; i++) {
			// set indices
			itTri->mIndices[0] = triIndices[i*3];
			itTri->mIndices[1] = triIndices[i*3 + 1];
			itTri->mIndices[2] = triIndices[i*3 + 2];
			itTri->mVisited = false;

			// compute face normal
			float u[3], v[3];
			for(size_t j = 0; j < 3; j++) {
				float f = vertices[itTri->mIndices[0]*3 + j];
				u[j] = vertices[itTri->mIndices[1]*3 + j] - f;
				v[j] = vertices[itTri->mIndices[2]*3 + j] - f;
			}
			float n[3];
			n[0] = u[1]*v[2] - u[2]*v[1];
			n[1] = u[2]*v[0] - u[0]*v[2];
			n[2] = u[0]*v[1] - u[1]*v[0];
			float l = n[0]*n[0] + n[1]*n[1] + n[2]*n[2];
			float s = 1.0f / std::sqrt(l);
			itTri->mNormal[0] = n[0] * s;
			itTri->mNormal[1] = n[1] * s;
			itTri->mNormal[2] = n[2] * s;

			// for each edge: look for coplanar, consistently oriented, adjacent face
			bool newFace = true;
			for(size_t e = 0; e < 3; e++) {
				std::pair<HalfEdgeFacesMap::iterator, HalfEdgeFacesMap::iterator> range = halfEdgeFaces.equal_range(makeEdgeId(itTri->mIndices[(e+1) % 3], itTri->mIndices[e]));
				uint32_t adjacentFace = uint32_t(-1);
				for(; range.first != range.second; ++range.first) {
					TriFace& otherTri = triangles[getFaceIndex(range.first->second)];
					if(otherTri.mAdjacentFace[getEdgeIndex(range.first->second)] != uint32_t(-1))
						continue;
					const float dot = itTri->mNormal[0] * otherTri.mNormal[0] + itTri->mNormal[1] * otherTri.mNormal[1] + itTri->mNormal[2] * otherTri.mNormal[2];
					if(dot >= 0.999f) {
						adjacentFace = range.first->second;
						otherTri.mAdjacentFace[getEdgeIndex(range.first->second)] = makeFaceId(i, e);
						newFace = false;
						break;
					}
				}
				itTri->mAdjacentFace[e] = adjacentFace;
				halfEdgeFaces.insert(std::make_pair(makeEdgeId(itTri->mIndices[e], itTri->mIndices[(e+1) % 3]), makeFaceId(i, e)));
			}
			if(newFace)
				numFaces++;
			++itTri;
		}

		indices.reserve(numTriIndices - (numTris - numFaces) * 2);
		faceCounts.reserve(numFaces);

		// extract polygons
		for(uint32_t t = 0; t < numTris; t++) {
			TriFace* tri = &triangles[t];
			if(tri->mVisited)
				continue;

			uint32_t numFaceIndices = 0;
			uint32_t firstVertex;
			uint32_t lastVertex;
			uint32_t lastEdge;

			// find first contour edge
			while(true) {
				tri->mVisited = true;
				for(uint32_t e = 0; e < 3; e++) {
					if(tri->mAdjacentFace[e] == uint32_t(-1)) {
						firstVertex = tri->mIndices[e];
						lastVertex = tri->mIndices[(e + 1) % 3];
						indices.push_back(firstVertex);
						indices.push_back(lastVertex);
						numFaceIndices += 2;
						lastEdge = e;
						goto foundFirstEdge;
					}
				}
				uint32_t e;
				for(e = 0; e < 3; e++) {
					TriFace* adjacentTri = &triangles[getFaceIndex(tri->mAdjacentFace[e])];
					if(adjacentTri->mVisited)
						continue;
					tri = adjacentTri;
				}
				if(e == 3)
					break;
			}
			continue;

	foundFirstEdge:
			// walk along the contour until returning to the first vertex
			while(true) {
				for(uint32_t e = (lastEdge + 1) % 3; e < 3; e++) {
					if(tri->mAdjacentFace[e] != uint32_t(-1))
						break;
					const uint32_t i = tri->mIndices[(e + 1) % 3];
					if(i == firstVertex)
						goto done;
					lastVertex = i;
					indices.push_back(lastVertex);
					numFaceIndices++;
					lastEdge = e;
				}

				const uint32_t adj = tri->mAdjacentFace[(lastEdge + 1) % 3];
				tri = &triangles[getFaceIndex(adj)];
				tri->mVisited = true;
				lastEdge = getEdgeIndex(adj);
			}

	done:	// add polygon
			faceCounts.push_back(numFaceIndices);
		}
	}
}

// generates leaf shapes for the given initial shape geometry (coplanar, adjacent triangles are initially merged to a single polygon)
bool Context::generate(const float* vertices, uint32_t numVertices, const uint32_t* triIndices, uint32_t numTriIndices, const wchar_t* specialMaterial) {
	clearMeshes();
	clearMaterials();

	// create attribute map reflecting the current attribute values
	ScopedObject<prt::AttributeMapBuilder> aBuilder(prt::AttributeMapBuilder::create());
	for(size_t i = 0; i < mAttributes.size(); i++) {
		const Attribute* attr = mAttributes[i];
		switch(attr->mType) {
			case Attribute::ATTR_BOOL:
				aBuilder->setBool(attr->mName.c_str(), static_cast<const BoolAttribute*>(attr)->mValue);
				break;
			case Attribute::ATTR_FLOAT:
				aBuilder->setFloat(attr->mName.c_str(), static_cast<const FloatAttribute*>(attr)->mValue);
				break;
			case Attribute::ATTR_STRING:
			case Attribute::ATTR_DIRECTORY:
			case Attribute::ATTR_FILE:
			case Attribute::ATTR_COLOR:
				aBuilder->setString(attr->mName.c_str(), static_cast<const StringAttribute*>(attr)->mValue.c_str());
				break;
			case Attribute::ATTR_ENUM_BOOL: {
				const EnumAttribute* enumAttr = static_cast<const EnumAttribute*>(attr);
				if(enumAttr->mIndex < enumAttr->mAnnotation->getNumArguments())
					aBuilder->setBool(attr->mName.c_str(), enumAttr->mAnnotation->getArgument(enumAttr->mIndex)->getBool());
				break;
			}
			case Attribute::ATTR_ENUM_FLOAT: {
				const EnumAttribute* enumAttr = static_cast<const EnumAttribute*>(attr);
				if(enumAttr->mIndex < enumAttr->mAnnotation->getNumArguments())
					aBuilder->setFloat(attr->mName.c_str(), enumAttr->mAnnotation->getArgument(enumAttr->mIndex)->getFloat());
				break;
			}
			case Attribute::ATTR_ENUM_STRING: {
				const EnumAttribute* enumAttr = static_cast<const EnumAttribute*>(attr);
				if(enumAttr->mIndex < enumAttr->mAnnotation->getNumArguments())
					aBuilder->setString(attr->mName.c_str(), enumAttr->mAnnotation->getArgument(enumAttr->mIndex)->getStr());
				break;
			}
		}
	}
	ScopedObject<const prt::AttributeMap> generateAttrs(aBuilder->createAttributeMap());

	// create initial shape
	ScopedObject<prt::InitialShapeBuilder> isb(prt::InitialShapeBuilder::create());
	std::vector<double> vertexCoords(numVertices * 3);
	for(size_t i = 0; i < numVertices; i++) {
		vertexCoords[3*i + 0] =  vertices[3*i + 0];
		vertexCoords[3*i + 1] =  vertices[3*i + 1];
		vertexCoords[3*i + 2] =  vertices[3*i + 2];
	}
	std::vector<uint32_t> indices;
	std::vector<uint32_t> faceCounts;
	mergeAdjacentTriangles(vertices, numVertices, triIndices, numTriIndices, indices, faceCounts);
	prt::Status status = isb->setGeometry(vertexCoords.data(), vertexCoords.size(), indices.data(), indices.size(), faceCounts.data(), faceCounts.size());
	if(status != prt::STATUS_OK) {
		std::string msg = "InitialShapeBuilder setGeometry failed status = ";
		msg += prt::getStatusDescription(status);
		postUnityLogMessage(std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(msg).c_str(), prt::LOG_ERROR);
		return false;
	}
	isb->setAttributes(mRuleFiles[mRuleFileIndex].c_str(), mStartRules[mStartRuleIndex]->getName(), isb->computeSeed(), L"", generateAttrs.get(), mResolveMap);
	ScopedObject<const prt::InitialShape> shape(isb->createInitialShapeAndReset());

	// generate, using the Unity encoder; the results are accumulated into mMeshes and mMaterials by the output handler
	const prt::InitialShape* initialShapes[] = { shape.get() };
	OutputHandler outputHandler(*this, specialMaterial);
	status = prt::generate(initialShapes, _countof(initialShapes), 0, &ENCODER_ID_UNITY, 1, &mUnityEncOpts, &outputHandler, theCache, NULL);
	if(status != prt::STATUS_OK) {
		std::string msg = "prt generate failed: ";
		msg += prt::getStatusDescription(status);
		postUnityLogMessage(std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(msg).c_str(), prt::LOG_ERROR);
		return false;
	}

	return true;
}


//////	Public member functions for retrieving the generate result

size_t Context::getMaterialCount() const {
	return mMaterials.size();
}

bool Context::getMaterial(size_t materialIndex, const wchar_t*& name, const float*& color, const wchar_t*& diffuseTexture) const {
	if(materialIndex >= mMaterials.size())
		return false;
	const Material* material = mMaterials[materialIndex];
	name = material->mName.c_str();
	color = material->mColor;
	diffuseTexture = material->mDiffuseTexture.c_str();
	return true;
}

size_t Context::getMeshCount() const {
	return mMeshes.size();
}

bool Context::getMesh(size_t meshIndex, const wchar_t*& name, uint32_t& numVertices, const float*& vertices, const float*& normals, const float*& texcoords) const {
	if(meshIndex >= mMeshes.size())
		return false;
	const Mesh* mesh = mMeshes[meshIndex];
	name = mesh->mName.c_str();
	numVertices = mesh->mVertices.size() / 3;
	vertices = mesh->mVertices.data();
	normals = mesh->mNormals.data();
	texcoords = mesh->mTexcoords.empty() ? NULL : mesh->mTexcoords.data();
	return true;
}

size_t Context::getSubMeshCount(size_t meshIndex) const {
	if(meshIndex < mMeshes.size())
		return mMeshes[meshIndex]->mSubMeshes.size();
	else
		return 0;
}

bool Context::getSubMesh(size_t meshIndex, size_t subMeshIndex, const uint32_t*& indices, uint32_t& numIndices, uint32_t& materialIndex) const {
	if(meshIndex >= mMeshes.size())
		return false;
	const Mesh* mesh = mMeshes[meshIndex];
	if(subMeshIndex >= mesh->mSubMeshes.size())
		return false;
	const Mesh::SubMesh& subMesh = mesh->mSubMeshes[subMeshIndex];
	indices = mesh->mIndices.data() + subMesh.mFirstIndex;
	numIndices = subMesh.mNumIndices;
	materialIndex = subMesh.mMaterial;
	return true;
}


// discards the result from the last generate (to free memory as soon as the result is no longer needed)
void Context::releaseMeshesAndMaterials() {
	clearMaterials();
	clearMeshes();
}
