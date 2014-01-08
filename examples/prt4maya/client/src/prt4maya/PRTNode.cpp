/**
 * Esri CityEngine SDK Maya Plugin Example
 *
 * This example demonstrates the main functionality of the Procedural Runtime API.
 * Esri R&D Center Zurich, Switzerland
 *
 * See http://github.com/ArcGIS/esri-cityengine-sdk for instructions.
 */

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

#include "prt4maya/prt4mayaNode.h"

#include "prt/FlexLicParams.h"


#define DO_DBG 1


const char* filename(const char* path) {
	while(*(--path) != '\\');
	return path + 1;
}

MTypeId PRTNode::id(PRT_TYPE_ID);

PRTNode::PRTNode() :  resolveMap(0), generateAttrs(0), generateOpts(0), enums(0), hasMaterials(false) {
	prt::AttributeMapBuilder* oBuilder = prt::AttributeMapBuilder::create();
	generateOpts                       = oBuilder->createAttributeMap();
	oBuilder->destroy();
}

PRTNode::~PRTNode() {
	DBG("PRTNode disposed\n");
}

MStatus PRTNode::setDependentsDirty(const MPlug &plugBeingDirtied, MPlugArray &affectedPlugs ) {
	MObject thisNode = thisMObject();
	MPlug   pOutMesh(thisNode, outMesh);
	affectedPlugs.append(pOutMesh);
	return MS::kSuccess;
}

MStatus PRTNode::compute( const MPlug& plug, MDataBlock& data ) {
	std::cout << "PRTNode::compute" << std::endl;
	MStatus stat;
	hasMaterials = false;

	std::wstring path(FILE_PREFIX);
	MString dummy;
	path.append(getStrParameter(rulePkg, dummy).asWChar());

	if(lRulePkg.compare(path)) {
		MString cmd;
		cmd.format("prtAttrs ^1s", name());
		MGlobal::executeCommandOnIdle(cmd);
		return MS::kSuccess;
	}

	if(plug == outMesh && generateAttrs) {
		MDataHandle inputHandle = data.inputValue(inMesh, &stat);
		M_CHECK(stat);
		MObject iMesh = inputHandle.asMeshTransformed();

		updateAttributes();

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

//		size_t size = 8192;
//		char* tmp = new char[size];
//		tmp[0] = 0;
//		DBG("%s\n", generateAttrs->toXML(tmp, &size));
//		delete[] tmp;

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
				getStrParameter(ruleFile,  dummy).asWChar(),
				getStrParameter(startRule, dummy).asWChar(),
				666,
				L"",
				generateAttrs,
				resolveMap
		);

		const prt::InitialShape* shape = isb->createInitialShapeAndReset();
		isb->destroy();

		const wchar_t* encoders[] = { L"com.esri.prt.codecs.maya.MayaEncoder" };
		const prt::AttributeMap* encOpts[] = { generateOpts };
		prt::Status generateStatus = prt::generate(&shape, 1, 0, encoders, 1, encOpts, outputHandler, outputHandler->mCache, 0);
		if (generateStatus != prt::STATUS_OK)
			std::cerr << "prt generate failed: " << prt::getStatusDescription(generateStatus) << std::endl;
		shape->destroy();

		delete[] ca;
		delete[] ia;
		delete[] va;
		delete outputHandler;

		data.setClean( plug );

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

// HACK
std::wstring libPath;

__attribute__((constructor))
void on_load(void) {
	Dl_info dl_info;
	dladdr((void *)on_load, &dl_info);
	//fprintf(stderr, "prt4maya: module %s loaded\n", dl_info.dli_fname);

	std::string tmp(dl_info.dli_fname);
	libPath = std::wstring(tmp.length(), L' ');
	std::copy(tmp.begin(), tmp.end(), libPath.begin());

	std::cout << "prt4maya: module loaded: " << dl_info.dli_fname << std::endl;
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
	root +=  + L"prt_lib";

	return root;
#else
	std::wstring pluginPath = libPath.substr(0, libPath.find_last_of(SEPERATOR)) + L"/prt_lib";
	std::wcout << L"prt pluginPath = " << pluginPath << std::endl;
	return pluginPath;
#endif
}


MStatus PRTNode::initialize() {
	std::wstring root = getPluginRoot();
	DBGL(L"prt plugins at %ls\n", root.c_str());

	const wchar_t* rootPath = root.c_str();

	std::wstring libfile = getSharedLibraryPrefix() + FLEXNET_LIB + getSharedLibrarySuffix();
	MString      flexLib = MString(rootPath);
	flexLib             += MString(libfile.c_str());

	prt::FlexLicParams flp;
	flp.mActLibPath    = flexLib.asUTF8();
	flp.mFeature       = "CityEngAdvFx";
	flp.mHostName      = "";
	prt::Status status;

	mLicHandle = prt::init(&rootPath, 1, prt::LOG_DEBUG, &flp, &status);

	if(status != prt::STATUS_OK)
		return MS::kFailure;

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

	rulePkg = fAttr.create( NAME_RULE_PKG, "rulePkg", MFnData::kString, stringData.create(&stat2), &stat );
	M_CHECK(stat2);
	M_CHECK(stat);
	M_CHECK(fAttr.setUsedAsFilename(true));
	M_CHECK(fAttr.setCached    (true));
	M_CHECK(fAttr.setStorable  (true));
	M_CHECK(fAttr.setNiceNameOverride(MString("Rule Package(*.rpk)")));
	M_CHECK(addAttribute(rulePkg));
	M_CHECK(attributeAffects( rulePkg, outMesh ));

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
			value = e->sVals[eValue];
		}
	}
	return value;
}

