#include "PRTModifierAction.h"

#include "PRTModifierCommand.h"
#include "node/MayaCallbacks.h"
#include "node/Utilities.h"

#include "prt/StringUtils.h"

#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnStringData.h>

#include <limits>
#ifdef _WIN32
#	include <Windows.h>
#else
#	include <dlfcn.h>
#endif

#define CHECK_STATUS(st) if ((st) != MS::kSuccess) { break; }
namespace {
	const wchar_t* ENC_MAYA = L"MayaEncoder";
	const wchar_t* ENC_ATTR = L"com.esri.prt.core.AttributeEvalEncoder";
	const char*    FILE_PREFIX = "file:///";
	const MString  NAME_GENERATE = "Generate_Model";

	const wchar_t* ANNOT_START_RULE = L"@StartRule";
	const wchar_t* ANNOT_RANGE = L"@Range";
	const wchar_t* ANNOT_COLOR = L"@Color";
	const wchar_t* ANNOT_DIR = L"@Directory";
	const wchar_t* ANNOT_FILE = L"@File";
	const wchar_t* NULL_KEY = L"#NULL#";
} // namespace

PRTModifierAction::PRTModifierAction()
{

	EncoderInfoUPtr encInfo(prt::createEncoderInfo(ENC_MAYA));
	const prt::AttributeMap* am;
	encInfo->createValidatedOptionsAndStates(nullptr, &am, nullptr);
	mMayaEncOpts.emplace_back(AttributeMapUPtr(am));

	const prt::AttributeMap* am2;
	EncoderInfoUPtr encInfoAttr(prt::createEncoderInfo(ENC_ATTR));
	encInfoAttr->createValidatedOptionsAndStates(nullptr, &am2, nullptr);
	mAttrEncOpts.emplace_back(AttributeMapUPtr(am2));
}

void PRTModifierAction::fillAttributesFromNode(const MObject& node) {
	MStatus           stat;
	const MFnDependencyNode fNode(node, &stat);
	MCHECK(stat);

	const unsigned int count = fNode.attributeCount(&stat);
	MCHECK(stat);

	AttributeMapBuilderUPtr aBuilder(prt::AttributeMapBuilder::create());

	for (unsigned int i = 0; i < count; i++) {
		const MObject attr = fNode.attribute(i, &stat);
		if (stat != MS::kSuccess) continue;

		const MPlug plug(node, attr);
		if (!(plug.isDynamic())) continue;

		const MString       briefName = plug.partialName();
		const std::wstring  name = mBriefName2prtAttr[briefName.asWChar()];

		if (attr.hasFn(MFn::kNumericAttribute)) {
			MFnNumericAttribute nAttr(attr);

			if (nAttr.unitType() == MFnNumericData::kBoolean) {
				bool b, db;
				nAttr.getDefault(db);
				MCHECK(plug.getValue(b));
				if (b != db)
					aBuilder->setBool(name.c_str(), b);
			}
			else if (nAttr.unitType() == MFnNumericData::kDouble) {
				double d, dd;
				nAttr.getDefault(dd);
				MCHECK(plug.getValue(d));
				if (d != dd)
					aBuilder->setFloat(name.c_str(), d);
			}
			else if (nAttr.isUsedAsColor()) {
				float r;
				float g;
				float b;

				nAttr.getDefault(r, g, b);
				wchar_t dcolor[] = L"#000000";
				prtu::toHex(dcolor, r, g, b);

				MObject rgb;
				MCHECK(plug.getValue(rgb));

				MFnNumericData fRGB(rgb);
				MCHECK(fRGB.getData(r, g, b));

				wchar_t color[] = L"#000000";
				prtu::toHex(color, r, g, b);

				if (std::wcscmp(dcolor, color))
					aBuilder->setString(name.c_str(), color);
			}

		}
		else if (attr.hasFn(MFn::kTypedAttribute)) {
			MFnTypedAttribute tAttr(attr);
			MString       s;
			MFnStringData dsd;
			MObject       dso = dsd.create(&stat);
			MCHECK(stat);
			MCHECK(tAttr.getDefault(dso));

			MFnStringData fDs(dso, &stat);
			MCHECK(stat);

			MCHECK(plug.getValue(s));
			if (s != fDs.string(&stat)) {
				MCHECK(stat);
				aBuilder->setString(name.c_str(), s.asWChar());
			}
		}
		else if (attr.hasFn(MFn::kEnumAttribute)) {
			MFnEnumAttribute eAttr(attr);

			short di;
			short i;
			MCHECK(eAttr.getDefault(di));
			MCHECK(plug.getValue(i));
			if (i != di)
				aBuilder->setString(name.c_str(), eAttr.fieldName(i).asWChar());
		}
	}

	mGenerateAttrs.reset(aBuilder->createAttributeMap());
}

