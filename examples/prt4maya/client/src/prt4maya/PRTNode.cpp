/**
 * Esri CityEngine SDK Maya Plugin Example
 *
 * This example demonstrates the main functionality of the Procedural Runtime API.
 * Esri R&D Center Zurich, Switzerland
 *
 * See http://github.com/ArcGIS/esri-cityengine-sdk for instructions.
 */

#include "Utilities.h"

#include <sstream>

#include <maya/MFnPlugin.h>

#include <maya/MFnTransform.h>
#include <maya/MFnSet.h>
#include <maya/MFnPhongShader.h>
#include <maya/MDGModifier.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MDagPath.h>
#include <maya/MItDependencyNodes.h>

#include "wrapper/MayaOutputHandler.h"

#include "prt4maya/PRTNode.h"

#include "prt/FlexLicParams.h"


namespace {
static const bool ENABLE_LOG_CONSOLE	= true;
static const bool ENABLE_LOG_FILE		= false;
}


MTypeId PRTNode::theID(PRT_TYPE_ID);

PRTNode::PRTNode() : mResolveMap(0), mGenerateAttrs(0), mMayaEncOpts(0), mAttrEncOpts(0), mEnums(0), mHasMaterials(false) {
	prt::AttributeMapBuilder* b = prt::AttributeMapBuilder::create();
	mMayaEncOpts                = b->createAttributeMap();
	mAttrEncOpts                = b->createAttributeMap();
	b->destroy();

	{
		prt::EncoderInfo  const* encInfo = prt::createEncoderInfo(ENC_MAYA);	
		encInfo->createValidatedOptionsAndStates(0, &mMayaEncOpts, 0);
		encInfo->destroy();
	}

	{
		prt::EncoderInfo  const* encInfo = prt::createEncoderInfo(ENC_ATTR);	
		encInfo->createValidatedOptionsAndStates(0, &mAttrEncOpts, 0);
		encInfo->destroy();
	}
}

PRTNode::~PRTNode() {
	DBG("PRTNode disposed\n");
}

MStatus PRTNode::setDependentsDirty(const MPlug& /*plugBeingDirtied*/, MPlugArray& affectedPlugs) {
	MObject thisNode = thisMObject();
	MPlug   pOutMesh(thisNode, outMesh);
	affectedPlugs.append(pOutMesh);
	return MS::kSuccess;
}

MStatus PRTNode::compute( const MPlug& plug, MDataBlock& data ) {
	MStatus stat;
	mHasMaterials = false;

	std::wstring path(FILE_PREFIX);
	MString dummy;
	path.append(getStrParameter(theRulePkg, dummy).asWChar());

	if(mLRulePkg.compare(path)) {
		MString cmd;
		cmd.format("prtAttrs ^1s", name());
		MGlobal::executeCommandOnIdle(cmd);
		return MS::kSuccess;
	}

	if(plug == outMesh && mGenerateAttrs) {
		MDataHandle inputHandle = data.inputValue(inMesh, &stat);
		M_CHECK(stat);
		MObject iMesh = inputHandle.asMeshTransformed();

		updateShapeAttributes();

		MFnMesh iMeshFn(iMesh);

		MFloatPointArray vertices;
		MIntArray        pcounts;
		MIntArray        pconnect;

		iMeshFn.getPoints(vertices);
		iMeshFn.getVertices(pcounts, pconnect);

		double*   va = new double[vertices.length() * 3];
		uint32_t* ia = new uint32_t[pconnect.length()];
		uint32_t* ca = new uint32_t[pcounts.length()];

		for(int i = vertices.length(); --i >= 0;) {
			va[i * 3 + 0] = vertices[i].x;
			va[i * 3 + 1] = vertices[i].y;
			va[i * 3 + 2] = vertices[i].z;
		}
		pconnect.get((int*)ia);
		pcounts.get((int*)ca);

		MayaOutputHandler* outputHandler = createOutputHandler(&plug, &data);
		MString            dummy;

		prt::InitialShapeBuilder* isb = prt::InitialShapeBuilder::create();
		prt::Status setGeoStatus = isb->setGeometry(
				va,
				vertices.length()*3,
				ia,
				pconnect.length(),
				ca,
				pcounts.length()
		);
		if (setGeoStatus != prt::STATUS_OK)
			std::cerr << "InitialShapeBuilder setGeometry failed status = " << prt::getStatusDescription(setGeoStatus) << std::endl;

		isb->setAttributes(
				getStrParameter(mRuleFile,  dummy).asWChar(),
				getStrParameter(mStartRule, dummy).asWChar(),
				isb->computeSeed(),
				L"",
				mGenerateAttrs,
				mResolveMap
		);

		const prt::InitialShape* shape          = isb->createInitialShapeAndReset();
		prt::Status              generateStatus = prt::generate(&shape, 1, 0, &ENC_MAYA, 1, &mMayaEncOpts, outputHandler, PRTNode::theCache, 0);
		if (generateStatus != prt::STATUS_OK)
			std::cerr << "prt generate failed: " << prt::getStatusDescription(generateStatus) << std::endl;
		isb->destroy();
		shape->destroy();

		delete[] ca;
		delete[] ia;
		delete[] va;
		delete   outputHandler;

		data.setClean(plug);

		MString cmd;
		cmd.format("prtMaterials ^1s", name());
		MGlobal::executeCommandOnIdle(cmd);
	}

	return MS::kSuccess;
}

