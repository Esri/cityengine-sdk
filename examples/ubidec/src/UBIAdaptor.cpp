#include "UBIAdaptor.h"

#include "prtx/Log.h"
#include "prtx/PRTUtils.h"

#include "prt/AttributeMap.h"
#include "prt/ContentType.h"
#include "prt/StringUtils.h"

#include "boost/format.hpp"
#include "boost/thread/mutex.hpp"
#include "boost/thread/locks.hpp"
#include "boost/filesystem/fstream.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/filesystem/operations.hpp"

#include <iostream>
#include <sstream>
#include <limits>


const std::wstring UBIAdaptor::ID			= L"com.esri.prt.examples.UBIAdaptor";
const std::wstring UBIAdaptor::NAME			= L"UBI Adaptor";
const std::wstring UBIAdaptor::DESCRIPTION	= L"Resolves URIs from UBI repositories.";


UBIAdaptor::UBIAdaptor() {
}

UBIAdaptor::~UBIAdaptor() {
}

std::istream* UBIAdaptor::createStream(prtx::URIPtr uri) const {
	boost::filesystem::path p = uri->getPath();
	std::wcout << L"UBIAdaptor create stream: " << p << std::endl;
	assert(boost::filesystem::exists(p));

	// read content at uri location and create a stream for the decoder
	boost::filesystem::ifstream fIn(p, std::ios::in);
	std::stringstream ss;
	ss << fIn.rdbuf();
	std::string redirectData = ss.str();

	// TODO: implement actual logic to fetch "real" data stream from redirectData
	//       currently just passing redirectData on to UBIDecoder to see if it arrives...

	return new std::istringstream(redirectData, std::ios_base::in);
}

void UBIAdaptor::destroyStream(std::istream* stream) const {
	delete stream;
	std::wcout << L"UBIAdaptor::destroyStream" << std::endl;
}

prt::ResolveMap const* UBIAdaptor::createResolveMap(prtx::URIPtr uri) const {
	prt::ResolveMapBuilder* rmb = prt::ResolveMapBuilder::create();
	prt::ResolveMap const* rm = rmb->createResolveMap();
	rmb->destroy();
	return rm;
}


UBIAdaptorFactory::UBIAdaptorFactory() {
	log_info("UBIAdaptorFactory");
}

UBIAdaptorFactory::~UBIAdaptorFactory() {
}

bool UBIAdaptorFactory::canHandleURI(prtx::URIPtr uri) const {
	return (uri->getScheme() == prtx::URI::SCHEME_FILE /*&& uri->getBaseName() == L"redirect"*/ && uri->getExtension() == L".ubi");
}
