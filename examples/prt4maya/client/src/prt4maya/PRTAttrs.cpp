#define MNoPluginEntry
#define MNoVersionString
#include "prt4mayaNode.h"
#include <limits>

inline MString & PRTAttrs::getStringParameter(MObject & node, MObject & attr, MString & value) {
	MPlug plug(node, attr);
	plug.getValue(value);
	return value;
}

MStatus PRTAttrs::addParameter(MFnDependencyNode & node, MObject & attr, MFnAttribute& tAttr) {
	M_CHECK(tAttr.setKeyable (true));
	//	M_CHECK(tAttr.setCached  (true));
	//	M_CHECK(tAttr.setStorable(true));
	M_CHECK(tAttr.setHidden(false));
	//	M_CHECK(tAttr.setAffectsAppearance(true));
	//	M_CHECK(tAttr.setIndeterminant(true));
	//	M_CHECK(tAttr.setChannelBox(true));
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
	for(int i = (int)e->bVals.length(); --i >= 0;) {
		if((e->bVals[i] != 0) == value) {
			idx = (short)i;
			break;
		}
	}

	return addEnumParameter(node, attr, name, idx, e);
}

MStatus PRTAttrs::addEnumParameter(MFnDependencyNode & node, MObject & attr, const MString & name, double value, PRTEnum * e) {
	short idx = 0;
	for(int i = (int)e->fVals.length(); --i >= 0;) {
		if(e->fVals[i] == value) {
			idx = (short)i;
			break;
		}
	}

	return addEnumParameter(node, attr, name, idx, e);
}

MStatus PRTAttrs::addEnumParameter(MFnDependencyNode & node, MObject & attr, const MString & name, MString value, PRTEnum * e) {
	short idx = 0;
	for(int i = (int)e->bVals.length(); --i >= 0;) {
		if(e->sVals[i] == value) {
			idx = (short)i;
			break;
		}
	}

	return addEnumParameter(node, attr, name, idx, e);
}

MStatus PRTAttrs::addEnumParameter(MFnDependencyNode & node, MObject & attr, const MString & name, short value, PRTEnum * e) {
	MStatus stat;

	attr = e->eAttr.create(longName(name), briefName(name), value, &stat);
	M_CHECK(stat);

	M_CHECK(e->fill());

	M_CHECK(addParameter(node, attr, e->eAttr));

	MPlug plug(node.object(), attr);
	M_CHECK(plug.setValue(value));

	return MS::kSuccess;
}

MStatus PRTAttrs::addFileParameter(MFnDependencyNode & node, MObject & attr, const MString & name, MString & value ) {
	MStatus           stat;
	MStatus           stat2;
	MFnStringData		  stringData;
	MFnTypedAttribute sAttr;

	attr = sAttr.create(longName(name), briefName(name), MFnData::kString, stringData.create(value, &stat2), &stat );
	M_CHECK(stat2);
	M_CHECK(stat);
	M_CHECK(sAttr.setUsedAsFilename(true));
	M_CHECK(addParameter(node, attr, sAttr));

	MPlug plug(node.object(), attr);
	M_CHECK(plug.setValue(MString()));


	return MS::kSuccess;
}

