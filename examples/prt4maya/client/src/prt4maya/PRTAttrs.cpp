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
#include "Utilities.h"
#include "PRTNode.h"
#include "wrapper/MayaOutputHandler.h"
#include <limits>

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
	M_CHECK(tAttr.setKeyable (true));
	M_CHECK(tAttr.setHidden(false));
	M_CHECK(node.addAttribute(attr, MFnDependencyNode::kLocalDynamicAttr));

	return MS::kSuccess;
}

MString PRTAttrs::longName(const MString & attrName) {
	return attrName.substring(attrName.index('$') + 1, attrName.length());
}

MString PRTAttrs::briefName(const MString & attrName) {
	return attrName;
}

MStatus PRTAttrs::addBoolParameter(MFnDependencyNode & node, MObject & attr, const MString & name, bool value) {
	MStatus stat;
	MFnNumericAttribute nAttr;
	attr = nAttr.create(longName(name), briefName(name), MFnNumericData::kBoolean, value, &stat);
	if ( stat != MS::kSuccess ) throw stat;

	M_CHECK(addParameter(node, attr, nAttr));

	MPlug plug(node.object(), attr);
	M_CHECK(plug.setValue(value));

	return MS::kSuccess;
}

MStatus PRTAttrs::addFloatParameter(MFnDependencyNode & node, MObject & attr, const MString & name, double value, double min, double max) {
	MStatus stat;
	MFnNumericAttribute nAttr;
	attr = nAttr.create(longName(name), briefName(name), MFnNumericData::kDouble, value, &stat );
	if ( stat != MS::kSuccess ) throw stat;

	if(!isnan(min)) {
		M_CHECK(nAttr.setMin(min));
	}

	if(!isnan(max)) {
		M_CHECK(nAttr.setMax( max ));
	}

	M_CHECK(addParameter(node, attr, nAttr));

	MPlug plug(node.object(), attr);
	M_CHECK(plug.setValue(value));

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
	M_CHECK(stat);

	M_CHECK(e->fill());

	M_CHECK(addParameter(node, attr, e->mAttr));

	MPlug plug(node.object(), attr);
	M_CHECK(plug.setValue(value));

	return MS::kSuccess;
}

MStatus PRTAttrs::addFileParameter(MFnDependencyNode & node, MObject & attr, const MString & name, MString & value ) {
	MStatus           stat;
	MStatus           stat2;
	MFnStringData		  stringData;
	MFnTypedAttribute sAttr;

	attr = sAttr.create(longName(name), briefName(name), MFnData::kString, stringData.create("", &stat2), &stat );
	M_CHECK(stat2);
	M_CHECK(stat);
	M_CHECK(sAttr.setUsedAsFilename(true));
	M_CHECK(addParameter(node, attr, sAttr));
	M_CHECK(sAttr.setNiceNameOverride(value));

	MPlug plug(node.object(), attr);
	M_CHECK(plug.setValue(MString()));


	return MS::kSuccess;
}

MStatus PRTAttrs::addColorParameter(MFnDependencyNode & node, MObject & attr, const MString & name, MString & value ) {
	MStatus             stat;
	MFnNumericAttribute nAttr;

	const wchar_t* s = value.asWChar();

	attr = nAttr.createColor(longName(name), briefName(name), &stat );
	M_CHECK(stat);

	double r = 0.0;
	double g = 0.0;
	double b = 0.0;

	if (s[0] == '#' && wcslen(s) >= 7) {
		r = (double)((fromHex(s[1]) << 4) + fromHex(s[2])) / 255.0;
		g = (double)((fromHex(s[3]) << 4) + fromHex(s[4])) / 255.0;
		b = (double)((fromHex(s[5]) << 4) + fromHex(s[6])) / 255.0;

		nAttr.setDefault(r, g, b);
	}

	M_CHECK(addParameter(node, attr, nAttr));

	MFnNumericData fnData;
	MObject        rgb = fnData.create(MFnNumericData::k3Double, &stat);
	M_CHECK(stat);

	fnData.setData(r, g, b);
	MPlug plug(node.object(), attr);
	M_CHECK(plug.setValue(rgb));

	return MS::kSuccess;
}


