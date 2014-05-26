#include "UBIDecoder.h"
#include "UBIAdaptor.h"

#include "prtx/ExtensionManager.h"
#include <iostream>


#ifdef _WIN32
#	define STLENC_EXPORTS_API __declspec(dllexport)
#else
#	define STLENC_EXPORTS_API __attribute__ ((visibility ("default")))
#endif


static const int MINIMAL_VERSION_MAJOR = 1;
static const int MINIMAL_VERSION_MINOR = 1;


extern "C" {


STLENC_EXPORTS_API void registerExtensionFactories(prtx::ExtensionManager* manager) {
	try {
		manager->addFactory(UBIDecoderFactory::instance());
		manager->addFactory(UBIAdaptorFactory::instance());
	} catch (std::exception& e) {
		std::cerr << __FUNCTION__ << " caught exception: " <<  e.what() << std::endl;
	} catch (...) {
		std::cerr << __FUNCTION__ << " caught unknown exception: " << std::endl;
	}
}


STLENC_EXPORTS_API void unregisterExtensionFactories(prtx::ExtensionManager* /*manager*/) {
}


STLENC_EXPORTS_API int getMinimalVersionMajor() {
	return MINIMAL_VERSION_MAJOR;
}


STLENC_EXPORTS_API int getMinimalVersionMinor() {
	return MINIMAL_VERSION_MINOR;
}


} // extern "C"
