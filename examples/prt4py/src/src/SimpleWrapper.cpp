#include "SimpleWrapper.h"

#include "prt/API.h"
#include "prt/FileOutputCallbacks.h"
#include "prt/StringUtils.h"
#include "prt/FlexLicParams.h"

#include <vector>
#include <cstring>
#include <iostream>
#include <fstream>
#include <iterator>


namespace {

	wchar_t const* createUTF16FromOS(char const* s) {
		size_t wl = strlen(s) + 1;
		size_t len = wl;
		wchar_t* w = new wchar_t[wl];
		prt::StringUtils::toUTF16FromOSNarrow(s, w, &len);
		if (len != wl) {
			delete [] w;
			w = new wchar_t[len];
			prt::StringUtils::toUTF16FromOSNarrow(s, w, &len);
		}
		return w;
	}
	
	std::string objectToXML(prt::Object const* obj) {
		if (obj == 0) return "object pointer is not valid";
		
		
		
		const size_t siz = 4096;
		size_t actualSiz = siz;
		std::string buffer(siz, ' ');
		obj->toXML((char*)&buffer[0], &actualSiz);
		buffer.resize(actualSiz-1); // ignore terminating 0
		if(siz != actualSiz)
			obj->toXML((char*)&buffer[0], &actualSiz);
		return buffer;
	}

	struct PRTContext {
		prt::Object const* mLicHandle;
		prt::ConsoleLogHandler* mConsoleLogHandler;
		
		PRTContext() : mLicHandle(0), mConsoleLogHandler(0) {
			mConsoleLogHandler = prt::ConsoleLogHandler::create(prt::LogHandler::ALL, prt::LogHandler::ALL_COUNT);
			//prt::addLogHandler(mConsoleLogHandler);
		}
		~PRTContext() {
			//prt::removeLogHandler(mConsoleLogHandler);
			mConsoleLogHandler->destroy();
			mLicHandle->destroy();
		}
	};
	
	PRTContext* theContext = 0;
	const bool DBG = true;
	
} // anonymous namespace


int _prtInit(const char* prtPath, const char* licType) {
	theContext = new PRTContext();
	
	std::string prtRoot(prtPath);
	std::string prtExtPath = prtRoot + "/lib";
	std::string prtActLib = prtRoot + "/bin/libflexnet_prt.so";
	
	prt::FlexLicParams lp;
	lp.mActLibPath = prtActLib.c_str(); // NOTE: what encoding?
	lp.mFeature = licType;
	lp.mHostName = ""; // TODO
	
	wchar_t const* wprtExtPath = createUTF16FromOS(prtExtPath.c_str());
	wchar_t const* prtPlugins[1] = { wprtExtPath };

	prt::Status status = prt::STATUS_UNSPECIFIED_ERROR;
	theContext->mLicHandle = prt::init(prtPlugins, 1, prt::LOG_DEBUG, &lp, &status);

	delete [] wprtExtPath;

	return static_cast<int>(status);
}