// Sets the mesh object for the action  to operate on
void PRTModifierAction::setMesh(MObject& _inMesh, MObject& _outMesh)
{
	inMesh = _inMesh;
	outMesh = _outMesh;
}

// plugin root = location of prt4maya shared library
const std::string& PRTModifierAction::getPluginRoot() {
	static std::string* rootPath = nullptr;
	if (rootPath == nullptr) {
#ifdef _MSC_VER
		char dllPath[_MAX_PATH];
		char drive[8];
		char dir[_MAX_PATH];
		HMODULE hModule = 0;

		GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)PRTModifierCommand::creator, &hModule);
		const DWORD res = ::GetModuleFileName(hModule, dllPath, _MAX_PATH);
		if (res == 0) {
			// TODO DWORD e = ::GetLastError();
			throw std::runtime_error("failed to get plugin location");
		}

		_splitpath_s(dllPath, drive, 8, dir, _MAX_PATH, 0, 0, 0, 0);
		rootPath = new std::string(drive);
		rootPath->append(dir);
#else
		Dl_info dl_info;
		dladdr((void *)getPluginRoot, &dl_info);
		const std::string tmp(dl_info.dli_fname);
		rootPath = new std::string(tmp.substr(0, tmp.find_last_of(prtu::getDirSeparator<char>()))); // accepted mem leak
#endif
		if (*rootPath->rbegin() != prtu::getDirSeparator<char>())
			rootPath->append(1, prtu::getDirSeparator<char>());
	}
	return *rootPath;
}



MStatus PRTModifierAction::updateRuleFiles(MObject& node, const MString& rulePkg) {
	mRulePkg = rulePkg;
	MStatus  stat;

	const std::string utf8Path(mRulePkg.asUTF8());
	std::vector<char> percentEncodedPath(2 * utf8Path.size() + 1);
	size_t len = percentEncodedPath.size();
	prt::StringUtils::percentEncode(utf8Path.c_str(), &percentEncodedPath[0], &len);
	if (len > percentEncodedPath.size() + 1) {
		percentEncodedPath.resize(len);
		prt::StringUtils::percentEncode(utf8Path.c_str(), &percentEncodedPath[0], &len);
	}

	std::string uri(FILE_PREFIX);
	uri.append(&percentEncodedPath[0]);

	mEnums.clear();
	mRuleFile.clear();
	mStartRule.clear();

	MString unpackDir = MGlobal::executeCommandStringResult("workspace -q -fullName");
	unpackDir += "/assets";
	prt::Status resolveMapStatus = prt::STATUS_UNSPECIFIED_ERROR;

	std::wstring utf16URI;
	utf16URI.resize(uri.size() + 1);
	len = utf16URI.size();
	if (prt::StringUtils::toUTF16FromUTF8(uri.c_str(), &utf16URI[0], &len)) {
		utf16URI.resize(len);
		prt::StringUtils::toUTF16FromUTF8(uri.c_str(), &utf16URI[0], &len);
	}

	mResolveMap.reset(prt::createResolveMap(utf16URI.c_str(), unpackDir.asWChar(), &resolveMapStatus));
	if (resolveMapStatus == prt::STATUS_OK) {
		size_t nKeys;
		const wchar_t * const* keys = mResolveMap->getKeys(&nKeys);
		std::wstring sCGB(L".cgb");
		for (size_t k = 0; k < nKeys; k++) {
			const std::wstring key = std::wstring(keys[k]);
			if (std::equal(sCGB.rbegin(), sCGB.rend(), key.rbegin())) {
				mRuleFile = key;
				break;
			}
		}

		if (mRuleFile.length() > 0)
		{
			const prt::RuleFileInfo::Entry* startRule = nullptr;

			prt::Status infoStatus = prt::STATUS_UNSPECIFIED_ERROR;
			RuleFileInfoUPtr info(prt::createRuleFileInfo(mResolveMap->getString(mRuleFile.c_str()), nullptr, &infoStatus));
			if (infoStatus == prt::STATUS_OK) {
				for (size_t r = 0; r < info->getNumRules(); r++) {
					if (info->getRule(r)->getNumParameters() > 0)
						continue;
					for (size_t a = 0; a < info->getRule(r)->getNumAnnotations(); a++) {
						if (!(std::wcscmp(info->getRule(r)->getAnnotation(a)->getName(), ANNOT_START_RULE))) {
							startRule = info->getRule(r);
							break;
						}
					}
				}
			}
			if (startRule) {
				mStartRule = startRule->getName();

				AttributeMapBuilderUPtr aBuilder(prt::AttributeMapBuilder::create());
				if (node != MObject::kNullObj)
					createNodeAttributes(node, mRuleFile, mStartRule, aBuilder.get(), info.get());
				mGenerateAttrs.reset(aBuilder->createAttributeMap());
			}

		}

	}
	else {
		mResolveMap = nullptr;
	}

	return MS::kSuccess;
}