void* PRTNode::creator() {
	return new PRTNode();
}

#ifdef _MSC_VER
const wchar_t SEPERATOR = L'\\';
#else
const wchar_t SEPERATOR = L'/';
#endif

#ifndef _MSC_VER
#include <dlfcn.h>
#include <cstdio>

std::wstring libPath;

__attribute__((constructor))
void on_load(void) {
	Dl_info dl_info;
	dladdr((void *)on_load, &dl_info);

	std::string tmp(dl_info.dli_fname);
	libPath = std::wstring(tmp.length(), L' ');
	std::copy(tmp.begin(), tmp.end(), libPath.begin());
}
#endif

std::wstring getPluginRoot() {
#ifdef _MSC_VER
	wchar_t*  dllPath = new wchar_t[_MAX_PATH];
	wchar_t*  drive   = new wchar_t[8];
	wchar_t*  dir     = new wchar_t[_MAX_PATH];
	wchar_t*  fname   = new wchar_t[_MAX_PATH];
	wchar_t*  ext     = new wchar_t[_MAX_PATH];
	HMODULE   hModule = 0;

	GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)PRTNode::creator, &hModule);

	if(!::GetModuleFileNameW(hModule, (LPWCH)dllPath, _MAX_PATH))
		return L"<error>";

	_wsplitpath_s(dllPath, drive, 8, dir, _MAX_PATH, fname, _MAX_PATH, ext, _MAX_PATH);

	std::wstring root = drive;
	root += dir;

	return root;
#else
	return libPath.substr(0, libPath.find_last_of(SEPERATOR));
#endif
}


