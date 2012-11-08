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
#include "prt/APIInitialShape.h"

#include "prtx/IStream.h"
#include "prtx/ExtensionFactory.h"
#include "prtx/IEncoder.h"

#include "encoder/IMayaOutputHandler.h"


class MayaEncoder : public prtx::IEncoder {
public:
	MayaEncoder();
	virtual ~MayaEncoder();

public:
	virtual void encode(const prt::InitialShape** initialShapes, size_t initialShapeCount, prtx::AbstractResolveMapPtr am,
			const prt::AttributeMap* options, prt::OutputHandler* const outputHandler);

	virtual void encodeContent(prtx::IOutputStream* stream, const prtx::ContentPtrVector& content) {
		throw std::runtime_error("not implemented");
	}

public:
	virtual const wchar_t* getID() const { return L"com.esri.prt.codecs.maya.MayaEncoder"; }
	virtual prt::ContentType getContentType() const { return prt::CT_GEOMETRY; }
	virtual ExtensionType getExtensionType() const { return ET_ENCODER; }

	static void destroyMayaData(struct MayaOutputHandler* mayaData);

private:
	void convertGeometry(prtx::AbstractResolveMapPtr am, prtx::ContentPtrVector& geometries, IMayaOutputHandler* mayaOutput);
	void unpackRPK(std::wstring rpkPath);
};


class MayaEncoderFactory : public prtx::ExtensionFactory {
public:
	MayaEncoder* create(const wchar_t**, const size_t&, const wchar_t**, const size_t&) { return new MayaEncoder(); }
};

#endif /* MAYAENCODER_H_ */