MStatus PRTModifierAction::doIt()
{
	MStatus status;

	// Get access to the mesh's function set
	const MFnMesh iMeshFn(inMesh);

	MFloatPointArray vertices;
	MIntArray        pcounts;
	MIntArray        pconnect;

	iMeshFn.getPoints(vertices);
	iMeshFn.getVertices(pcounts, pconnect);

	std::vector<double> va(vertices.length()*3);
	for (int i = static_cast<int>(vertices.length()); --i >= 0;) {
		va[i * 3 + 0] = vertices[i].x;
		va[i * 3 + 1] = vertices[i].y;
		va[i * 3 + 2] = vertices[i].z;
	}
	
	std::vector <uint32_t> ia(pconnect.length());
	pconnect.get((int*)ia.data());
	std::vector <uint32_t> ca(pcounts.length());
	pcounts.get((int*)ca.data());

	std::unique_ptr<MayaCallbacks> outputHandler(new MayaCallbacks(inMesh, outMesh));

	InitialShapeBuilderUPtr isb(prt::InitialShapeBuilder::create());
	prt::Status setGeoStatus = isb->setGeometry(
		va.data(),
		va.size(),
		ia.data(),
		ia.size(),
		ca.data(),
		ca.size()
	);
	if (setGeoStatus != prt::STATUS_OK)
		std::cerr << "InitialShapeBuilder setGeometry failed status = " << prt::getStatusDescription(setGeoStatus) << std::endl;

	isb->setAttributes(
		mRuleFile.c_str(),
		mStartRule.c_str(),
		prtu::computeSeed(vertices),
		L"",
		mGenerateAttrs.get(),
		mResolveMap.get()
	);

	std::unique_ptr <const prt::InitialShape, PRTDestroyer> shape(isb->createInitialShapeAndReset());
	AttributeMapNOPtrVector encOpts = prtu::toPtrVec(mMayaEncOpts);
	InitialShapeNOPtrVector shapes = { shape.get() };
	const prt::Status        generateStatus = prt::generate(shapes.data(), shapes.size(), 0, &ENC_MAYA, encOpts.size(), encOpts.data(), outputHandler.get(), PRTModifierAction::theCache, 0);
	if (generateStatus != prt::STATUS_OK)
		std::cerr << "prt generate failed: " << prt::getStatusDescription(generateStatus) << std::endl;

	return status;
}

namespace UnitQuad {
	const double   vertices[] = { 0, 0, 0,  0, 0, 1,  1, 0, 1,  1, 0, 0 };
	const size_t   vertexCount = 12;
	const uint32_t indices[] = { 0, 1, 2, 3 };
	const size_t   indexCount = 4;
	const uint32_t faceCounts[] = { 4 };
	const size_t   faceCountsCount = 1;
	const int32_t  seed = prtu::computeSeed(vertices, vertexCount);
}

