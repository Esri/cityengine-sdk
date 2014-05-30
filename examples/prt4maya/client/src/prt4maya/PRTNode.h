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
#include "wrapper/MayaCallbacks.h"

#ifdef _MSC_VER
#	include <WinNT.h>
#endif


#define PRT_TYPE_ID 0x8666b

extern const wchar_t*	ENC_ATTR;
extern const char*	    FILE_PREFIX;
extern const MString	NAME_GENERATE;


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

	virtual MStatus                compute( const MPlug& plug, MDataBlock& data );
	virtual MStatus                setDependentsDirty(const MPlug &plugBeingDirtied, MPlugArray &affectedPlugs);

	static  void *                 creator();
	static  MStatus                initialize();
	static const std::string&      getPluginRoot();

	const PRTEnum *                findEnum(const MObject & attr) const;
	void                           destroyEnums();
	MStatus                        attachMaterials();
	static void                    initLogger();
	static void                    uninitialize();
	MayaCallbacks*             createOutputHandler(const MPlug* plug, MDataBlock* data);

	std::wstring                   mRuleFile;
	std::wstring                   mStartRule;
	MObject                        mGenerate;
	bool                           mCreatedInteractively;

	std::string                    mLRulePkg;
	const prt::ResolveMap*         mResolveMap;
	const prt::AttributeMap*       mGenerateAttrs;
	const prt::AttributeMap*       mMayaEncOpts;
	const prt::AttributeMap*       mAttrEncOpts;

	std::map<std::wstring, std::wstring> mBriefName2prtAttr;

	static MTypeId                 theID;
	static MObject                 rulePkg;
	static MObject                 inMesh;
	static MObject                 outMesh;
	static const prt::Object*      theLicHandle;
	static prt::CacheObject*       theCache;
	static MStringArray            theShadingGroups;

private:
	PRTEnum*                       mEnums;
	bool                           mHasMaterials;
	MStringArray                   mShadingGroups;
	MIntArray                      mShadingRanges;
	MString                        mShadingCmd;

	static prt::ConsoleLogHandler* theLogHandler;
	static prt::FileLogHandler*    theFileLogHandler;
	static int                     theNodeCount;

	MString&            getStrParameter(MObject & attr, MString & value);
	bool                getBoolParameter(MObject & attr);
	MStatus             updateShapeAttributes();
};


class PRTAttrs : public MPxCommand {
public:
	MStatus         doIt(const MArgList& args);
	static void*    creator();
	static MStatus  updateRuleFiles(MFnDependencyNode & node, MString & rulePkg);
private:
	static MString& getStringParameter(MObject & node, MObject & attr, MString & value);
	static MStatus  setStringParameter(MObject & node, MObject & attr, MString & value);
	static MStatus  addBoolParameter(MFnDependencyNode & node, MObject & attr, const MString & name, bool val);
	static MStatus  addFloatParameter(MFnDependencyNode & node,  MObject & attr, const MString & name, double val, double min, double max);
	static MStatus  addStrParameter(MFnDependencyNode & node,  MObject & attr, const MString & name, MString & attrDefault);
	static MStatus  addFileParameter(MFnDependencyNode & node,  MObject & attr, const MString & name, const MString & attrDefault, MString & ext);
	static MStatus  addEnumParameter(MFnDependencyNode & node,  MObject & attr, const MString & name, bool value, PRTEnum * e);
	static MStatus  addEnumParameter(MFnDependencyNode & node,  MObject & attr, const MString & name, double value, PRTEnum * e);
	static MStatus  addEnumParameter(MFnDependencyNode & node,  MObject & attr, const MString & name, MString value, PRTEnum * e);
	static MStatus  addEnumParameter(MFnDependencyNode & node,  MObject & attr, const MString & name, short value, PRTEnum * e);
	static MStatus  addColorParameter(MFnDependencyNode & node,  MObject & attr, const MString & name, MString& attrDefault);
	static MStatus  addParameter(MFnDependencyNode & node, MObject & attr ,  MFnAttribute& tAttr);
	static MStatus  updateStartRules(MFnDependencyNode & node);
	static MStatus  createAttributes(MFnDependencyNode & node, const std::wstring & ruleFile, const std::wstring & startRule, prt::AttributeMapBuilder* aBuilder, const prt::RuleFileInfo* info);
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


class PRTCreate : public MPxCommand
{
public:
	MStatus doIt( const MArgList& args );
	static void* creator();
private:
};

#endif /* PRT4MAYA_NODE_H_ */
