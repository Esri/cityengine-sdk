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

#include "PRTNode.h"

#include "Utilities.h"
#include "wrapper/MayaCallbacks.h"

#include "prt/Status.h"
#include "prt/StringUtils.h"

#include <cmath>
#include <limits>


namespace {
const wchar_t*	ANNOT_START_RULE	= L"@StartRule";
const wchar_t*	ANNOT_RANGE			= L"@Range";
const wchar_t*	ANNOT_COLOR			= L"@Color";
const wchar_t*	ANNOT_DIR			= L"@Directory";
const wchar_t*	ANNOT_FILE			= L"@File";
const wchar_t*	NULL_KEY			= L"#NULL#";
}

inline MString & PRTAttrs::getStringParameter(MObject & node, MObject & attr, MString & value) {
	MPlug plug(node, attr);
	plug.getValue(value);
	return value;
}

inline MStatus PRTAttrs::setStringParameter(MObject & node, MObject & attr, MString & value) {
	MPlug plug(node, attr);
	return plug.setValue(value);
}

MStatus PRTAttrs::addParameter(MFnDependencyNode & node, MObject & attr, MFnAttribute& tAttr) {
	if(!(node.hasAttribute(tAttr.shortName()))) {
		MCHECK(tAttr.setKeyable (true));
		MCHECK(tAttr.setHidden(false));
		MCHECK(tAttr.setStorable(true));
		MCHECK(node.addAttribute(attr));
	}
	return MS::kSuccess;
}

MString PRTAttrs::longName(const MString& attrName) {
	return prtu::toCleanId(attrName.substring(attrName.index('$') + 1, attrName.length()));
}

MString PRTAttrs::briefName(const MString & attrName) {
	return prtu::toCleanId(attrName);
}

MStatus PRTAttrs::addBoolParameter(MFnDependencyNode & node, MObject & attr, const MString & name, bool value) {
	MStatus stat;
	MFnNumericAttribute nAttr;
	attr = nAttr.create(longName(name), briefName(name), MFnNumericData::kBoolean, value, &stat);
	if ( stat != MS::kSuccess ) throw stat;

	MCHECK(addParameter(node, attr, nAttr));

	MPlug plug(node.object(), attr);
	MCHECK(plug.setValue(value));

	return MS::kSuccess;
}

MStatus PRTAttrs::addFloatParameter(MFnDependencyNode & node, MObject & attr, const MString & name, double value, double min, double max) {
	MStatus stat;
	MFnNumericAttribute nAttr;
	attr = nAttr.create(longName(name), briefName(name), MFnNumericData::kDouble, value, &stat );
	if ( stat != MS::kSuccess ) throw stat;

	if(!isnan(min)) {
		MCHECK(nAttr.setMin(min));
	}

	if(!isnan(max)) {
		MCHECK(nAttr.setMax( max ));
	}

	MCHECK(addParameter(node, attr, nAttr));

	MPlug plug(node.object(), attr);
	MCHECK(plug.setValue(value));

	return MS::kSuccess;
}

MStatus PRTAttrs::addEnumParameter(MFnDependencyNode & node, MObject & attr, const MString & name, bool value, PRTEnum * e) {
	short idx = 0;
	for(int i = (int)e->mBVals.length(); --i >= 0;) {
		if((e->mBVals[i] != 0) == value) {
			idx = (short)i;
			break;
		}
	}

	return addEnumParameter(node, attr, name, idx, e);
}

MStatus PRTAttrs::addEnumParameter(MFnDependencyNode & node, MObject & attr, const MString & name, double value, PRTEnum * e) {
	short idx = 0;
	for(int i = (int)e->mFVals.length(); --i >= 0;) {
		if(e->mFVals[i] == value) {
			idx = (short)i;
			break;
		}
	}

	return addEnumParameter(node, attr, name, idx, e);
}

MStatus PRTAttrs::addEnumParameter(MFnDependencyNode & node, MObject & attr, const MString & name, MString value, PRTEnum * e) {
	short idx = 0;
	for(int i = (int)e->mSVals.length(); --i >= 0;) {
		if(e->mSVals[i] == value) {
			idx = (short)i;
			break;
		}
	}

	return addEnumParameter(node, attr, name, idx, e);
}

MStatus PRTAttrs::addEnumParameter(MFnDependencyNode & node, MObject & attr, const MString & name, short value, PRTEnum * e) {


	MStatus stat;

	attr = e->mAttr.create(longName(name), briefName(name), value, &stat);
	MCHECK(stat);

	MCHECK(e->fill());

	MCHECK(addParameter(node, attr, e->mAttr));

	MPlug plug(node.object(), attr);
	MCHECK(plug.setValue(value));

	return MS::kSuccess;
}