MStatus PRTModifierAction::createNodeAttributes(MObject& nodeObj, const std::wstring & ruleFile, const std::wstring & startRule, prt::AttributeMapBuilder* aBuilder, const prt::RuleFileInfo* info) {
	MStatus           stat;
	MStatus           stat2;
	MFnNumericData    numericData;
	MFnTypedAttribute tAttr;
	MFnStringData     attrDefaultStr;
	MString           dummy;

	MFnDependencyNode node(nodeObj, &stat);
	MCHECK(stat);

	std::unique_ptr<MayaCallbacks> outputHandler(new MayaCallbacks(MObject::kNullObj, MObject::kNullObj));
	AttributeMapUPtr attrs(aBuilder->createAttributeMap());

	InitialShapeBuilderUPtr isb(prt::InitialShapeBuilder::create());
	isb->setGeometry(
		UnitQuad::vertices,
		UnitQuad::vertexCount,
		UnitQuad::indices,
		UnitQuad::indexCount,
		UnitQuad::faceCounts,
		UnitQuad::faceCountsCount
	);
	isb->setAttributes(
		ruleFile.c_str(),
		startRule.c_str(),
		UnitQuad::seed,
		L"",
		attrs.get(),
		mResolveMap.get()
	);
	std::unique_ptr<const prt::InitialShape, PRTDestroyer> shape(isb->createInitialShapeAndReset());
	InitialShapeNOPtrVector shapes = { shape.get() };
	AttributeMapNOPtrVector encOpts = prtu::toPtrVec(mAttrEncOpts);
	prt::generate(shapes.data(), shapes.size(), nullptr, &ENC_ATTR, encOpts.size(), encOpts.data(), outputHandler.get(), theCache, nullptr);

	const std::map<std::wstring, MayaCallbacks::AttributeHolder>& evalAttrs = outputHandler->getAttrs();

	mBriefName2prtAttr[NAME_GENERATE.asWChar()] = NAME_GENERATE.asWChar();

	for (size_t i = 0; i < info->getNumAttributes(); i++) {


		const MString name = MString(info->getAttribute(i)->getName());
		MObject attr;

		if (info->getAttribute(i)->getNumParameters() != 0) continue;

		mBriefName2prtAttr[prtu::toCleanId(name).asWChar()] = name.asWChar();

		switch (info->getAttribute(i)->getReturnType()) {
		case prt::AAT_BOOL: {
			const prt::Annotation* enumAnnotation = nullptr;
			for (size_t a = 0; a < info->getAttribute(i)->getNumAnnotations(); a++) {
				const prt::Annotation* an = info->getAttribute(i)->getAnnotation(a);
				if (!(std::wcscmp(an->getName(), ANNOT_RANGE)))
					enumAnnotation = an;
			}
			const bool value = evalAttrs.find(name.asWChar())->second.mBool;
			if (enumAnnotation) {
				mEnums.emplace_front();
				MCHECK(addEnumParameter(enumAnnotation, node, attr, name, value, mEnums.front()));
			}
			else {
				MCHECK(addBoolParameter(node, attr, name, value));
			}
			break;
		}
		case prt::AAT_FLOAT: {
			double min = std::numeric_limits<double>::quiet_NaN();
			double max = std::numeric_limits<double>::quiet_NaN();
			const prt::Annotation* enumAnnotation = nullptr;
			for (size_t a = 0; a < info->getAttribute(i)->getNumAnnotations(); a++) {
				const prt::Annotation* an = info->getAttribute(i)->getAnnotation(a);
				if (!(std::wcscmp(an->getName(), ANNOT_RANGE))) {
					if (an->getNumArguments() == 2 && an->getArgument(0)->getType() == prt::AAT_FLOAT && an->getArgument(1)->getType() == prt::AAT_FLOAT) {
						min = an->getArgument(0)->getFloat();
						max = an->getArgument(1)->getFloat();
					}
					else
						enumAnnotation = an;
				}
			}

			const double value = evalAttrs.find(name.asWChar())->second.mFloat;

			if (enumAnnotation) {
				mEnums.emplace_front();
				MCHECK(addEnumParameter(enumAnnotation, node, attr, name, value, mEnums.front()));
			}
			else {
				MCHECK(addFloatParameter(node, attr, name, value, min, max));
			}
			break;
		}
		case prt::AAT_STR: {
			MString exts;
			bool    asFile = false;
			bool    asColor = false;
			const prt::Annotation* enumAnnotation = nullptr;
			for (size_t a = 0; a < info->getAttribute(i)->getNumAnnotations(); a++) {
				const prt::Annotation* an = info->getAttribute(i)->getAnnotation(a);
				if (!(std::wcscmp(an->getName(), ANNOT_RANGE)))
					enumAnnotation = an;
				else if (!(std::wcscmp(an->getName(), ANNOT_COLOR)))
					asColor = true;
				else if (!(std::wcscmp(an->getName(), ANNOT_DIR))) {
					exts = MString(an->getName());
					asFile = true;
				}
				else if (!(std::wcscmp(an->getName(), ANNOT_FILE))) {
					asFile = true;
					for (size_t arg = 0; arg < an->getNumArguments(); arg++) {
						if (an->getArgument(arg)->getType() == prt::AAT_STR) {
							exts += MString(an->getArgument(arg)->getStr());
							exts += " (*.";
							exts += MString(an->getArgument(arg)->getStr());
							exts += ");;";
						}
					}
					exts += "All Files (*.*)";
				}
			}

			const std::wstring value = evalAttrs.find(name.asWChar())->second.mString;
			const MString mvalue(value.c_str());
			if (!(asColor) && mvalue.length() == 7 && value[0] == L'#')
				asColor = true;

			if (enumAnnotation) {
				mEnums.emplace_front();
				MCHECK(addEnumParameter(enumAnnotation, node, attr, name, mvalue, mEnums.front()));
			}
			else if (asFile) {
				MCHECK(addFileParameter(node, attr, name, mvalue, exts));
			}
			else if (asColor) {
				MCHECK(addColorParameter(node, attr, name, mvalue));
			}
			else {
				MCHECK(addStrParameter(node, attr, name, mvalue));
			}
			break;
		}
		default:
			break;
		}
	}

	return MS::kSuccess;
}


