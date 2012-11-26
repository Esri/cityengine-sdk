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

#include "prt/ContentType.h"
#include "prt/InitialShape.h"

#include "prtx/IStream.h"
#include "prtx/EncoderFactory.h"
#include "prtx/IEncoder.h"

#include "encoder/IMayaOutputHandler.h"


class MayaEncoder : public prtx::IEncoder {
public:
	MayaEncoder(const prt::AttributeMap* options);
	virtual ~MayaEncoder();

public:
	virtual void init(prtx::IGenerateContext& context) { }
	virtual void encode(prtx::IGenerateContext& context, size_t initialShapeIndex);
	virtual void encodeContent(prtx::IGenerateContext& context, size_t initialShapeIndex, const prtx::ContentPtrVector& content) {
		throw std::runtime_error("not implemented");
	}
	virtual void finish(prtx::IGenerateContext& context) { }

public:
	virtual const wchar_t* getID() const { return L"com.esri.prt.codecs.maya.MayaEncoder"; }
	virtual prt::ContentType getContentType() const { return prt::CT_GEOMETRY; }
	virtual ExtensionType getExtensionType() const { return ET_ENCODER; }

	static void destroyMayaData(struct MayaOutputHandler* mayaData);

private:
	void convertGeometry(prtx::AbstractResolveMapPtr am, prtx::GeometryPtrVector& geometries, IMayaOutputHandler* mayaOutput);
	void unpackRPK(std::wstring rpkPath);
};


class MayaEncoderFactory : public prtx::EncoderFactory {
public:
	MayaEncoder* create(const wchar_t**, const size_t&, const wchar_t**, const size_t&, const prt::AttributeMap* attributes) {
		return new MayaEncoder(attributes);
	}
};

#endif /* MAYAENCODER_H_ */
