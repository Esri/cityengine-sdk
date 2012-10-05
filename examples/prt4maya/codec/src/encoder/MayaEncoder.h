/*
 * MayaEncoder.h
 *
 *  Created on: Sep 11, 2012
 *      Author: shaegler
 */

#ifndef MAYAENCODER_H_
#define MAYAENCODER_H_

#include <iostream>
#include <stdexcept>

#include "spi/base/IStream.h"
#include "spi/extension/ExtensionFactory.h"
#include "spi/codec/IEncoder.h"

#include "util.h"
#include "maya_codec.h"


class MayaEncoder : public prtspi::IEncoder {
public:
	MayaEncoder();
	virtual ~MayaEncoder();

public:
	virtual void encode(prtspi::IOutputStream* stream, const prtspi::InitialShape** initialShapes, size_t initialShapeCount,
			prtspi::AbstractResolveMapPtr am, const prt::Attributable* options);
	virtual void encodeContent(prtspi::IOutputStream* stream, const prtspi::IContentArray* content) {
		UNUSED(stream); UNUSED(content); throw std::runtime_error("not implemented");
	}

public:
	virtual const wchar_t* getID() const { return L"com.esri.prt.codecs.maya.MayaEncoder"; }
	virtual ContentType getContentType() const { return CT_GEOMETRY; }
	virtual ExtensionType getExtensionType() const { return ET_ENCODER; }

	static void destroyMayaData(struct MayaData* mayaData);
private:
	void convertGeometry(prtspi::IOutputStream* stream, prtspi::IContentArray* geometries);

};


class MayaEncoderFactory : public prtspi::ExtensionFactory {
public:
	MayaEncoder* create(const wchar_t**, const size_t&, const wchar_t**, const size_t&) { return new MayaEncoder(); }
};

#endif /* MAYAENCODER_H_ */
