/**
 * Esri CityEngine SDK Maya Plugin Example
 *
 * This example demonstrates the main functionality of the Procedural Runtime API.
 * Esri R&D Center Zurich, Switzerland
 *
 * See http://github.com/ArcGIS/esri-cityengine-sdk for instructions.
 */


#ifndef PRT4MAYA_NODE_H_
#define PRT4MAYA_NODE_H_

#include <cmath>

#include <cstdlib>
#include <vector>
#include <algorithm>

#ifdef _MSC_VER
#include <float.h>
#endif

#include <maya/MPxNode.h> 
#include <maya/MString.h> 
#include <maya/MTypeId.h> 
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MAngle.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MFnStringData.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFloatPoint.h>
#include <maya/MFloatPointArray.h>
#include <maya/MIntArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MItMeshVertex.h>
#include <maya/MPxCommand.h>
#include <maya/MArgList.h>
#include <maya/MIOStream.h>
#include <maya/MStringArray.h>
#include <maya/MPlugArray.h>
#include <maya/MGlobal.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MSelectionList.h>
#include <maya/MDagPath.h>
#include <maya/MFnSet.h>
#include <maya/MItMeshPolygon.h>

#include "prt/RuleFileInfo.h"
#include "prt/AttributeMap.h"
#include "prt/LogHandler.h"
#include "prt/API.h"
#include "wrapper/MayaOutputHandler.h"

#ifdef _MSC_VER
#include <WinNT.h>
#endif

#define PRT_TYPE_ID 0x8666b

static const MString  NAME_RULE_PKG      ("Rule_Package");
static const MString  NAME_RULE_FILE     ("Rule_File");
static const MString  NAME_START_RULE    ("Start_Rule");
static const MString  NAME_RPK           ("CGA_Rule_Package");
static const wchar_t* FILE_PREFIX      = L"file://";
static const wchar_t* FLEXNET_LIB      = L"flexnet_prt";
static const wchar_t* ANNOT_START_RULE = L"@StartRule";
static const wchar_t* ANNOT_RANGE      = L"@Range";
static const wchar_t* ANNOT_COLOR      = L"@Color";
static const wchar_t* ANNOT_DIR        = L"@Directory";
static const wchar_t* ANNOT_FILE       = L"@File";
static const wchar_t* ANNOT_GROUP      = L"@Group";
static const wchar_t* ENC_MAYA         = L"com.esri.prt.codecs.maya.MayaEncoder";
static const wchar_t* ENC_ATTR         = L"com.esri.prt.core.AttributeEvalEncoder";

class PRTNode;

class PRTEnum {
	friend class PRTAttrs;
	friend class PRTNode;
public:

	PRTEnum(PRTNode * node, const prt::Annotation* annot = 0);
	~PRTEnum() {}; 

	void    add(const MString & key, const MString & value);
	MStatus fill();

	PRTEnum*               mNext;
	MFnEnumAttribute       mAttr;
private:
	const prt::Annotation* mAnnot;
	MStringArray           mKeys;
	MStringArray           mSVals;
	MDoubleArray           mFVals;
	MIntArray              mBVals;
};

class PRTNode : public MPxNode {
	friend class PRTEnum;
public:
	PRTNode();
	virtual ~PRTNode();

	virtual MStatus         compute( const MPlug& plug, MDataBlock& data );
	virtual MStatus         setDependentsDirty(const MPlug &plugBeingDirtied, MPlugArray &affectedPlugs);

	static  void *          creator();
	static  MStatus         initialize();

public: 
	void               destroyEnums();
	const PRTEnum *    findEnum(const MObject & attr) const;
	MStatus            attachMaterials();
	static void        initLogger();
	static void        uninitialize();
	MayaOutputHandler* createOutputHandler(const MPlug* plug, MDataBlock* data);

	MObject                        mRuleFile;
	MObject                        mStartRule;

	std::wstring                   mLRulePkg;
	const prt::ResolveMap*         mResolveMap;
	const prt::AttributeMap*       mGenerateAttrs;
	const prt::AttributeMap*       mMayaEncOpts;
	const prt::AttributeMap*       mAttrEncOpts;

	static  MTypeId                theID;
	static  MObject                theRulePkg;
	static  MObject                inMesh;
	static  MObject                outMesh;
	static prt::CacheObject*       theCache;
private:
	PRTEnum*                       mEnums;
	bool                           mHasMaterials;
	MStringArray                   mShadingGroups;
	MIntArray                      mShadingRanges;

	static prt::ConsoleLogHandler* theLogHandler;
	static prt::FileLogHandler* theFileLogHandler;
	static const prt::Object*      theLicHandle;

	MString& getStrParameter(MObject & attr, MString & value);
	MStatus  updateShapeAttributes();
};

class PRTAttrs : public MPxCommand {
public:
	MStatus doIt( const MArgList& args );
	static void* creator();
private:
	static MString& getStringParameter(MObject & node, MObject & attr, MString & value);
	static MStatus  setStringParameter(MObject & node, MObject & attr, MString & value);
	MStatus         addBoolParameter(MFnDependencyNode & node, MObject & attr, const MString & name, bool val);
	MStatus         addFloatParameter(MFnDependencyNode & node,  MObject & attr, const MString & name, double val, double min, double max);
	MStatus         addStrParameter(MFnDependencyNode & node,  MObject & attr, const MString & name, MString & attrDefault);
	MStatus         addFileParameter(MFnDependencyNode & node,  MObject & attr, const MString & name, MString & attrDefault);
	MStatus         addEnumParameter(MFnDependencyNode & node,  MObject & attr, const MString & name, bool value, PRTEnum * e);
	MStatus         addEnumParameter(MFnDependencyNode & node,  MObject & attr, const MString & name, double value, PRTEnum * e);
	MStatus         addEnumParameter(MFnDependencyNode & node,  MObject & attr, const MString & name, MString value, PRTEnum * e);
	MStatus         addEnumParameter(MFnDependencyNode & node,  MObject & attr, const MString & name, short value, PRTEnum * e);
	MStatus         addColorParameter(MFnDependencyNode & node,  MObject & attr, const MString & name, MString& attrDefault);
	MStatus         addParameter(MFnDependencyNode & node, MObject & attr ,  MFnAttribute& tAttr);
	MStatus         updateRuleFiles(MFnDependencyNode & node, MString & rulePkg);
	MStatus         updateStartRules(MFnDependencyNode & node, MStringArray & ruleFiles);
	MStatus         createAttributes(MFnDependencyNode & node, MString & ruleFile, MString & startRule, prt::AttributeMapBuilder* aBuilder, const prt::RuleFileInfo* info);
	static MString  longName(const MString & attrName);
	static MString  briefName(const MString & attrName);
};

class PRTMaterials : public MPxCommand
{
public:
	MStatus doIt( const MArgList& args );
	static void* creator();
private:
};

#endif /* PRT4MAYA_NODE_H_ */