MStatus PRTModifierEnum::fill(const prt::Annotation* annot) {
	if (annot) {
		MStatus stat;
		for (size_t arg = 0; arg < annot->getNumArguments(); arg++) {
			const wchar_t* key = annot->getArgument(arg)->getKey();
			if (!(std::wcscmp(key, NULL_KEY)))
				key = annot->getArgument(arg)->getStr();
			mKeys.append(MString(annot->getArgument(arg)->getKey()));
			switch (annot->getArgument(arg)->getType()) {
			case prt::AAT_BOOL:
				MCHECK(mAttr.addField(MString(key), mBVals.length()));
				mBVals.append(annot->getArgument(arg)->getBool());
				mFVals.append(std::numeric_limits<double>::quiet_NaN());
				mSVals.append("");
				break;
			case prt::AAT_FLOAT:
				MCHECK(mAttr.addField(MString(key), mFVals.length()));
				mBVals.append(false);
				mFVals.append(annot->getArgument(arg)->getFloat());
				mSVals.append("");
				break;
			case prt::AAT_STR:
				MCHECK(mAttr.addField(MString(key), mSVals.length()));
				mBVals.append(false);
				mFVals.append(std::numeric_limits<double>::quiet_NaN());
				mSVals.append(MString(annot->getArgument(arg)->getStr()));
				break;
			default:
				break;
			}
		}
	}
	else {
		for (unsigned int i = 0; i < mKeys.length(); i++)
			mAttr.addField(mKeys[i], (short)i);
	}
	return MS::kSuccess;
}

template<typename T> T PRTModifierAction::getPlugValueAndRemoveAttr(MFnDependencyNode & node, const MString & briefName, const T& defaultValue) {
	T plugValue = defaultValue;
	if (node.hasAttribute(briefName)) {
		const MPlug plug = node.findPlug(briefName);
		if (plug.isDynamic())
		{
			T d;
			if (plug.getValue(d) == MS::kSuccess)
				plugValue = d;
		}
		node.removeAttribute(node.attribute(briefName));
	}
	return plugValue;
}

