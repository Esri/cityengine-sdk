/*
 * MayaEncoder.h
 *
 *  Created on: Sep 11, 2012
 *      Author: shaegler
 */

#ifndef MAYAENCODER_H_
#define MAYAENCODER_H_

#include <string>
#include <iostream>
#include <stdexcept>

#include "prt/ContentType.h"
#include "prt/InitialShape.h"

#include "prtx/ResolveMap.h"
#include "prtx/Encoder.h"
#include "prtx/EncoderFactory.h"
#include "prtx/EncoderInfoBuilder.h"


class IMayaOutputHandler;

class MayaEncoder : public prtx::GeometryEncoder {
public:
	static const std::wstring ID;
	static const std::wstring NAME;
	static const std::wstring DESCRIPTION;

public:
	MayaEncoder();
	virtual ~MayaEncoder();

public:
	virtual void init(prtx::IGenerateContext& /*context*/);
	virtual void encode(prtx::IGenerateContext& context, size_t initialShapeIndex);
	virtual void finish(prtx::IGenerateContext& /*context*/);

//public:
//	static void destroyMayaData(struct MayaOutputHandler* mayaData);

private:
	void convertGeometry(
			const std::wstring& cgbName,
			const prtx::GeometryPtrVector& geometries,
			const std::vector<prtx::MaterialPtrVector>& mat,
			IMayaOutputHandler* mayaOutput
	);
};


class MayaEncoderFactory : public prtx::EncoderFactory, public prtx::Singleton<MayaEncoderFactory> {
public:
	static MayaEncoderFactory* createSingleton() {
		prtx::EncoderInfoBuilder encoderInfoBuilder;

		encoderInfoBuilder.setID(MayaEncoder::ID);
		encoderInfoBuilder.setName(MayaEncoder::NAME);
		encoderInfoBuilder.setDescription(MayaEncoder::DESCRIPTION);
		encoderInfoBuilder.setType(prt::CT_GEOMETRY);

		return new MayaEncoderFactory(encoderInfoBuilder.createInfo());
	}

	MayaEncoderFactory(const prt::EncoderInfo* info) : prtx::EncoderFactory(info) { }
	virtual ~MayaEncoderFactory() { }
	virtual MayaEncoder* create() { return new MayaEncoder(); }
};


#endif /* MAYAENCODER_H_ */
