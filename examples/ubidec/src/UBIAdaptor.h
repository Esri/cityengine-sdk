#ifndef UBIADAPTOR_H
#define UBIADAPTOR_H

#include "prtx/URI.h"
#include "prtx/Adaptor.h"
#include "prtx/AdaptorFactory.h"
#include "prtx/PRTUtils.h"
#include "prtx/Singleton.h"

#include "prt/ContentType.h"

#include <string>
#include <istream>


class UBIAdaptor : public prtx::StreamAdaptor {
public:
	UBIAdaptor();
	virtual ~UBIAdaptor();

public:
	virtual prt::ResolveMap const* createResolveMap(prtx::URIPtr uri) const;

protected:
	virtual std::istream* createStream(prtx::URIPtr uri) const;
	virtual void destroyStream(std::istream* stream) const;

public:
	static const std::wstring ID;
	static const std::wstring NAME;
	static const std::wstring DESCRIPTION;
};


class UBIAdaptorFactory : public prtx::AdaptorFactory, public prtx::Singleton<UBIAdaptorFactory> {
public:
	UBIAdaptorFactory();
	virtual ~UBIAdaptorFactory();

	virtual UBIAdaptor* create() const { return new UBIAdaptor(); }

	virtual const std::wstring& getID() const { return UBIAdaptor::ID; };
	virtual const std::wstring& getName() const { return UBIAdaptor::NAME; }
	virtual const std::wstring& getDescription() const { return UBIAdaptor::DESCRIPTION; }
	virtual float getMerit() const { return getDefaultMerit() + 1.0f; }
	virtual bool canHandleURI(prtx::URIPtr uri) const;

	static UBIAdaptorFactory* createInstance() { return new UBIAdaptorFactory(); }
};


#endif /* UBIADAPTOR_H */
