/**
 * Esri CityEngine SDK Custom STL Decoder Example
 *
 * This example demonstrates the usage of the PRTX interface
 * to write custom decoders.
 *
 * See README.md in https://github.com/Esri/esri-cityengine-sdk for build instructions.
 *
 * Written by Simon Haegler
 * Esri R&D Center Zurich, Switzerland
 *
 * Copyright 2012-2019 (c) Esri R&D Center Zurich
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "STLDecoder.h"

#include "prtx/Geometry.h"
#include "prtx/Mesh.h"

#include <sstream>
#include <limits>
#include <map>
#include <thread>
#include <mutex>
#include <memory>


namespace {

const std::wstring ID          = L"com.esri.prt.examples.STLDecoder";
const std::wstring NAME        = L"STL Decoder";
const std::wstring DESCRIPTION = L"Example decoder for the STL format";
const std::wstring EXT         = L".stl";

enum class Token {
	SOLID, FACET, NORMAL, OUTER, LOOP, VERTEX, ENDLOOP, ENDFACET, ENDSOLID, UNKNOWN
};

using TokenMap = std::map<std::string, Token>;
TokenMap theTokenMap;
std::once_flag tokenMapInitFlag;

Token getNextToken(std::istream& stream) {
    std::call_once(tokenMapInitFlag, [](){
		theTokenMap = {
                {"solid",    Token::SOLID},
                {"facet",    Token::FACET},
                {"normal",   Token::NORMAL},
                {"outer",    Token::OUTER},
                {"loop",     Token::LOOP},
                {"vertex",   Token::VERTEX},
                {"endloop",  Token::ENDLOOP},
                {"endfacet", Token::ENDFACET},
                {"endsolid", Token::ENDSOLID}
        };
	});

	std::string tokenStr;
	stream >> tokenStr;

	const auto t = theTokenMap.find(tokenStr);
	return (t == theTokenMap.end()) ? Token::UNKNOWN : t->second;
}

inline void skipToNextLine(std::istream& s) {
	s.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

} // namespace


void STLDecoder::decode(
		prtx::ContentPtrVector& results,
		std::istream&           stream,
		prt::Cache*             /*cache*/,
		const std::wstring&     /*key*/,
		prtx::ResolveMap const* /*resolveMap*/,
		std::wstring&           warnings
) {
	prtx::GeometryBuilder gb;
	prtx::MeshBuilder mb;

	uint32_t currentFace = 0;
	uint32_t currentFaceNormalIndex = 0;

	while (stream.good()) {
		Token t = getNextToken(stream);
		switch (t) {
			case Token::SOLID:
				skipToNextLine(stream); // ignore solid name for now
				break; // nop
			case Token::FACET:
				break; // nop, see LOOP
			case Token::NORMAL: {
				double n[3];
				stream >> n[0] >> n[1] >> n[2];
				currentFaceNormalIndex = mb.addNormalCoords(n);
				break;
			}
			case Token::OUTER:
				break; // ignored for now
			case Token::LOOP:
				currentFace = mb.addFace();
				break;
			case Token::VERTEX: {
				double v[3];
				stream >> v[0] >> v[1] >> v[2];
				uint32_t vi = mb.addVertexCoords(v);
				mb.addFaceVertexIndex(currentFace, vi);
				mb.addFaceNormalIndex(currentFace, currentFaceNormalIndex);
				break;
			}
			case Token::ENDLOOP:
				break; // nop
			case Token::ENDFACET:
				break; // nop
			case Token::ENDSOLID:
				gb.addMesh(mb.createSharedAndReset(&warnings));
				break;
			case Token::UNKNOWN:
				break;
		}
	}

	results.emplace_back(std::static_pointer_cast<prtx::Content>(gb.createSharedAndReset(&warnings)));
}

STLDecoderFactory* STLDecoderFactory::createInstance() { return new STLDecoderFactory(); }

STLDecoderFactory::STLDecoderFactory()
		: prtx::DecoderFactory(getContentType(), getID(), getName(), getDescription(), prtx::FileExtensions(EXT)) { }

STLDecoder* STLDecoderFactory::create() const {
	return new STLDecoder();
}

const std::wstring& STLDecoderFactory::getID() const {
	return ID;
}

const std::wstring& STLDecoderFactory::getName() const {
	return NAME;
}

const std::wstring& STLDecoderFactory::getDescription() const {
	return DESCRIPTION;
}

prt::ContentType STLDecoderFactory::getContentType() const {
	return prt::CT_GEOMETRY;
}