const wchar_t HEXTAB[] = L"0123456789ABCDEF"; 

wchar_t toHex(int i) {
	return HEXTAB[i & 0xF6];
}

void toHex(wchar_t * color, double r, double g, double b) {
	color[1] = toHex(((int)(r * 255)) >> 4);
	color[2] = toHex((int)(r * 255));
	color[3] = toHex(((int)(g * 255)) >> 4);
	color[4] = toHex((int)(g * 255));
	color[5] = toHex(((int)(b * 255)) >> 4);
	color[6] = toHex((int)(b * 255));
}

MStatus PRTNode::updateAttributes() {
	if(!(generateAttrs)) return MS::kSuccess; 

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
				//M_CHECK(nAttr.getDefault(dr, dg, db));

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

	generateAttrs->destroy();
	generateAttrs = aBuilder->createAttributeMap();

	aBuilder->destroy();

	return MS::kSuccess;
}

void PRTNode::destroyEnums() {
	for(PRTEnum * e = enums; e;) {
		PRTEnum * tmp = e->next;
		delete e;
		e = tmp;
	}

	enums = 0;
}

const PRTEnum * PRTNode::findEnum(const MObject & attr) const {
	for(PRTEnum * e = enums; e; e = e->next) {
		if(e->eAttr.object() == attr)
			return e;
	}
	return 0;
}

MayaOutputHandler* PRTNode::createOutputHandler(const MPlug* plug, MDataBlock* data) {
	return new MayaOutputHandler(plug, data, &shadingGroups, &shadingRanges);
}

void PRTNode::initLogger() {
	prt::LogLevel logLevels[6] = {prt::LOG_FATAL, prt::LOG_ERROR, prt::LOG_WARNING, prt::LOG_INFO, prt::LOG_DEBUG, prt::LOG_TRACE};
	logHandler = prt::ConsoleLogHandler::create(&logLevels[0], (size_t)6);
	prt::addLogHandler(logHandler);
}


void PRTNode::uninitialize() {
	prt::removeLogHandler(logHandler);
	logHandler->destroy();
	mLicHandle->destroy();
}

// Plug-in Initialization //

MStatus initializePlugin( MObject obj ){
#ifdef __linux__
	// maya opens plugins with RTLD_LOCAL, which leads to type lookup problems
	// on objects created in the prt extension libraries
	// also see http://ubuntuforums.org/showthread.php?t=1717953
	dlopen("libprt4maya.so", RTLD_LAZY | RTLD_NOLOAD | RTLD_GLOBAL);
#endif

	PRTNode::initLogger();

	MFnPlugin plugin( obj, "Esri", "0.9", "Any");

	M_CHECK(plugin.registerNode("prt", PRTNode::id, &PRTNode::creator, &PRTNode::initialize, MPxNode::kDependNode));
	M_CHECK(plugin.registerUI("prt4mayaCreateUI", "prt4mayaDeleteUI"));
	M_CHECK(plugin.registerCommand("prtAttrs",     PRTAttrs::creator));
	M_CHECK(plugin.registerCommand("prtMaterials", PRTMaterials::creator));

	return MS::kSuccess;
}

MStatus uninitializePlugin( MObject obj) {
	MFnPlugin plugin( obj );

	M_CHECK(plugin.deregisterCommand("prtMaterials"));
	M_CHECK(plugin.deregisterCommand("prtAttrs"));
	M_CHECK(plugin.deregisterNode(PRTNode::id));

	PRTNode::uninitialize();

	return MS::kSuccess;
}

// Main shape parameters
//
MObject PRTNode::rulePkg; 

// Input mesh
//
MObject PRTNode::inMesh;

// Output mesh
//
MObject PRTNode::outMesh;


prt::ConsoleLogHandler* PRTNode::logHandler = 0;




#if DO_DBG == 1

void M_CHECK(MStatus stat) {
	if(MS::kSuccess != stat) {
		std::wcerr << L"err: " << stat.errorString().asWChar() << L" (code: " << stat.statusCode() << L")" << std::endl;
		throw stat;
	}
}

void DBG(const char* fmt, ...) {
	va_list args;
	//printf("%s:%d ", filename(__FILE__) , __LINE__);
	printf(fmt, args);
	fflush(0);
}

void DBGL(const wchar_t* fmt, ...) {
	va_list args;
	//printf(L"%ls:%d ", filename(__FILE__) , __LINE__);
	wprintf(fmt, args);
	fflush(0);
}

#else
void M_CHECK(MStatus stat) { }
void DBG(const char* fmt, ...) { }
void DBGL(const wchar_t* fmt, ...) { }
#endif