MStatus PRTAttrs::addFileParameter(MFnDependencyNode & node, MObject & attr, const MString & name, const MString & value, MString & exts ) {
	MStatus           stat;
	MStatus           stat2;
	MFnStringData		  stringData;
	MFnTypedAttribute sAttr;

	attr = sAttr.create(longName(name), briefName(name), MFnData::kString, stringData.create("", &stat2), &stat );
	MCHECK(stat2);
	MCHECK(stat);
	MCHECK(sAttr.setUsedAsFilename(true));
	MCHECK(addParameter(node, attr, sAttr));
	MCHECK(sAttr.setNiceNameOverride(exts));

	MPlug plug(node.object(), attr);
	MCHECK(plug.setValue(value));


	return MS::kSuccess;
}

MStatus PRTAttrs::addColorParameter(MFnDependencyNode & node, MObject & attr, const MString & name, MString & value ) {
	MStatus             stat;
	MFnNumericAttribute nAttr;

	const wchar_t* s = value.asWChar();

	attr = nAttr.createColor(longName(name), briefName(name), &stat );
	MCHECK(stat);

	double r = 0.0;
	double g = 0.0;
	double b = 0.0;

	if (s[0] == '#' && wcslen(s) >= 7) {
		r = (double)((prtu::fromHex(s[1]) << 4) + prtu::fromHex(s[2])) / 255.0;
		g = (double)((prtu::fromHex(s[3]) << 4) + prtu::fromHex(s[4])) / 255.0;
		b = (double)((prtu::fromHex(s[5]) << 4) + prtu::fromHex(s[6])) / 255.0;

		nAttr.setDefault(r, g, b);
	}

	MCHECK(addParameter(node, attr, nAttr));

	MFnNumericData fnData;
	MObject        rgb = fnData.create(MFnNumericData::k3Double, &stat);
	MCHECK(stat);

	fnData.setData(r, g, b);
	MPlug plug(node.object(), attr);
	MCHECK(plug.setValue(rgb));

	return MS::kSuccess;
}


MStatus PRTAttrs::addStrParameter(MFnDependencyNode & node, MObject & attr, const MString & name, MString & value ) {
	MStatus           stat;
	MStatus           stat2;
	MFnStringData		  stringData;
	MFnTypedAttribute sAttr;

	attr = sAttr.create(longName(name), briefName(name), MFnData::kString, stringData.create(value, &stat2), &stat );
	MCHECK(stat2);
	MCHECK(stat);
	MCHECK(addParameter(node, attr, sAttr));

	MPlug plug(node.object(), attr);
	MCHECK(plug.setValue(value));

	return MS::kSuccess;
}

MStatus PRTAttrs::updateRuleFiles(MFnDependencyNode & node, MString & rulePkg) {
	PRTNode* prtNode = (PRTNode*)node.userNode();
	MStatus  stat;

	std::string utf8Path(rulePkg.asUTF8());
	std::vector<char> percentEncodedPath(2*utf8Path.size()+1);
	size_t len = percentEncodedPath.size();
	prt::StringUtils::percentEncode(utf8Path.c_str(), &percentEncodedPath[0], &len);
	if(len > percentEncodedPath.size()+1){
		percentEncodedPath.resize(len);
		prt::StringUtils::percentEncode(utf8Path.c_str(), &percentEncodedPath[0], &len);
	}

	std::string uri(FILE_PREFIX);
	uri.append(&percentEncodedPath[0]);

	prtNode->mLRulePkg = uri;

	if(prtNode->mCreatedInteractively) {
		int count = (int)node.attributeCount(&stat);
		MCHECK(stat);

		MObjectArray attrs;

		for(int i = 0; i < count; i++) {
			MObject attr = node.attribute(i, &stat);
			if(stat != MS::kSuccess) continue;
			attrs.append(attr);
		}

		for(unsigned int i = 0; i < attrs.length(); i++) {
			MPlug   plug(node.object(), attrs[i]);
			MString name = plug.partialName();

			if(prtNode->mBriefName2prtAttr.count(name.asWChar()))
				node.removeAttribute(attrs[i]);
		}
		prtNode->destroyEnums();
	} else {
		node.removeAttribute(node.attribute(NAME_GENERATE, &stat));
		MCHECK(stat);
	}

	prtNode->mRuleFile.clear();
	prtNode->mStartRule.clear();

	MString      unpackDir       = MGlobal::executeCommandStringResult("workspace -q -fullName");
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
		const wchar_t * const* keys   = prtNode->mResolveMap->getKeys(&nKeys);
		std::wstring           sCGB(L".cgb");
		for(size_t k = 0; k < nKeys; k++) {
			std::wstring key = std::wstring(keys[k]);
			if(std::equal(sCGB.rbegin(), sCGB.rend(), key.rbegin())) {
				prtNode->mRuleFile = key;
				break;
			}
		}
	} else {
		prtNode->mResolveMap = 0;
	}

	if(prtNode->mRuleFile.length() > 0)
		updateStartRules(node);

	return MS::kSuccess;
}