MStatus PRTNode::initialize() {
	std::wstring root   = getPluginRoot();
	std::wstring prtLib = root + SEPERATOR + L"prt_lib";

	DBGL(L"prt plugins at %ls\n", prtLib.c_str());

	const wchar_t* prtLibPath = prtLib.c_str();

	std::wstring libfile = getSharedLibraryPrefix() + FLEXNET_LIB + getSharedLibrarySuffix();
	MString      flexLib = MString(root.c_str());
	flexLib             += MString("..\\");
	flexLib             += MString(libfile.c_str());

	prt::FlexLicParams flp;
	flp.mActLibPath    = flexLib.asUTF8();
	flp.mFeature       = "CityEngAdvFx";
	flp.mHostName      = "";
	prt::Status status;

	theLicHandle = prt::init(&prtLibPath, 1, prt::LOG_TRACE, &flp, &status);

	if(status != prt::STATUS_OK)
		return MS::kFailure;

	theCache = prt::CacheObject::create(prt::CacheObject::CACHE_TYPE_DEFAULT);

	MFnTypedAttribute   typedFn;
	MStatus             stat;

	outMesh = typedFn.create( "outMesh", "om", MFnData::kMesh, &stat ); 
	M_CHECK(stat);  
	typedFn.setStorable(false);
	typedFn.setWritable(false);
	stat = addAttribute( outMesh );
	M_CHECK(stat);  

	inMesh = typedFn.create( "inMesh", "im", MFnData::kMesh, &stat ); 
	M_CHECK(stat);  
	typedFn.setStorable(false);
	typedFn.setHidden(true);
	M_CHECK(addAttribute(inMesh));
	M_CHECK(attributeAffects(inMesh, outMesh));

	MStatus           stat2;
	MFnStringData  	  stringData;
	MFnTypedAttribute fAttr;

	theRulePkg = fAttr.create( NAME_RULE_PKG, "rulePkg", MFnData::kString, stringData.create(&stat2), &stat );
	M_CHECK(stat2);
	M_CHECK(stat);
	M_CHECK(fAttr.setUsedAsFilename(true));
	M_CHECK(fAttr.setCached    (true));
	M_CHECK(fAttr.setStorable  (true));
	M_CHECK(fAttr.setNiceNameOverride(MString("Rule Package(*.rpk)")));
	M_CHECK(addAttribute(theRulePkg));
	M_CHECK(attributeAffects(theRulePkg, outMesh ));

	return MS::kSuccess;
}

inline MString & PRTNode::getStrParameter(MObject & attr, MString & value) {
	MPlug plug(thisMObject(), attr);

	if(attr.hasFn(MFn::kNumericAttribute)) {
		double fValue;
		plug.getValue(fValue);
		value.set(fValue);
	} else if(attr.hasFn(MFn::kTypedAttribute)) {
		plug.getValue(value);
	} else if(attr.hasFn(MFn::kEnumAttribute)) {
		short eValue;
		const PRTEnum* e = findEnum(attr);
		if(e) {
			plug.getValue(eValue);
			value = e->mSVals[eValue];
		}
	}
	return value;
}

MStatus PRTNode::updateShapeAttributes() {
	if(!(mGenerateAttrs)) return MS::kSuccess; 

	MStatus           stat;
	MObject           node = thisMObject();
	MFnDependencyNode fNode(node, &stat);
	M_CHECK(stat);

	int count = (int)fNode.attributeCount(&stat);
	M_CHECK(stat);

	prt::AttributeMapBuilder* aBuilder = prt::AttributeMapBuilder::create();

	for(int i = 0; i < count; i++) {
		MObject attr = fNode.attribute(i, &stat);
		if(stat != MS::kSuccess) continue;

		MPlug plug(node, attr);
		if(!(plug.isDynamic())) continue;

		MString name = plug.partialName();
		name = name.substring(name.index('$') + 1, name.length());

		if(attr.hasFn(MFn::kNumericAttribute)) {
			MFnNumericAttribute nAttr(attr);

			if(nAttr.unitType() == MFnNumericData::kBoolean) {
				bool b, db; 
				nAttr.getDefault(db);
				M_CHECK(plug.getValue(b));
				if(b != db)
					aBuilder->setBool(name.asWChar(), b);
			} else if(nAttr.unitType() == MFnNumericData::kDouble) {
				double d, dd; 
				nAttr.getDefault(dd);
				M_CHECK(plug.getValue(d));
				if(d != dd) {
					aBuilder->setFloat(name.asWChar(), d);
				}
			} else if(nAttr.isUsedAsColor()) {
				double r, dr = 0.0;
				double g, dg = 0.0;
				double b, db = 0.0;

				wchar_t dcolor[] = L"#000000";
				toHex(dcolor, dr, dg, db);

				MObject rgb;
				plug.getValue(rgb);
				MFnNumericData fRGB(rgb);
				fRGB.getData(r, g, b);

				wchar_t color[]  = L"#000000";
				toHex(color, r, g, b);

				if(wcscmp(color, dcolor))
					aBuilder->setString(name.asWChar(), color);
			}

		} else if(attr.hasFn(MFn::kTypedAttribute)) {
			MFnTypedAttribute tAttr(attr);
			MString       s;
			MFnStringData dsd;
			MObject       dso = dsd.create(&stat);
			M_CHECK(stat);
			M_CHECK(tAttr.getDefault(dso));

			MFnStringData fDs(dso, &stat);
			M_CHECK(stat);

			M_CHECK(plug.getValue(s));
			if(s != fDs.string(&stat)) {
				M_CHECK(stat);
				aBuilder->setString(name.asWChar(), s.asWChar());
			}
		} else if(attr.hasFn(MFn::kEnumAttribute)) {
			MFnEnumAttribute eAttr(attr);
		}
	}

	mGenerateAttrs->destroy();
	mGenerateAttrs = aBuilder->createAttributeMap();

	aBuilder->destroy();

	return MS::kSuccess;
}

