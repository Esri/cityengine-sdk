#ifndef PRT_EXAMPLE_UBIDECODER_H_
#define PRT_EXAMPLE_UBIDECODER_H_

#include "prtx/Singleton.h"
#include "prtx/DecoderFactory.h"
#include "prtx/Decoder.h"

#include "prt/AttributeMap.h"
#include "prt/Callbacks.h"

#include <string>


class UBIDecoder : public prtx::GeometryDecoder {
public:
	UBIDecoder();
	virtual ~UBIDecoder();

public:
	virtual void decode(
			prtx::ContentPtrVector& results,
			std::istream& stream,
			prt::Cache* cache,
			const std::wstring& key,
			prtx::ResolveMap const* resolveMap,
			std::wstring& warnings
	);

public:
	static const std::wstring ID;
	static const std::wstring NAME;
	static const std::wstring DESCRIPTION;
};


class UBIDecoderFactory : public prtx::DecoderFactory, public prtx::Singleton<UBIDecoderFactory> {
protected:
	UBIDecoderFactory() : prtx::DecoderFactory(getContentType(), getID(), getName(), getDescription(), prtx::FileExtensions(L".ubi")) { }
	virtual ~UBIDecoderFactory() { }

public:
	virtual UBIDecoder* create() const;

	virtual const std::wstring& getID() const { return UBIDecoder::ID; }
	virtual const std::wstring& getName() const { return UBIDecoder::NAME; }
	virtual const std::wstring& getDescription() const { return UBIDecoder::DESCRIPTION; }
	virtual float getMerit() const { return getDefaultMerit() + 1.0; }

	virtual prt::ContentType getContentType() const { return prt::CT_GEOMETRY; }

	static UBIDecoderFactory* createInstance() {
		return new UBIDecoderFactory();
	}
};


#endif /* PRT_EXAMPLE_UBIDECODER_H_ */