MStatus PRTAttrs::addStrParameter(MFnDependencyNode & node, MObject & attr, const MString & name, MString & value ) {
	MStatus           stat;
	MStatus           stat2;
	MFnStringData		  stringData;
	MFnTypedAttribute sAttr;

	attr = sAttr.create(longName(name), briefName(name), MFnData::kString, stringData.create(value, &stat2), &stat );
	M_CHECK(stat2);
	M_CHECK(stat);
	M_CHECK(addParameter(node, attr, sAttr));

	MPlug plug(node.object(), attr);
	M_CHECK(plug.setValue(value));

	return MS::kSuccess;
}

MStatus PRTAttrs::updateRuleFiles(MFnDependencyNode & node, MString & rulePkg) {
	PRTNode* prtNode = (PRTNode*)node.userNode();
	MStatus  stat;

	std::wstring path(FILE_PREFIX);
	path.append(rulePkg.asWChar());

	prtNode->mLRulePkg = path;

	int count = (int)node.attributeCount(&stat);
	M_CHECK(stat);

	MObjectArray attrs;

	for(int i = 0; i < count; i++) {
		MObject attr = node.attribute(i, &stat);
		if(stat != MS::kSuccess) continue;
		attrs.append(attr);
	}

	for(unsigned int i = 0; i < attrs.length(); i++) {
		MPlug   plug(node.object(), attrs[i]);
		MString name = plug.partialName();
		if(name.index('$') >= 0) {
			node.removeAttribute(attrs[i]);
		}	else if(name == NAME_RULE_FILE) {
			node.removeAttribute(attrs[i]);
		} else if(name == NAME_START_RULE) {
			node.removeAttribute(attrs[i]);
		}
	}
	prtNode->destroyEnums();

	MStringArray ruleFiles;
	prt::Status resolveMapStatus = prt::STATUS_UNSPECIFIED_ERROR;
	prtNode->mResolveMap = prt::createResolveMap(path.c_str(), true, &resolveMapStatus);
	if(resolveMapStatus == prt::STATUS_OK) {
		size_t nKeys;
		const wchar_t** keys   = prtNode->mResolveMap->getKeys(&nKeys);
		std::wstring    sCGB(L".cgb");
		for(size_t k = 0; k < nKeys; k++) {
			std::wstring key = std::wstring(keys[k]);
			if(std::equal(sCGB.rbegin(), sCGB.rend(), key.rbegin()))
				ruleFiles.append(MString(key.c_str()));
		}
	} else {
		prtNode->mResolveMap = 0;
	}

	if(ruleFiles.length() > 0)
		updateStartRules(node, ruleFiles);

	return MS::kSuccess;
}

