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


const char* filename(const char* path) {
	while(*(--path) != '\\');
	return path + 1;
}

MTypeId PRTNode::id(PRT_TYPE_ID);

PRTNode::PRTNode() :  resolveMap(0), generateAttrs(0), generateOpts(0), enums(0), hasMaterials(false) {
	prt::AttributeMapBuilder* oBuilder = prt::AttributeMapBuilder::create();
	generateOpts  = oBuilder->createAttributeMap();
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
	MStatus stat;
	hasMaterials = false;

	std::wstring path(RPK_PREFIX);
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

		size_t size = 8192;
		char* tmp = new char[size];
		tmp[0] = 0;
		DBG("%s\n", generateAttrs->toXML(tmp, &size));
		delete[] tmp;

		MayaOutputHandler mdata(&plug, &data, &shadingGroups, &shadingRanges);
		const prt::InitialShape* shape = prt::InitialShapeBuilder::create(va, vertices.length() * 3, ia, pconnect.length(), ca, pcounts.length(),
				// hacky
				generateAttrs->getString(L"ruleFile"),
				generateAttrs->getString(L"startRule"),
				666,
				L"",

				generateAttrs);

		const wchar_t* encoders[] = { L"com.esri.prt.codecs.maya.MayaEncoder" };
		const prt::AttributeMap* encOpts[] = { generateOpts };
		prt::ProceduralRT::generate(&shape, 1, resolveMap, encoders, 1, encOpts, &mdata);

		// TODO: Error handling

		shape->destroy();

		delete[] ca;
		delete[] ia;
		delete[] va;

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
	fprintf(stderr, "prt4maya: module %s loaded\n", dl_info.dli_fname);

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
	std::wstring root = getPluginRoot();
	//DBGL("prt dir %ls\n", root.c_str());

	prt::Status status = prt::ProceduralRT::init(root.c_str(), prt::LOG_TRACE);

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
	MFnStringData		  stringData;
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

	MString dummy;
	aBuilder->setString(L"ruleFile",  getStrParameter(ruleFile,  dummy).asWChar());
	aBuilder->setString(L"startRule", getStrParameter(startRule, dummy).asWChar());

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


void PRTNode::initLogger() {
	prt::LogLevel logLevels[6] = {prt::LOG_FATAL, prt::LOG_ERROR, prt::LOG_WARNING, prt::LOG_INFO, prt::LOG_DEBUG, prt::LOG_TRACE};
	logHandler = prt::ConsoleLogHandler::create(&logLevels[0], (size_t)6);
	prt::ProceduralRT::addLogHandler(logHandler);
}


void PRTNode::clearLogger() {
	prt::ProceduralRT::removeLogHandler(logHandler);
	logHandler->destroy();
}

// Plug-in Initialization //

MStatus initializePlugin( MObject obj ){
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

	PRTNode::clearLogger();

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
