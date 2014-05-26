#include "UBIDecoder.h"

#include "prtx/Shape.h"
#include "prtx/ShapeIterator.h"
#include "prtx/GenerateContext.h"
#include "prtx/Geometry.h"
#include "prtx/EncodeOptions.h"
#include "prtx/EncoderInfoBuilder.h"

#include "boost/foreach.hpp"

#include <ostream>
#include <sstream>


const std::wstring UBIDecoder::ID			= L"com.esri.prt.examples.UBIDecoder";
const std::wstring UBIDecoder::NAME			= L"Ubisoft Example Encoder";
const std::wstring UBIDecoder::DESCRIPTION	= L"Example decoder for the UBI format";


UBIDecoder::UBIDecoder() {
}

UBIDecoder::~UBIDecoder() {
}

void UBIDecoder::decode(
		prtx::ContentPtrVector& results,
		std::istream& stream,
		prt::Cache* cache,
		const std::wstring& key,
		prtx::ResolveMap const* resolveMap,
		std::wstring& warnings
) {
	prtx::URIPtr uri = prtx::ResolveMap::resolveKeyWithURIFallback(resolveMap, key);

	log_winfo(L"UBIDecoder::decode begin: ref = '%s'") % uri->wstring();

	while (stream.good()) {
		std::cout << stream;
	}

//	prtx::GeometryBuilder geoBuilder;
//	BOOST_FOREACH(ProtoMesh& p, meshes) {
//		geoBuilder.addMesh(p.builder.createSharedAndReset(&warnings));
//	}
//	geoBuilder.setURI(uri);
//	results.push_back(boost::static_pointer_cast<prtx::Content>(prtx::GeometryPtr(geoBuilder.createAndReset(&warnings))));

	log_info("UBIDecoder::decode done");
}


UBIDecoder* UBIDecoderFactory::create() const {
	return new UBIDecoder();
}