void PRTNode::destroyEnums() {
	for(PRTEnum* e = mEnums; e;) {
		PRTEnum * tmp = e->mNext;
		delete e;
		e = tmp;
	}

	mEnums = 0;
}

const PRTEnum* PRTNode::findEnum(const MObject & attr) const {
	for(PRTEnum* e = mEnums; e; e = e->mNext) {
		if(e->mAttr.object() == attr)
			return e;
	}
	return 0;
}

MayaOutputHandler* PRTNode::createOutputHandler(const MPlug* plug, MDataBlock* data) {
	return new MayaOutputHandler(plug, data, &mShadingGroups, &mShadingRanges);
}

void PRTNode::initLogger() {
	if (ENABLE_LOG_CONSOLE) {
		theLogHandler = prt::ConsoleLogHandler::create(prt::LogHandler::ALL, prt::LogHandler::ALL_COUNT);
		prt::addLogHandler(theLogHandler);
	}

	if (ENABLE_LOG_FILE) {
		std::wstring logPath   = getPluginRoot() + SEPERATOR + L"prt4maya.log";
		theFileLogHandler = prt::FileLogHandler::create(prt::LogHandler::ALL, prt::LogHandler::ALL_COUNT, logPath.c_str());
		prt::addLogHandler(theFileLogHandler);
	}
}


void PRTNode::uninitialize() {
	if (ENABLE_LOG_CONSOLE) {
		prt::removeLogHandler(theLogHandler);
		theLogHandler->destroy();
	}
	if (ENABLE_LOG_FILE) {
		prt::removeLogHandler(theFileLogHandler);
		theFileLogHandler->destroy();
	}
	theLicHandle->destroy();
	theCache->destroy();
}

MStatus initializePlugin( MObject obj ){
#ifdef __linux__
	dlopen("libprt4maya.so", RTLD_LAZY | RTLD_NOLOAD | RTLD_GLOBAL);
#endif

	PRTNode::initLogger();

	MFnPlugin plugin( obj, "Esri", "1.0", "Any");

	M_CHECK(plugin.registerNode("prt", PRTNode::theID, &PRTNode::creator, &PRTNode::initialize, MPxNode::kDependNode));
	M_CHECK(plugin.registerUI("prt4mayaCreateUI", "prt4mayaDeleteUI"));
	M_CHECK(plugin.registerCommand("prtAttrs",     PRTAttrs::creator));
	M_CHECK(plugin.registerCommand("prtMaterials", PRTMaterials::creator));

	return MS::kSuccess;
}

MStatus uninitializePlugin( MObject obj) {
	MFnPlugin plugin( obj );

	M_CHECK(plugin.deregisterCommand("prtMaterials"));
	M_CHECK(plugin.deregisterCommand("prtAttrs"));
	M_CHECK(plugin.deregisterNode(PRTNode::theID));

	PRTNode::uninitialize();

	return MS::kSuccess;
}

// Main shape parameters
//
MObject PRTNode::theRulePkg; 

// Input mesh
//
MObject PRTNode::inMesh;

// Output mesh
//
MObject PRTNode::outMesh;

// statics

prt::ConsoleLogHandler*	PRTNode::theLogHandler = 0;
prt::FileLogHandler*	PRTNode::theFileLogHandler = 0;
const prt::Object*      PRTNode::theLicHandle = 0;
prt::CacheObject*       PRTNode::theCache = 0;
