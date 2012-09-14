#include <iostream>

#include "spi/prtspi.h"
#include "encoder/MayaEncoder.h"

#include "codecs_maya.h"


static const int MINIMAL_VERSION_MAJOR = 2012;
static const int MINIMAL_VERSION_MINOR = 1;
static const int MINIMAL_VERSION_BUILD = 0;


extern "C" {


CODECS_EXPORTS_API void registerExtensionFactories(prtspi::IExtensionManager* manager) {
	prtspi::Log::trace("codecs maya library: registerExtensionFactories begin");
	manager->addFactory(new MayaEncoderFactory());
	prtspi::Log::trace("codecs maya library: registerExtensionFactories done");
}


CODECS_EXPORTS_API int getMinimalVersionMajor()
{
	return MINIMAL_VERSION_MAJOR;
}


CODECS_EXPORTS_API int getMinimalVersionMinor()
{
	return MINIMAL_VERSION_MINOR;
}


CODECS_EXPORTS_API int getMinimalVersionBuild()
{
	return MINIMAL_VERSION_BUILD;
}


}
