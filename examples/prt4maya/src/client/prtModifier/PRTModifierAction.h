#ifndef _PRTFty
#define _PRTFty


#include "polyModifierBase/polyModifierFty.h"
#include "prt/API.h"
#include <maya/MObject.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MStringArray.h>
#include <maya/MIntArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MString.h>
#include <maya/MPlugArray.h>
#include <map>

class PRTModifierAction;

class PRTModifierEnum {
	friend class PRTModifierAction;

public:
	PRTModifierEnum(PRTModifierAction* action, const prt::Annotation* annot = nullptr);
    virtual ~PRTModifierEnum() { }

	void add(const MString & key, const MString & value);
	MStatus fill();

public:
	PRTModifierEnum*        mNext;
	MFnEnumAttribute        mAttr;

private:
	const prt::Annotation*  mAnnot;
	MStringArray            mKeys;
	MStringArray            mSVals;
	MDoubleArray            mFVals;
	MIntArray               mBVals;
}; // class PRTModifierEnum


class PRTModifierAction : public polyModifierFty
{
	friend class PRTModifierEnum;

public:
	PRTModifierAction();
	~PRTModifierAction() override;

	MStatus updateRuleFiles(MObject& node, const MString& rulePkg);
	void fillAttributesFromNode(const MObject& node);
	void setMesh(MObject& _inMesh, MObject& _outMesh);


	// polyModifierFty inherited methods
	MStatus		doIt() override;

	//init in PRTModifierPlugin::initializePlugin, destroyed in PRTModifierPlugin::uninitializePlugin
	static const prt::Object*       thePRT;
	static prt::CacheObject*        theCache;
	static prt::ConsoleLogHandler*  theLogHandler;
	static prt::FileLogHandler*     theFileLogHandler;
	static const std::string&       getPluginRoot();


private:

	//init in PRTModifierAction::PRTModifierAction()
	const prt::AttributeMap* mMayaEncOpts;
	const prt::AttributeMap* mAttrEncOpts;

	// Mesh Nodes: only used during doIt
	MObject inMesh;
	MObject outMesh;

	// Set in updateRuleFiles(rulePkg)
	MString                       mRulePkg;
	std::wstring                  mRuleFile;
	std::wstring                  mStartRule;
	const prt::ResolveMap*        mResolveMap;

	//init in fillAttributesFromNode()
	const prt::AttributeMap*      mGenerateAttrs;

	PRTModifierEnum* mEnums;
	std::map<std::wstring, std::wstring> mBriefName2prtAttr;
	MStatus createNodeAttributes(MObject& node, const std::wstring & ruleFile, const std::wstring & startRule, prt::AttributeMapBuilder* aBuilder, const prt::RuleFileInfo* info);
	void destroyEnums();
	static MStatus  addParameter(MFnDependencyNode & node, MObject & attr, MFnAttribute& tAttr);
	static MStatus  addBoolParameter(MFnDependencyNode & node, MObject & attr, const MString & name, bool defaultValue);
	static MStatus  addFloatParameter(MFnDependencyNode & node, MObject & attr, const MString & name, double defaultValue, double min, double max);
	static MStatus  addStrParameter(MFnDependencyNode & node, MObject & attr, const MString & name, const MString & defaultValue);
	static MStatus  addFileParameter(MFnDependencyNode & node, MObject & attr, const MString & name, const MString & defaultValue, const MString & ext);
	static MStatus  addEnumParameter(MFnDependencyNode & node, MObject & attr, const MString & name, bool defaultValue, PRTModifierEnum * e);
	static MStatus  addEnumParameter(MFnDependencyNode & node, MObject & attr, const MString & name, double defaultValue, PRTModifierEnum * e);
	static MStatus  addEnumParameter(MFnDependencyNode & node, MObject & attr, const MString & name, MString defaultValue, PRTModifierEnum * e);
	static MStatus  addEnumParameter(MFnDependencyNode & node, MObject & attr, const MString & name, short defaultValue, PRTModifierEnum * e);
	static MStatus  addColorParameter(MFnDependencyNode & node, MObject & attr, const MString & name, const MString& defaultValue);
	static MString  longName(const MString & attrName);
	static MString  briefName(const MString & attrName);
	template<typename T> static T getPlugValueAndRemoveAttr(MFnDependencyNode & node, const MString & briefName, const T & defaultValue);
};

#endif