MString PRTModifierAction::longName(const MString& attrName) {
	return prtu::toCleanId(attrName.substring(attrName.index('$') + 1, attrName.length()));
}

MString PRTModifierAction::briefName(const MString & attrName) {
	return prtu::toCleanId(attrName);
}

MStatus PRTModifierAction::addParameter(MFnDependencyNode & node, MObject & attr, MFnAttribute& tAttr) {
	if (!(node.hasAttribute(tAttr.shortName()))) {
		MCHECK(tAttr.setKeyable(true));
		MCHECK(tAttr.setHidden(false));
		MCHECK(tAttr.setStorable(true));
		MCHECK(node.addAttribute(attr));
	}
	return MS::kSuccess;
}

MStatus PRTModifierAction::addBoolParameter(MFnDependencyNode & node, MObject & attr, const MString & name, bool defaultValue) {
	MStatus stat;
	MFnNumericAttribute nAttr;

	bool plugValue = getPlugValueAndRemoveAttr(node, briefName(name), defaultValue);
	attr = nAttr.create(longName(name), briefName(name), MFnNumericData::kBoolean, defaultValue, &stat);
	if (stat != MS::kSuccess) throw stat;

	MCHECK(addParameter(node, attr, nAttr));

	MPlug plug(node.object(), attr);
	MCHECK(plug.setValue(plugValue));

	return MS::kSuccess;
}

MStatus PRTModifierAction::addFloatParameter(MFnDependencyNode & node, MObject & attr, const MString & name, double defaultValue, double min, double max) {
	MStatus stat;
	MFnNumericAttribute nAttr;

	double plugValue = getPlugValueAndRemoveAttr(node, briefName(name), defaultValue);
	attr = nAttr.create(longName(name), briefName(name), MFnNumericData::kDouble, defaultValue, &stat);
	if (stat != MS::kSuccess)
		throw stat;

	if (!prtu::isnan(min)) {
		MCHECK(nAttr.setMin(min));
	}

	if (!prtu::isnan(max)) {
		MCHECK(nAttr.setMax(max));
	}

	MCHECK(addParameter(node, attr, nAttr));

	MPlug plug(node.object(), attr);
	MCHECK(plug.setValue(plugValue));

	return MS::kSuccess;
}

MStatus PRTModifierAction::addEnumParameter(const prt::Annotation* annot, MFnDependencyNode & node, MObject & attr, const MString & name, bool defaultValue, PRTModifierEnum & e) {
	short idx = 0;
	for (int i = static_cast<int>(e.mBVals.length()); --i >= 0;) {
		if ((e.mBVals[i] != 0) == defaultValue) {
			idx = static_cast<short>(i);
			break;
		}
	}

	return addEnumParameter(annot, node, attr, name, idx, e);
}

MStatus PRTModifierAction::addEnumParameter(const prt::Annotation* annot, MFnDependencyNode & node, MObject & attr, const MString & name, double defaultValue, PRTModifierEnum & e) {
	short idx = 0;
	for (int i = static_cast<int>(e.mFVals.length()); --i >= 0;) {
		if (e.mFVals[i] == defaultValue) {
			idx = static_cast<short>(i);
			break;
		}
	}

	return addEnumParameter(annot, node, attr, name, idx, e);
}

MStatus PRTModifierAction::addEnumParameter(const prt::Annotation* annot, MFnDependencyNode & node, MObject & attr, const MString & name, MString defaultValue, PRTModifierEnum & e) {
	short idx = 0;
	for (int i = static_cast<int>(e.mSVals.length()); --i >= 0;) {
		if (e.mSVals[i] == defaultValue) {
			idx = static_cast<short>(i);
			break;
		}
	}

	return addEnumParameter(annot, node, attr, name, idx, e);
}

