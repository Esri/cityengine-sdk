#include "UBIDecoder.h"

#include "prtx/Shape.h"
#include "prtx/ShapeIterator.h"
#include "prtx/GenerateContext.h"
#include "prtx/Geometry.h"
#include "prtx/EncodeOptions.h"
#include "prtx/EncoderInfoBuilder.h"
#include "prtx/DataBackend.h"

#include "prt/StringUtils.h"

#include "boost/foreach.hpp"

#include <ostream>
#include <sstream>
#include <fstream>


const std::wstring UBIDecoder::ID			= L"com.esri.prt.examples.UBIDecoder";
const std::wstring UBIDecoder::NAME			= L"Ubisoft Example Encoder";
const std::wstring UBIDecoder::DESCRIPTION	= L"Example decoder for custom ubisoft format";


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
	std::wcout << L"UBIDecoder::decode" << std::endl;

	std::istreambuf_iterator<char> eos;
	std::string data(std::istreambuf_iterator<char>(stream), eos);
	// TODO: implement custom decoder logic for 'data'

	prtx::GeometryBuilder gb;
	// TODO: fill gb with geometry... (currently, ce will put a fallback geometry 'unknown asset')
	results.push_back(boost::static_pointer_cast<prtx::Content>(gb.createShared()));

	std::wcout << L"UBIDecoder::decode done" << std::endl;
}


UBIDecoder* UBIDecoderFactory::create() const {
	return new UBIDecoder();
}