MStatus PRTAttrs::updateStartRules(MFnDependencyNode & node, MStringArray & ruleFiles) {
	PRTNode* prtNode = (PRTNode*)node.userNode();

	PRTEnum * eRuleFiles = new PRTEnum(prtNode);

	for(unsigned int i = 0; i < ruleFiles.length(); i++)
		eRuleFiles->add(ruleFiles[i].substring(ruleFiles[i].index('/') + 1, ruleFiles[i].rindex('.') - 1), ruleFiles[i]);

	M_CHECK(addEnumParameter(node, prtNode->mRuleFile, NAME_RULE_FILE, ruleFiles[0], eRuleFiles));

	std::vector<const prt::RuleFileInfo::Entry*> annotStartRules;
	std::vector<const prt::RuleFileInfo::Entry*> noArgRules;

	prt::Status infoStatus = prt::STATUS_UNSPECIFIED_ERROR;
	const prt::RuleFileInfo* info = prt::createRuleFileInfo(prtNode->mResolveMap->getString(ruleFiles[0].asWChar()), 0, &infoStatus); // TODO: callback???
	if (infoStatus == prt::STATUS_OK) {
		for(size_t r = 0; r < info->getNumRules(); r++) {
			if(info->getRule(r)->getNumParameters() > 0) continue;
			bool startRule = false;
			for(size_t a = 0; a < info->getRule(r)->getNumAnnotations(); a++) {
				if(!(wcscmp(info->getRule(r)->getAnnotation(a)->getName(), ANNOT_START_RULE))) {
					annotStartRules.push_back(info->getRule(r));
					startRule = true;
				}
			}
			if(!startRule)
				noArgRules.push_back(info->getRule(r));
		}
	}

	std::vector<const prt::RuleFileInfo::Entry*> startRules = annotStartRules.size() > 0 ? annotStartRules : noArgRules;

	MStringArray startRuleList;

	for(size_t r = 0; r < startRules.size(); r++)
		startRuleList.append(startRules[r]->getName());

	if(startRuleList.length() > 0) {
		PRTEnum* eStartRule = new PRTEnum(prtNode);
		
		for(size_t r = 0; r < startRules.size(); r++)
			eStartRule->add(longName(MString(startRules[r]->getName())), startRules[r]->getName());

		M_CHECK(addEnumParameter(node, prtNode->mStartRule, NAME_START_RULE, startRuleList[0], eStartRule));

		if(prtNode->mGenerateAttrs) {
			prtNode->mGenerateAttrs->destroy();
			prtNode->mGenerateAttrs = 0;
		}

		prt::AttributeMapBuilder* aBuilder = prt::AttributeMapBuilder::create();
		createAttributes(node, ruleFiles[0], startRuleList[0], aBuilder, info);
		prtNode->mGenerateAttrs = aBuilder->createAttributeMap();
		aBuilder->destroy();
	}

	return MS::kSuccess;
}

PRTEnum::PRTEnum(PRTNode* node, const prt::Annotation* an) : mAnnot(an), mNext(node->mEnums) {
	node->mEnums = this;
}