int _prtGenerate(const char* rulePackageURI, InitialShapeData* initialShapes, size_t initialShapeCount, const char* encoderID, const char* outputPath) {
	if (DBG) {
		std::cout << ">>> _prtGenerate" << std::endl;
		std::cout << "    rulePackageURI = " << rulePackageURI << std::endl;
		std::cout << "    #initialshapes = " << initialShapeCount << std::endl;
		std::cout << "    encoderID      = " << encoderID << std::endl;
		std::cout << "    outputPath     = " << outputPath << std::endl;
	}
	
	prt::Status status = prt::STATUS_UNSPECIFIED_ERROR;
	wchar_t const* wrulePackageURI = createUTF16FromOS(rulePackageURI);
	prt::ResolveMap const* resolveMap = prt::createResolveMap(wrulePackageURI, 0, &status);
	delete [] wrulePackageURI;
	if (status != prt::STATUS_OK)
		return status;
	
	status = prt::STATUS_UNSPECIFIED_ERROR;

	prt::AttributeMapBuilder* amb = prt::AttributeMapBuilder::create();
	prt::AttributeMap const* attributes = amb->createAttributeMapAndReset();
	
	std::vector<prt::InitialShape const*> initialShapePtrs(initialShapeCount);
	prt::InitialShapeBuilder* isb = prt::InitialShapeBuilder::create();
	for (size_t i = 0; i < initialShapeCount; i++) {
 		const InitialShapeData& isd = initialShapes[i];
		if (DBG) {
			std::cout << ">>> initial shape = " << isd.mName << std::endl;
			std::cout << "    ruleFile      = " << isd.mRuleFile << std::endl;
			std::cout << "    mStartRule    = " << isd.mStartRule << std::endl;
			std::cout << "    mRandomSeed   = " << isd.mRandomSeed << std::endl;
			std::cout << "    #vtx count    = " << isd.mVertexCoordsCount << std::endl;
			std::cout << "    vertices      = [ ";
			std::copy(isd.mVertexCoords, isd.mVertexCoords+isd.mVertexCoordsCount, std::ostream_iterator<double>(std::cout, " "));
			std::cout << "]" << std::endl;
			
			std::cout << "    #idx count    = " << isd.mIndicesCount << std::endl;
			std::cout << "    vtx indices   = [ ";
			std::copy(isd.mIndices, isd.mIndices+isd.mIndicesCount, std::ostream_iterator<uint32_t>(std::cout, " "));
			std::cout << "]" << std::endl;
				
			std::cout << "    #face count   = " << isd.mFaceCountsCount << std::endl;
		}
		
		const wchar_t* wname = createUTF16FromOS(isd.mName);
		const wchar_t* wruleFile = createUTF16FromOS(isd.mRuleFile);
		const wchar_t* wstartRule = createUTF16FromOS(isd.mStartRule);
		prt::Status s = isb->setAttributes(wruleFile, wstartRule, isd.mRandomSeed, wname, attributes, resolveMap);
		if (s != prt::STATUS_OK)
			std::cout << "initial shape setAttributes: " << prt::getStatusDescription(s) << std::endl;
		delete [] wstartRule;
		delete [] wruleFile;
		delete [] wname;
		
		s = isb->setGeometry(isd.mVertexCoords, isd.mVertexCoordsCount, isd.mIndices, isd.mIndicesCount, isd.mFaceCounts, isd.mFaceCountsCount);
		if (s != prt::STATUS_OK)
			std::cout << "initial shape setGeometry: " << prt::getStatusDescription(s) << std::endl;
		
		initialShapePtrs[i] = isb->createInitialShapeAndReset(&s);
		if (s != prt::STATUS_OK)
			std::cout << "createInitialShapeAndReset: " << prt::getStatusDescription(s) << std::endl;
		
// 		if (DBG)std::cout << objectToXML(initialShapePtrs[i]) << std::endl;
 	}
 	isb->destroy();
 		
	const wchar_t* wencoderID = createUTF16FromOS(encoderID);
	const wchar_t* allEncoderIDs[1] = { wencoderID };
	const wchar_t* woutputPath = createUTF16FromOS(outputPath);
	
	prt::EncoderInfo const* encInfo = prt::createEncoderInfo(wencoderID);
	prt::AttributeMap const* encOptsIn = amb->createAttributeMapAndReset();
	prt::AttributeMap const* encOpts = 0;
	encInfo->createValidatedOptionsAndStates(encOptsIn, &encOpts);
	encOptsIn->destroy();
	encInfo->destroy();
	
	prt::AttributeMap const* allEncOpts[1] = { encOpts };
	
	prt::CacheObject* cache = prt::CacheObject::create(prt::CacheObject::CACHE_TYPE_DEFAULT);
	prt::FileOutputCallbacks* foh = prt::FileOutputCallbacks::create(woutputPath);
	status = prt::generate(&initialShapePtrs[0], initialShapePtrs.size(), 0, allEncoderIDs, 1, allEncOpts, foh, cache, 0, 0);
	cache->destroy();
	foh->destroy();
	
	encOpts->destroy();

	delete [] woutputPath;
	delete [] wencoderID;
		
	for (prt::InitialShape const*  is: initialShapePtrs)
		is->destroy();

	attributes->destroy();
	amb->destroy();
	resolveMap->destroy();
	
	return status;
}

const char* _prtGetStatusMessage(int status) {
	return prt::getStatusDescription(static_cast<prt::Status>(status));
}

int _prtRelease() {
	delete theContext;
	return prt::STATUS_OK;
}
