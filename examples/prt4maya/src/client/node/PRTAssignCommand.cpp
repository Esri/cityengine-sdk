/**
 * Esri CityEngine SDK Maya Plugin Example
 *
 * This example demonstrates the main functionality of the Procedural Runtime API.
 * Esri R&D Center Zurich, Switzerland
 *
 * See http://github.com/ArcGIS/esri-cityengine-sdk for instructions.
 */

#define MNoPluginEntry
#define MNoVersionString

#include "node/PRTNode.h"
#include "node/Utilities.h"
#include "node/MayaCallbacks.h"

#include "prt/Status.h"
#include "prt/StringUtils.h"

#include <cmath>
#include <limits>


namespace {
const wchar_t* ANNOT_START_RULE = L"@StartRule";
const wchar_t* ANNOT_RANGE      = L"@Range";
const wchar_t* ANNOT_COLOR      = L"@Color";
const wchar_t* ANNOT_DIR        = L"@Directory";
const wchar_t* ANNOT_FILE       = L"@File";
const wchar_t* NULL_KEY         = L"#NULL#";
} // namespace


inline MString & PRTAssignCommand::getStringParameter(MObject & node, MObject & attr, MString & value) {
	const MPlug plug(node, attr);
	plug.getValue(value);
	return value;
}

inline MStatus PRTAssignCommand::setStringParameter(MObject & node, MObject & attr, MString & value) {
	MPlug plug(node, attr);
	return plug.setValue(value);
}

MStatus PRTAssignCommand::addParameter(MFnDependencyNode & node, MObject & attr, MFnAttribute& tAttr) {
	if(!(node.hasAttribute(tAttr.shortName()))) {
		MCHECK(tAttr.setKeyable (true));
		MCHECK(tAttr.setHidden(false));
		MCHECK(tAttr.setStorable(true));
		MCHECK(node.addAttribute(attr));
	}
	return MS::kSuccess;
}