MStatus PRTEnum::fill() {
	if(mAnnot) {
		MStatus stat;
		for(size_t arg = 0; arg < mAnnot->getNumArguments(); arg++) {
			switch(mAnnot->getArgument(arg)->getType()) {
				mKeys.append(MString(mAnnot->getArgument(arg)->getKey()));
				case prt::AAT_BOOL:
					M_CHECK(mAttr.addField(MString(mAnnot->getArgument(arg)->getKey()), mBVals.length()));
					mBVals.append(mAnnot->getArgument(arg)->getBool());
					mFVals.append(std::numeric_limits<double>::quiet_NaN());
					mSVals.append("");
					break;
				case prt::AAT_FLOAT:
					M_CHECK(mAttr.addField(MString(mAnnot->getArgument(arg)->getKey()), mFVals.length()));
					mBVals.append(false);
					mFVals.append(mAnnot->getArgument(arg)->getFloat());
					mSVals.append("");
					break;
				case prt::AAT_STR:
					M_CHECK(mAttr.addField(MString(mAnnot->getArgument(arg)->getKey()), mSVals.length()));
					mBVals.append(false);
					mFVals.append(std::numeric_limits<double>::quiet_NaN());
					mSVals.append(MString(mAnnot->getArgument(arg)->getStr()));
					break;
			}
		}
	} else {
		for(unsigned int i = 0; i < mKeys.length(); i++)
			MStatus result = mAttr.addField(mKeys[i], (short)i);
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

MStatus PRTAttrs::createAttributes(MFnDependencyNode & node, MString & ruleFile, MString & startRule, prt::AttributeMapBuilder* aBuilder, const prt::RuleFileInfo* info) {
	MStatus           stat;
	MStatus           stat2;
	MFnNumericData    numericData;
	MFnTypedAttribute tAttr;
	MFnStringData     attrDefaultStr;
	PRTNode*          prtNode = (PRTNode*)node.userNode();
	MString           dummy;

	MayaOutputHandler* outputHandler = prtNode->createOutputHandler(0, 0);
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
			ruleFile.asWChar(),
			startRule.asWChar(),
			isb->computeSeed(),
			L"",
			attrs,
			prtNode->mResolveMap
	);
	const prt::InitialShape* shape = isb->createInitialShapeAndReset();

//	size_t size = 4096;
//	DBG("%s", shape->toXML((char*)malloc(size), &size));

	prt::Status generateStatus = prt::generate(&shape, 1, 0, &ENC_ATTR, 1, &prtNode->mAttrEncOpts, outputHandler, PRTNode::theCache, 0);

	const std::map<std::wstring, MayaOutputHandler::AttributeHolder>& evalAttrs = outputHandler->getAttrs();

	static const std::wstring STYLE = L"Default$"; // currently hardcoded to default style
	for(size_t i = 0; i < info->getNumAttributes(); i++) {
		PRTEnum*       e          = 0;
		bool           createAttr = false;

		std::wstring styledAttributeName(info->getAttribute(i)->getName());
		const MString  name      = MString(styledAttributeName.substr(STYLE.length()).c_str()); // FIXME
		MObject        attr;

		if(info->getAttribute(i)->getNumParameters() != 0) continue;

		switch(info->getAttribute(i)->getReturnType()) {
		case prt::AAT_BOOL: {
				for(size_t a = 0; a < info->getAttribute(i)->getNumAnnotations(); a++) {
					const prt::Annotation* an = info->getAttribute(i)->getAnnotation(a);
					if(!(wcscmp(an->getName(), ANNOT_RANGE)))
						e = new PRTEnum(prtNode, an);
				}

				bool value = evalAttrs.find(name.asWChar())->second.mBool;

				if(e) {
					M_CHECK(addEnumParameter(node, attr, name, value, e));
				} else {
					M_CHECK(addBoolParameter(node, attr, name, value));
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
					M_CHECK(addEnumParameter(node, attr, name, value, e));
				} else {
					M_CHECK(addFloatParameter(node, attr, name, value, min, max));
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
								exts += ");";
							}
						}
					}
				}

				std::wstring value = evalAttrs.find(name.asWChar())->second.mString;

				MString mvalue(value.c_str());
				if(e) {
					M_CHECK(addEnumParameter(node, attr, name, mvalue, e));
				} else if(asFile) {
					exts += "All Files (*.*)";
					M_CHECK(addFileParameter(node, attr, name, exts));
				} else if(asColor) {
					M_CHECK(addColorParameter(node, attr, name, mvalue));
				} else {
					M_CHECK(addStrParameter(node, attr, name, mvalue));
				}

				break;
			}
		}

		for(size_t a = 0; a < info->getAttribute(i)->getNumAnnotations(); a++) {
			const prt::Annotation* an = info->getAttribute(i)->getAnnotation(a);
			if(!(wcscmp(an->getName(), ANNOT_GROUP))) {
				MFnAttribute fAttr(attr);
				for(size_t arg = 0; arg < an->getNumArguments(); arg++) {
					if(an->getArgument(arg)->getType() == prt::AAT_STR)
						M_CHECK(fAttr.addToCategory(MString(an->getArgument(arg)->getStr())));
				}
			}
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
	M_CHECK(stat);

	MSelectionList tempList;
	tempList.add(prtNodeName);
	MObject prtNode;
	M_CHECK(tempList.getDependNode(0, prtNode));
	MFnDependencyNode fNode(prtNode, &stat);
	M_CHECK(stat);

	if(fNode.typeId().id() != PRT_TYPE_ID)
		return MS::kFailure;

	MString sRulePkg;
	updateRuleFiles(fNode, getStringParameter(prtNode, ((PRTNode*)fNode.userNode())->theRulePkg, sRulePkg));

	MGlobal::executeCommand(MString("refreshEditorTemplates"));

	return MS::kSuccess;
}

void* PRTAttrs::creator() {
	return new PRTAttrs;
}
