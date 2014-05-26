#include "VoxelEncoder.h"

#include "prtx/ExtensionManager.h"

#include <iostream>
#include <stdexcept>

#ifdef _WIN32
#	define VOXENC_EXPORTS_API __declspec(dllexport)
#else
#	define VOXENC_EXPORTS_API __attribute__ ((visibility ("default")))
#endif


namespace {
const int MINIMAL_VERSION_MAJOR = 1;
const int MINIMAL_VERSION_MINOR = 1;
}


extern "C" {

VOXENC_EXPORTS_API void registerExtensionFactories(prtx::ExtensionManager* manager) {
	try {
		manager->addFactory(VoxelEncoderFactory::instance());
	} catch (std::exception& e) {
		std::cerr << __FUNCTION__ << " caught exception: " <<  e.what() << std::endl;
	} catch (...) {
		std::cerr << __FUNCTION__ << " caught unknown exception: " << std::endl;
	}
}

VOXENC_EXPORTS_API void unregisterExtensionFactories(prtx::ExtensionManager*) {
}

VOXENC_EXPORTS_API int getMinimalVersionMajor() {
	return MINIMAL_VERSION_MAJOR;
}

VOXENC_EXPORTS_API int getMinimalVersionMinor() {
	return MINIMAL_VERSION_MINOR;
}

} // extern "C"