template<typename T> T PRTAssignCommand::getPlugValueAndRemoveAttr(MFnDependencyNode & node, const MString & briefName, const T& defaultValue) {
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

template double PRTAssignCommand::getPlugValueAndRemoveAttr<double>(MFnDependencyNode&, const MString &, const double&);
template bool PRTAssignCommand::getPlugValueAndRemoveAttr<bool>(MFnDependencyNode&, const MString &, const bool&);
template short PRTAssignCommand::getPlugValueAndRemoveAttr<short>(MFnDependencyNode&, const MString &, const short&);
template MString PRTAssignCommand::getPlugValueAndRemoveAttr<MString>(MFnDependencyNode&, const MString &, const MString&);

MString PRTAssignCommand::longName(const MString& attrName) {
	return prtu::toCleanId(attrName.substring(attrName.index('$') + 1, attrName.length()));
}

MString PRTAssignCommand::briefName(const MString & attrName) {
	return prtu::toCleanId(attrName);
}

MStatus PRTAssignCommand::addBoolParameter(MFnDependencyNode & node, MObject & attr, const MString & name, bool defaultValue) {
	MStatus stat;
	MFnNumericAttribute nAttr;

    bool plugValue = getPlugValueAndRemoveAttr(node, briefName(name), defaultValue);
	attr = nAttr.create(longName(name), briefName(name), MFnNumericData::kBoolean, defaultValue, &stat);
	if ( stat != MS::kSuccess ) throw stat;

	MCHECK(addParameter(node, attr, nAttr));

	MPlug plug(node.object(), attr);
	MCHECK(plug.setValue(plugValue));

	return MS::kSuccess;
}

MStatus PRTAssignCommand::addFloatParameter(MFnDependencyNode & node, MObject & attr, const MString & name, double defaultValue, double min, double max) {
	MStatus stat;
	MFnNumericAttribute nAttr;

    double plugValue = getPlugValueAndRemoveAttr(node, briefName(name), defaultValue);
	attr = nAttr.create(longName(name), briefName(name), MFnNumericData::kDouble, defaultValue, &stat );
	if (stat != MS::kSuccess)
		throw stat;

	if (!prtu::isnan(min)) {
		MCHECK(nAttr.setMin(min));
	}

	if (!prtu::isnan(max)) {
		MCHECK(nAttr.setMax( max ));
	}

	MCHECK(addParameter(node, attr, nAttr));

	MPlug plug(node.object(), attr);
	MCHECK(plug.setValue(plugValue));

	return MS::kSuccess;
}

MStatus PRTAssignCommand::addEnumParameter(MFnDependencyNode & node, MObject & attr, const MString & name, bool defaultValue, PRTEnum * e) {
	short idx = 0;
	for(int i = static_cast<int>(e->mBVals.length()); --i >= 0;) {
		if((e->mBVals[i] != 0) == defaultValue) {
			idx = static_cast<short>(i);
			break;
		}
	}

	return addEnumParameter(node, attr, name, idx, e);
}

MStatus PRTAssignCommand::addEnumParameter(MFnDependencyNode & node, MObject & attr, const MString & name, double defaultValue, PRTEnum * e) {
	short idx = 0;
	for(int i = static_cast<int>(e->mFVals.length()); --i >= 0;) {
		if(e->mFVals[i] == defaultValue) {
			idx = static_cast<short>(i);
			break;
		}
	}

	return addEnumParameter(node, attr, name, idx, e);
}

MStatus PRTAssignCommand::addEnumParameter(MFnDependencyNode & node, MObject & attr, const MString & name, MString defaultValue, PRTEnum * e) {
	short idx = 0;
	for(int i = static_cast<int>(e->mSVals.length()); --i >= 0;) {
		if(e->mSVals[i] == defaultValue) {
			idx = static_cast<short>(i);
			break;
		}
	}

	return addEnumParameter(node, attr, name, idx, e);
}

MStatus PRTAssignCommand::addEnumParameter(MFnDependencyNode & node, MObject & attr, const MString & name, short defaultValue, PRTEnum * e) {
	MStatus stat;

    short plugValue = getPlugValueAndRemoveAttr(node, briefName(name), defaultValue);
	attr = e->mAttr.create(longName(name), briefName(name), defaultValue, &stat);
	MCHECK(stat);

	MCHECK(e->fill());

	MCHECK(addParameter(node, attr, e->mAttr));

	MPlug plug(node.object(), attr);
	MCHECK(plug.setValue(plugValue));

	return MS::kSuccess;
}

MStatus PRTAssignCommand::addFileParameter(MFnDependencyNode & node, MObject & attr, const MString & name, const MString & defaultValue, const MString & exts ) {
	MStatus           stat;
	MStatus           stat2;
	MFnStringData     stringData;
	MFnTypedAttribute sAttr;

    MString plugValue = getPlugValueAndRemoveAttr(node, briefName(name), defaultValue);
	attr = sAttr.create(longName(name), briefName(name), MFnData::kString, stringData.create("", &stat2), &stat );
	MCHECK(stat2);
	MCHECK(stat);
	MCHECK(sAttr.setUsedAsFilename(true));
	MCHECK(addParameter(node, attr, sAttr));
	MCHECK(sAttr.setNiceNameOverride(exts));

	MPlug plug(node.object(), attr);
	MCHECK(plug.setValue(plugValue));

	return MS::kSuccess;
}

MStatus PRTAssignCommand::addColorParameter(MFnDependencyNode & node, MObject & attr, const MString & name, const MString & defaultValue) {
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

MStatus PRTAssignCommand::addStrParameter(MFnDependencyNode & node, MObject & attr, const MString & name, const MString & defaultValue ) {
	MStatus           stat;
	MStatus           stat2;
	MFnStringData     stringData;
	MFnTypedAttribute sAttr;

    MString plugValue = getPlugValueAndRemoveAttr(node, briefName(name), defaultValue);
	attr = sAttr.create(longName(name), briefName(name), MFnData::kString, stringData.create(defaultValue, &stat2), &stat );
	MCHECK(stat2);
	MCHECK(stat);
	MCHECK(addParameter(node, attr, sAttr));

	MPlug plug(node.object(), attr);
	MCHECK(plug.setValue(plugValue));

	return MS::kSuccess;
}

MStatus PRTAssignCommand::updateRuleFiles(MFnDependencyNode & node, const MString & rulePkg) {
	PRTNode* prtNode = (PRTNode*)node.userNode();
	MStatus  stat;

	const std::string utf8Path(rulePkg.asUTF8());
	std::vector<char> percentEncodedPath(2*utf8Path.size()+1);
	size_t len = percentEncodedPath.size();
	prt::StringUtils::percentEncode(utf8Path.c_str(), &percentEncodedPath[0], &len);
	if(len > percentEncodedPath.size()+1){
		percentEncodedPath.resize(len);
		prt::StringUtils::percentEncode(utf8Path.c_str(), &percentEncodedPath[0], &len);
	}

	std::string uri(FILE_PREFIX);
	uri.append(&percentEncodedPath[0]);

	const unsigned int count = node.attributeCount(&stat);
	MCHECK(stat);

	MObjectArray attrs;

	for(unsigned int i = 0; i < count; i++) {
		const MObject attr = node.attribute(i, &stat);
		if(stat != MS::kSuccess) continue;
		attrs.append(attr);
	}

	for(unsigned int i = 0; i < attrs.length(); i++) {
		const MPlug   plug(node.object(), attrs[i]);
		const MString name = plug.partialName();

		if(prtNode->mBriefName2prtAttr.count(name.asWChar()))
			node.removeAttribute(attrs[i]);
	}
	prtNode->destroyEnums();

	prtNode->mRuleFile.clear();
	prtNode->mStartRule.clear();

	MString unpackDir = MGlobal::executeCommandStringResult("workspace -q -fullName");
	unpackDir += "/assets";
	prt::Status resolveMapStatus = prt::STATUS_UNSPECIFIED_ERROR;

	std::wstring utf16URI;
	utf16URI.resize(uri.size()+1);
	len = utf16URI.size();
	if(prt::StringUtils::toUTF16FromUTF8(uri.c_str(), &utf16URI[0], &len)) {
		utf16URI.resize(len);
		prt::StringUtils::toUTF16FromUTF8(uri.c_str(), &utf16URI[0], &len);
	}

	prtNode->mResolveMap = prt::createResolveMap(utf16URI.c_str(), unpackDir.asWChar(), &resolveMapStatus);
	if(resolveMapStatus == prt::STATUS_OK) {
		size_t nKeys;
		const wchar_t * const* keys = prtNode->mResolveMap->getKeys(&nKeys);
		std::wstring sCGB(L".cgb");
		for(size_t k = 0; k < nKeys; k++) {
			const std::wstring key = std::wstring(keys[k]);
			if(std::equal(sCGB.rbegin(), sCGB.rend(), key.rbegin())) {
				prtNode->mRuleFile = key;
				break;
			}
		}
	} else {
		prtNode->mResolveMap = nullptr;
	}

	if(prtNode->mRuleFile.length() > 0)
		updateStartRules(node);

	return MS::kSuccess;
}

MStatus PRTAssignCommand::updateStartRules(MFnDependencyNode & node) {
	PRTNode* prtNode = (PRTNode*)node.userNode();

	const prt::RuleFileInfo::Entry* startRule = nullptr;

	prt::Status infoStatus = prt::STATUS_UNSPECIFIED_ERROR;
	const prt::RuleFileInfo* info = prt::createRuleFileInfo(prtNode->mResolveMap->getString(prtNode->mRuleFile.c_str()), nullptr, &infoStatus);
	if (infoStatus == prt::STATUS_OK) {
		for(size_t r = 0; r < info->getNumRules(); r++) {
			if(info->getRule(r)->getNumParameters() > 0)
				continue;
			for(size_t a = 0; a < info->getRule(r)->getNumAnnotations(); a++) {
				if(!(wcscmp(info->getRule(r)->getAnnotation(a)->getName(), ANNOT_START_RULE))) {
					startRule = info->getRule(r);
					break;
				}
			}
		}
	}
	if(startRule) {
		prtNode->mStartRule = startRule->getName();

		MCHECK(addBoolParameter(node, prtNode->mGenerate,  NAME_GENERATE, true));

		if(prtNode->mGenerateAttrs) {
			prtNode->mGenerateAttrs->destroy();
			prtNode->mGenerateAttrs = nullptr;
		}

		prt::AttributeMapBuilder* aBuilder = prt::AttributeMapBuilder::create();
		createAttributes(node, prtNode->mRuleFile, prtNode->mStartRule, aBuilder, info);
		prtNode->mGenerateAttrs = aBuilder->createAttributeMap();
		aBuilder->destroy();
	}

  if(info)
		info->destroy();

	return MS::kSuccess;
}

PRTEnum::PRTEnum(PRTNode* node, const prt::Annotation* an) : mAnnot(an), mNext(node->mEnums) {
	node->mEnums = this;
}

MStatus PRTEnum::fill() {
	if(mAnnot) {
		MStatus stat;
		for(size_t arg = 0; arg < mAnnot->getNumArguments(); arg++) {
			const wchar_t* key = mAnnot->getArgument(arg)->getKey();
			if(!(wcscmp(key, NULL_KEY)))
				key = mAnnot->getArgument(arg)->getStr();
			mKeys.append(MString(mAnnot->getArgument(arg)->getKey()));
			switch(mAnnot->getArgument(arg)->getType()) {
				case prt::AAT_BOOL:
					MCHECK(mAttr.addField(MString(key), mBVals.length()));
					mBVals.append(mAnnot->getArgument(arg)->getBool());
					mFVals.append(std::numeric_limits<double>::quiet_NaN());
					mSVals.append("");
					break;
				case prt::AAT_FLOAT:
					MCHECK(mAttr.addField(MString(key), mFVals.length()));
					mBVals.append(false);
					mFVals.append(mAnnot->getArgument(arg)->getFloat());
					mSVals.append("");
					break;
				case prt::AAT_STR:
					MCHECK(mAttr.addField(MString(key), mSVals.length()));
					mBVals.append(false);
					mFVals.append(std::numeric_limits<double>::quiet_NaN());
					mSVals.append(MString(mAnnot->getArgument(arg)->getStr()));
					break;
				default:
					break;
			}
		}
	} else {
		for(unsigned int i = 0; i < mKeys.length(); i++)
			mAttr.addField(mKeys[i], (short)i);
	}
	return MS::kSuccess;
}


void PRTEnum::add(const MString & key, const MString & value) {
	mKeys.append(key);
	mBVals.append(false);
	mFVals.append(std::numeric_limits<double>::quiet_NaN());
	mSVals.append(value);
}

namespace UnitQuad {
const double   vertices[]      = { 0, 0, 0,  0, 0, 1,  1, 0, 1,  1, 0, 0 };
const size_t   vertexCount     = 12;
const uint32_t indices[]       = { 0, 1, 2, 3 };
const size_t   indexCount      = 4;
const uint32_t faceCounts[]    = { 4 };
const size_t   faceCountsCount = 1;
const int32_t  seed            = prtu::computeSeed(vertices, vertexCount);
}

// TODO: make evalAttr finds more robust
MStatus PRTAssignCommand::createAttributes(MFnDependencyNode & node, const std::wstring & ruleFile, const std::wstring & startRule, prt::AttributeMapBuilder* aBuilder, const prt::RuleFileInfo* info) {
	MStatus           stat;
	MStatus           stat2;
	MFnNumericData    numericData;
	MFnTypedAttribute tAttr;
	MFnStringData     attrDefaultStr;
	PRTNode*          prtNode = (PRTNode*)node.userNode();
	MString           dummy;

	std::unique_ptr<MayaCallbacks> outputHandler(prtNode->createOutputHandler(nullptr, nullptr, nullptr));
	const prt::AttributeMap* attrs   = aBuilder->createAttributeMap();

	prt::InitialShapeBuilder* isb = prt::InitialShapeBuilder::create();
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
			attrs,
			prtNode->mResolveMap
	);
	const prt::InitialShape* shape = isb->createInitialShapeAndReset();

	prt::generate(&shape, 1, nullptr, &ENC_ATTR, 1, &prtNode->mAttrEncOpts, outputHandler.get(), PRTNode::theCache, nullptr);

	const std::map<std::wstring, MayaCallbacks::AttributeHolder>& evalAttrs = outputHandler->getAttrs();

	prtNode->mBriefName2prtAttr[NAME_GENERATE.asWChar()] = NAME_GENERATE.asWChar();

	for(size_t i = 0; i < info->getNumAttributes(); i++) {
		PRTEnum* e = nullptr;

		const MString name = MString(info->getAttribute(i)->getName());
		MObject attr;

		if(info->getAttribute(i)->getNumParameters() != 0) continue;

		prtNode->mBriefName2prtAttr[prtu::toCleanId(name).asWChar()] = name.asWChar();

		switch(info->getAttribute(i)->getReturnType()) {
		case prt::AAT_BOOL: {
				for(size_t a = 0; a < info->getAttribute(i)->getNumAnnotations(); a++) {
					const prt::Annotation* an = info->getAttribute(i)->getAnnotation(a);
					if(!(wcscmp(an->getName(), ANNOT_RANGE)))
						e = new PRTEnum(prtNode, an);
				}
				const bool value = evalAttrs.find(name.asWChar())->second.mBool;
				if(e) {
					MCHECK(addEnumParameter(node, attr, name, value, e));
				} else {
					MCHECK(addBoolParameter(node, attr, name, value));
				}
				break;
			}
		case prt::AAT_FLOAT: {
				double min = std::numeric_limits<double>::quiet_NaN();
				double max = std::numeric_limits<double>::quiet_NaN();
				for(size_t a = 0; a < info->getAttribute(i)->getNumAnnotations(); a++) {
					const prt::Annotation* an = info->getAttribute(i)->getAnnotation(a);
					if(!(wcscmp(an->getName(), ANNOT_RANGE))) {
						if(an->getNumArguments() == 2 && an->getArgument(0)->getType() == prt::AAT_FLOAT && an->getArgument(1)->getType() == prt::AAT_FLOAT) {
							min = an->getArgument(0)->getFloat();
							max = an->getArgument(1)->getFloat();
						} else
							e = new PRTEnum(prtNode, an);
					}
				}

				const double value = evalAttrs.find(name.asWChar())->second.mFloat;

				if(e) {
					MCHECK(addEnumParameter(node, attr, name, value, e));
				} else {
					MCHECK(addFloatParameter(node, attr, name, value, min, max));
				}
				break;
			}
		case prt::AAT_STR: {
				MString exts;
				bool    asFile  = false;
				bool    asColor = false;
				for(size_t a = 0; a < info->getAttribute(i)->getNumAnnotations(); a++) {
					const prt::Annotation* an = info->getAttribute(i)->getAnnotation(a);
					if(!(wcscmp(an->getName(), ANNOT_RANGE)))
						e = new PRTEnum(prtNode, an);
					else if(!(wcscmp(an->getName(), ANNOT_COLOR)))
						asColor = true;
					else if(!(wcscmp(an->getName(), ANNOT_DIR))) {
						exts = MString(an->getName());
						asFile = true;
					} else if(!(wcscmp(an->getName(), ANNOT_FILE))) {
						asFile = true;
						for(size_t arg = 0; arg < an->getNumArguments(); arg++) {
							if(an->getArgument(arg)->getType() == prt::AAT_STR) {
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
				if(!(asColor) && mvalue.length() == 7 && value[0] == L'#')
					asColor = true;

				if(e) {
					MCHECK(addEnumParameter(node, attr, name, mvalue, e));
				} else if(asFile) {
					MCHECK(addFileParameter(node, attr, name, mvalue, exts));
				} else if(asColor) {
					MCHECK(addColorParameter(node, attr, name, mvalue));
				} else {
					MCHECK(addStrParameter(node, attr, name, mvalue));
				}
				break;
			}
		default:
			break;
		}
	}

	shape->destroy();
	attrs->destroy();

	return MS::kSuccess;
}

MStatus PRTAssignCommand::doIt(const MArgList& args) {
	MStatus stat;

    if (args.length()!=2)
        return MS::kFailure;

	const MString prtNodeName = args.asString(0, &stat);
    MCHECK(stat);
    const MString prtRulePkg = args.asString(1, &stat);
	MCHECK(stat);

	MSelectionList tempList;
	tempList.add(prtNodeName);
	MObject prtNode;
	MCHECK(tempList.getDependNode(0, prtNode));
	MFnDependencyNode fNode(prtNode, &stat);
	MCHECK(stat);

	if(fNode.typeId().id() != PRT_TYPE_ID)
		return MS::kFailure;

	MString sRulePkg;
	updateRuleFiles(fNode, prtRulePkg);

	MGlobal::executeCommand(MString("refreshEditorTemplates"));

	return MS::kSuccess;
}

void* PRTAssignCommand::creator() {
	return new PRTAssignCommand;
}
