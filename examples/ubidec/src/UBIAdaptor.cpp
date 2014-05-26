#include "UBIAdaptor.h"

#include "prtx/Log.h"
#include "prtx/PRTUtils.h"

#include "prt/AttributeMap.h"
#include "prt/ContentType.h"
#include "prt/StringUtils.h"

#include "boost/filesystem/operations.hpp"
#include "boost/format.hpp"
#include "boost/thread/mutex.hpp"
#include "boost/thread/locks.hpp"

#include <iostream>
#include <sstream>
#include <limits>


// supports uri layout: file:/.../redirect.ubi?bigfile=dbref


const std::wstring UBIAdaptor::ID			= L"com.esri.prt.examples.UBIAdaptor";
const std::wstring UBIAdaptor::NAME			= L"UBI Adaptor";
const std::wstring UBIAdaptor::DESCRIPTION	= L"Resolves URIs from UBI repositories.";


UBIAdaptor::UBIAdaptor() {
}

UBIAdaptor::~UBIAdaptor() {
}

std::istream* UBIAdaptor::createStream(prtx::URIPtr uri) const {
	std::wstring q = uri->getQuery();
	std::string nq(' ', q.length());
	std::copy(q.begin(), q.end(), nq.begin());
	return new std::istringstream(nq);
}

void UBIAdaptor::destroyStream(std::istream* stream) const {
	delete stream;
}

prt::ResolveMap const* UBIAdaptor::createResolveMap(prtx::URIPtr uri) const {
	prt::ResolveMapBuilder* rmb = prt::ResolveMapBuilder::create();
	prt::ResolveMap const* rm = rmb->createResolveMap();
	rmb->destroy();
	return rm;
}


UBIAdaptorFactory::UBIAdaptorFactory() {
}

UBIAdaptorFactory::~UBIAdaptorFactory() {
}

bool UBIAdaptorFactory::canHandleURI(prtx::URIPtr uri) const {
	return (uri->getBaseName() == L"redirect" && uri->getExtension() == L".ubi");
}