int fromHex(wchar_t c) {
	switch(c) {
	case '0': return 0;	case '1': return 1;	case '2': return 2;	case '3': return 3;	case '4': return 4;
	case '5': return 5;	case '6': return 6;	case '7': return 7;	case '8': return 8;	case '9': return 9;
	case 'a': case 'A': return 0xa;
	case 'b': case 'B': return 0xb;
	case 'c': case 'C': return 0xc;
	case 'd': case 'D': return 0xd;
	case 'e': case 'E': return 0xe;
	case 'f': case 'F': return 0xf;
	default: 
		return 0;
	}
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

	std::wstring path(RPK_PREFIX);
	path.append(rulePkg.asWChar());

	prtNode->lRulePkg = path;

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
	if(prt::ProceduralRT::getResolveMap(path.c_str(), &prtNode->resolveMap) == prt::PRT_OK) {
		const wchar_t** keys   = prtNode->resolveMap->getKeys();
		const size_t    nKeys  = prtNode->resolveMap->getKeysSize();
		std::wstring    sClass(L".class");
		std::wstring    sCGB(L".cgb");
		for(size_t k = 0; k < nKeys; k++) {
			std::wstring key = std::wstring(keys[k]);
			if(    std::equal(sClass.rbegin(), sClass.rend(), key.rbegin())
				|| std::equal(sCGB.rbegin(),   sCGB.rend(),   key.rbegin()))
				ruleFiles.append(MString(key.c_str()));
		}
	} else {
		prtNode->resolveMap = 0;
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

	M_CHECK(addEnumParameter(node, prtNode->ruleFile, NAME_RULE_FILE, ruleFiles[0], eRuleFiles));

	std::vector<const prt::Entry*> annotStartRules;
	std::vector<const prt::Entry*> noArgRules;

	const prt::RuleFileInfo* info = 0;
	if(prt::ProceduralRT::createRuleFileInfo(prtNode->resolveMap->getString(ruleFiles[0].asWChar()), &info) == prt::PRT_OK) {
		for(size_t r = 0; r < info->getNumRules(); r++) {
			if(info->getRule(r)->getNumParameters() > 0) continue;
			bool startRule = false;
			for(size_t a = 0; a < info->getRule(r)->getNumAnnotations(); a++) {
				if(!(wcscmp(info->getRule(r)->getAnnotation(a)->getName(), L"@StartRule"))) {
					annotStartRules.push_back(info->getRule(r));
					startRule = true;
				}
			}
			if(!startRule)
				noArgRules.push_back(info->getRule(r));
		}
	}

	std::vector<const prt::Entry*> startRules = annotStartRules.size() > 0 ? annotStartRules : noArgRules;

	MStringArray startRuleList;

	for(size_t r = 0; r < startRules.size(); r++)
		startRuleList.append(startRules[r]->getName());

	if(startRuleList.length() > 0) {
		PRTEnum* eStartRule = new PRTEnum(prtNode);
		
		for(size_t r = 0; r < startRules.size(); r++)
			eStartRule->add(longName(MString(startRules[r]->getName())), startRules[r]->getName());

		M_CHECK(addEnumParameter(node, prtNode->startRule, NAME_START_RULE, startRuleList[0], eStartRule));

		if(prtNode->generateAttrs) {
			prtNode->generateAttrs->destroy();
			prtNode->generateAttrs = 0;
		}

		prt::AttributableBuilder* aBuilder = prt::AttributableBuilder::create();

		aBuilder->setString(L"ruleFile",  ruleFiles[0].asWChar());
		aBuilder->setString(L"startRule", startRuleList[0].asWChar());

		updateAttributes(node, aBuilder, info);

		prtNode->generateAttrs = aBuilder->createAttributable();
		aBuilder->destroy();
	}

	return MS::kSuccess;
}

PRTEnum::PRTEnum(PRTNode * node, const prt::Annotation* an) : annot(an), next(node->enums) {
	node->enums = this;
}

MStatus PRTEnum::fill() {
	if(annot) {
		MStatus stat;
		for(size_t arg = 0; arg < annot->getNumArguments(); arg++) {
			switch(annot->getArgument(arg)->getType()) {
				keys.append(MString(annot->getArgument(arg)->getKey()));
				case prt::CGA_BOOL:
					M_CHECK(eAttr.addField(MString(annot->getArgument(arg)->getKey()), bVals.length()));
					bVals.append(annot->getArgument(arg)->getBool());
					fVals.append(std::numeric_limits<double>::quiet_NaN());
					sVals.append("");
					break;
				case prt::CGA_FLOAT:
					M_CHECK(eAttr.addField(MString(annot->getArgument(arg)->getKey()), fVals.length()));
					bVals.append(false);
					fVals.append(annot->getArgument(arg)->getFloat());
					sVals.append("");
					break;
				case prt::CGA_STR:
					M_CHECK(eAttr.addField(MString(annot->getArgument(arg)->getKey()), sVals.length()));
					bVals.append(false);
					fVals.append(std::numeric_limits<double>::quiet_NaN());
					sVals.append(MString(annot->getArgument(arg)->getStr()));
					break;
			}
		}
	} else {
		for(unsigned int i = 0; i < keys.length(); i++)
			MStatus result = eAttr.addField(keys[i], (short)i);
	}
	return MS::kSuccess;
}


void PRTEnum::add(const MString & key, const MString & value) {
	keys.append(key);
	bVals.append(false);
	fVals.append(std::numeric_limits<double>::quiet_NaN());
	sVals.append(value);
}

static const double 	UnitQuad_vertices[]      = { 0, 0, 0,  0, 0, 1,  1, 0, 1,  1, 0, 0 };
static const size_t 	UnitQuad_vertexCount     = 12;
static const uint32_t	UnitQuad_indices[]       = { 0, 1, 2, 3 };
static const size_t 	UnitQuad_indexCount      = 4;
static const uint32_t	UnitQuad_faceCounts[]    = { 4 };
static const size_t 	UnitQuad_faceCountsCount = 1;

MStatus PRTAttrs::updateAttributes(MFnDependencyNode & node, prt::AttributableBuilder* aBuilder, const prt::RuleFileInfo* info) {
	MStatus           stat;
	MStatus           stat2;
	MFnNumericData    numericData;
	MFnTypedAttribute tAttr;
	MFnStringData     attrDefaultStr;
	PRTNode*          prtNode = (PRTNode*)node.userNode();

	prt::Attributable* attrs = aBuilder->createAttributable();
	prt::InitialShape* shape = prt::InitialShape::create(
			UnitQuad_vertices, 
			UnitQuad_vertexCount, 
			UnitQuad_indices, 
			UnitQuad_indexCount,
			UnitQuad_faceCounts, 
			UnitQuad_faceCountsCount,
			attrs);						

	for(size_t i = 0; i < info->getNumAttributes(); i++) {
		PRTEnum* e          = 0;
		bool     createAttr = false;
		const MString  name       = MString(info->getAttribute(i)->getName());
		MObject  attr;
		switch(info->getAttribute(i)->getReturnType()) {
		case prt::CGA_BOOL: {
				for(size_t a = 0; a < info->getAttribute(i)->getNumAnnotations(); a++) {
					const prt::Annotation* an = info->getAttribute(i)->getAnnotation(a);
					if(!(wcscmp(an->getName(), L"@Range")))
						e = new PRTEnum(prtNode, an);
				}
				bool value;
				prt::ProceduralRT::evalBool(shape, prtNode->resolveMap, info->getAttribute(i)->getName(), &value);

				if(e) {
					M_CHECK(addEnumParameter(node, attr, name, value, e));
				} else {
					M_CHECK(addBoolParameter(node, attr, name, value));
				}
			break;
			}
		case prt::CGA_FLOAT: {
				double min = std::numeric_limits<double>::quiet_NaN();
				double max = std::numeric_limits<double>::quiet_NaN();
				for(size_t a = 0; a < info->getAttribute(i)->getNumAnnotations(); a++) {
					const prt::Annotation* an = info->getAttribute(i)->getAnnotation(a);
					if(!(wcscmp(an->getName(), L"@Range"))) {
						if(an->getNumArguments() == 2 && an->getArgument(0)->getType() == prt::CGA_FLOAT && an->getArgument(1)->getType() == prt::CGA_FLOAT) {
							min = an->getArgument(0)->getFloat();
							max = an->getArgument(1)->getFloat();
						} else
							e = new PRTEnum(prtNode, an);
					}
				}
				double value;
				prt::ProceduralRT::evalFloat(shape, prtNode->resolveMap, info->getAttribute(i)->getName(), &value);

				if(e) {
					M_CHECK(addEnumParameter(node, attr, name, value, e));
				} else {
					M_CHECK(addFloatParameter(node, attr, name, value, min, max));
				}
				break;
			}
		case prt::CGA_STR: {
				MString exts;
				bool    asFile  = false;
				bool    asColor = false;
				for(size_t a = 0; a < info->getAttribute(i)->getNumAnnotations(); a++) {
					const prt::Annotation* an = info->getAttribute(i)->getAnnotation(a);
					if(!(wcscmp(an->getName(), L"@Range")))
						e = new PRTEnum(prtNode, an);
					else if(!(wcscmp(an->getName(), L"@Color")))
						asColor = true;
					else if(!(wcscmp(an->getName(), L"@Directory"))) {
						exts = MString(an->getName());
						asFile = true;
					} else if(!(wcscmp(an->getName(), L"@File"))) {
						asFile = true;
						for(size_t arg = 0; arg < an->getNumArguments(); arg++) {
							if(an->getArgument(arg)->getType() == prt::CGA_STR) {
								exts += MString(an->getArgument(arg)->getStr());
								exts += " (*.";
								exts += MString(an->getArgument(arg)->getStr());
								exts += ");;";
							}
						}
					}
				}
				wchar_t* value = new wchar_t[4096];
				prt::ProceduralRT::evalStr(shape, prtNode->resolveMap, info->getAttribute(i)->getName(), value, 4095);

				MString mvalue(value);
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

				delete[] value;
				break;
			}
		}

		for(size_t a = 0; a < info->getAttribute(i)->getNumAnnotations(); a++) {
			const prt::Annotation* an = info->getAttribute(i)->getAnnotation(a);
			if(!(wcscmp(an->getName(), L"@Group"))) {
				MFnAttribute fAttr(attr);
				for(size_t arg = 0; arg < an->getNumArguments(); arg++) {
					if(an->getArgument(arg)->getType() == prt::CGA_STR)
						M_CHECK(fAttr.addToCategory(MString(an->getArgument(arg)->getStr())));
				}
			}
		}
	}

	shape->destroy();
	attrs->destroy();

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
	updateRuleFiles(fNode, getStringParameter(prtNode, ((PRTNode*)fNode.userNode())->rulePkg, sRulePkg));

	return MS::kSuccess;
}

void* PRTAttrs::creator() {
	return new PRTAttrs;
}