MStatus PRTAttrs::updateStartRules(MFnDependencyNode & node) {
	PRTNode* prtNode = (PRTNode*)node.userNode();

	const prt::RuleFileInfo::Entry* startRule = 0;

	prt::Status infoStatus = prt::STATUS_UNSPECIFIED_ERROR;
	const prt::RuleFileInfo* info = prt::createRuleFileInfo(prtNode->mResolveMap->getString(prtNode->mRuleFile.c_str()), 0, &infoStatus);
	if (infoStatus == prt::STATUS_OK) {
		for(size_t r = 0; r < info->getNumRules(); r++) {
			if(info->getRule(r)->getNumParameters() > 0) continue;
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
			prtNode->mGenerateAttrs = 0;
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
			switch(mAnnot->getArgument(arg)->getType()) {
				mKeys.append(MString(mAnnot->getArgument(arg)->getKey()));
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

static const double 	UnitQuad_vertices[]      = { 0, 0, 0,  0, 0, 1,  1, 0, 1,  1, 0, 0 };
static const size_t 	UnitQuad_vertexCount     = 12;
static const uint32_t	UnitQuad_indices[]       = { 0, 1, 2, 3 };
static const size_t 	UnitQuad_indexCount      = 4;
static const uint32_t	UnitQuad_faceCounts[]    = { 4 };
static const size_t 	UnitQuad_faceCountsCount = 1;
static const int32_t	UnitQuad_seed = prtu::computeSeed(UnitQuad_vertices, UnitQuad_vertexCount);

// TODO: make evalAttr finds more robust
MStatus PRTAttrs::createAttributes(MFnDependencyNode & node, const std::wstring & ruleFile, const std::wstring & startRule, prt::AttributeMapBuilder* aBuilder, const prt::RuleFileInfo* info) {
	MStatus           stat;
	MStatus           stat2;
	MFnNumericData    numericData;
	MFnTypedAttribute tAttr;
	MFnStringData     attrDefaultStr;
	PRTNode*          prtNode = (PRTNode*)node.userNode();
	MString           dummy;

	MayaCallbacks* outputHandler = prtNode->createOutputHandler(0, 0);
	const prt::AttributeMap* attrs   = aBuilder->createAttributeMap();

	prt::InitialShapeBuilder* isb = prt::InitialShapeBuilder::create();
	isb->setGeometry(
			UnitQuad_vertices, 
			UnitQuad_vertexCount, 
			UnitQuad_indices, 
			UnitQuad_indexCount,
			UnitQuad_faceCounts,
			UnitQuad_faceCountsCount
	);
	isb->setAttributes(
			ruleFile.c_str(),
			startRule.c_str(),
			UnitQuad_seed,
			L"",
			attrs,
			prtNode->mResolveMap
	);
	const prt::InitialShape* shape = isb->createInitialShapeAndReset();

	prt::Status generateStatus = prt::generate(&shape, 1, 0, &ENC_ATTR, 1, &prtNode->mAttrEncOpts, outputHandler, PRTNode::theCache, 0);

	const std::map<std::wstring, MayaCallbacks::AttributeHolder>& evalAttrs = outputHandler->getAttrs();

	prtNode->mBriefName2prtAttr[NAME_GENERATE.asWChar()] = NAME_GENERATE.asWChar();

	for(size_t i = 0; i < info->getNumAttributes(); i++) {
		PRTEnum*       e          = 0;
		bool           createAttr = false;

		const MString  name       = MString(info->getAttribute(i)->getName());
		MObject        attr;

		if(info->getAttribute(i)->getNumParameters() != 0) continue;

		prtNode->mBriefName2prtAttr[prtu::toCleanId(name).asWChar()] = name.asWChar();

		switch(info->getAttribute(i)->getReturnType()) {
		case prt::AAT_BOOL: {
				for(size_t a = 0; a < info->getAttribute(i)->getNumAnnotations(); a++) {
					const prt::Annotation* an = info->getAttribute(i)->getAnnotation(a);
					if(!(wcscmp(an->getName(), ANNOT_RANGE)))
						e = new PRTEnum(prtNode, an);
				}
 
				bool value = evalAttrs.find(name.asWChar())->second.mBool;

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

				double value = evalAttrs.find(name.asWChar())->second.mFloat;

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

				std::wstring value = evalAttrs.find(name.asWChar())->second.mString;
				MString mvalue(value.c_str());
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
	delete outputHandler;

	return MS::kSuccess;
}

MStatus PRTAttrs::doIt(const MArgList& args) {
	MStatus stat;

	MString prtNodeName = args.asString(0, &stat);
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
	updateRuleFiles(fNode, getStringParameter(prtNode, ((PRTNode*)fNode.userNode())->rulePkg, sRulePkg));

	MGlobal::executeCommand(MString("refreshEditorTemplates"));

	return MS::kSuccess;
}

void* PRTAttrs::creator() {
	return new PRTAttrs;
}