MStatus PRTModifierAction::addEnumParameter(const prt::Annotation* annot, MFnDependencyNode & node, MObject & attr, const MString & name, short defaultValue, PRTModifierEnum & e) {
	MStatus stat;

	short plugValue = getPlugValueAndRemoveAttr(node, briefName(name), defaultValue);
	attr = e.mAttr.create(longName(name), briefName(name), defaultValue, &stat);
	MCHECK(stat);

	MCHECK(e.fill(annot));

	MCHECK(addParameter(node, attr, e.mAttr));

	MPlug plug(node.object(), attr);
	MCHECK(plug.setValue(plugValue));

	return MS::kSuccess;
}

MStatus PRTModifierAction::addFileParameter(MFnDependencyNode & node, MObject & attr, const MString & name, const MString & defaultValue, const MString & exts) {
	MStatus           stat;
	MStatus           stat2;
	MFnStringData     stringData;
	MFnTypedAttribute sAttr;

	MString plugValue = getPlugValueAndRemoveAttr(node, briefName(name), defaultValue);
	attr = sAttr.create(longName(name), briefName(name), MFnData::kString, stringData.create("", &stat2), &stat);
	MCHECK(stat2);
	MCHECK(stat);
	MCHECK(sAttr.setUsedAsFilename(true));
	MCHECK(addParameter(node, attr, sAttr));
	MCHECK(sAttr.setNiceNameOverride(exts));

	MPlug plug(node.object(), attr);
	MCHECK(plug.setValue(plugValue));

	return MS::kSuccess;
}

MStatus PRTModifierAction::addColorParameter(MFnDependencyNode & node, MObject & attr, const MString & name, const MString & defaultValue) {
	MStatus             stat;
	MFnNumericAttribute nAttr;

	const wchar_t* s = defaultValue.asWChar();
	double r = 0.0;
	double g = 0.0;
	double b = 0.0;

	if (s[0] == '#' && wcslen(s) >= 7) {
		r = static_cast<double>((prtu::fromHex(s[1]) << 4) + prtu::fromHex(s[2])) / 255.0;
		g = static_cast<double>((prtu::fromHex(s[3]) << 4) + prtu::fromHex(s[4])) / 255.0;
		b = static_cast<double>((prtu::fromHex(s[5]) << 4) + prtu::fromHex(s[6])) / 255.0;
	}

	MFnNumericData fnData;
	MObject        rgb = fnData.create(MFnNumericData::k3Double, &stat);
	MCHECK(stat);
	fnData.setData(r, g, b);

	MObject plugValue = getPlugValueAndRemoveAttr(node, briefName(name), rgb);
	attr = nAttr.createColor(longName(name), briefName(name), &stat);
	nAttr.setDefault(r, g, b);

	MCHECK(stat);
	MCHECK(addParameter(node, attr, nAttr));

	MPlug plug(node.object(), attr);
	MCHECK(plug.setValue(plugValue));

	return MS::kSuccess;
}

MStatus PRTModifierAction::addStrParameter(MFnDependencyNode & node, MObject & attr, const MString & name, const MString & defaultValue) {
	MStatus           stat;
	MStatus           stat2;
	MFnStringData     stringData;
	MFnTypedAttribute sAttr;

	MString plugValue = getPlugValueAndRemoveAttr(node, briefName(name), defaultValue);
	attr = sAttr.create(longName(name), briefName(name), MFnData::kString, stringData.create(defaultValue, &stat2), &stat);
	MCHECK(stat2);
	MCHECK(stat);
	MCHECK(addParameter(node, attr, sAttr));

	MPlug plug(node.object(), attr);
	MCHECK(plug.setValue(plugValue));

	return MS::kSuccess;
}



void PRTModifierEnum::add(const MString & key, const MString & value) {
	mKeys.append(key);
	mBVals.append(false);
	mFVals.append(std::numeric_limits<double>::quiet_NaN());
	mSVals.append(value);
}

// statics
prt::ConsoleLogHandler* PRTModifierAction::theLogHandler = nullptr;
prt::FileLogHandler*    PRTModifierAction::theFileLogHandler = nullptr;
const prt::Object*      PRTModifierAction::thePRT = nullptr;
prt::CacheObject*       PRTModifierAction::theCache = nullptr;