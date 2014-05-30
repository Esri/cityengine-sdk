/**
 * Esri CityEngine SDK Maya Plugin Example
 *
 * This example demonstrates the main functionality of the Procedural Runtime API.
 * Esri R&D Center Zurich, Switzerland
 *
 * See http://github.com/ArcGIS/esri-cityengine-sdk for instructions.
 */

#include "codecs_maya.h"

#include "prtx/prtx.h"
#include "prtx/ExtensionManager.h"

#include "encoder/MayaEncoder.h"


static const int MINIMAL_VERSION_MAJOR = 1;
static const int MINIMAL_VERSION_MINOR = 1;


extern "C" {


PRT4MAYA_CODEC_EXPORTS_API void registerExtensionFactories(prtx::ExtensionManager* manager) {
	manager->addFactory(MayaEncoderFactory::createInstance());
}


PRT4MAYA_CODEC_EXPORTS_API void unregisterExtensionFactories(prtx::ExtensionManager* /*manager*/) {
}


PRT4MAYA_CODEC_EXPORTS_API int getMinimalVersionMajor() {
	return MINIMAL_VERSION_MAJOR;
}


PRT4MAYA_CODEC_EXPORTS_API int getMinimalVersionMinor() {
	return MINIMAL_VERSION_MINOR;
}


}
