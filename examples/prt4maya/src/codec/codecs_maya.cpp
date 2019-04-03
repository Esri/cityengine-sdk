/**
 * Esri CityEngine SDK Maya Plugin Example
 *
 * This example demonstrates the main functionality of the Procedural Runtime API.
 *
 * See README.md in https://github.com/Esri/esri-cityengine-sdk for build instructions.
 *
 * Copyright (c) 2012-2019 Esri R&D Center Zurich
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "codecs_maya.h"

#include "prtx/prtx.h"
#include "prtx/ExtensionManager.h"

#include "encoder/MayaEncoder.h"


extern "C" {


PRT4MAYA_CODEC_EXPORTS_API void registerExtensionFactories(prtx::ExtensionManager* manager) {
	manager->addFactory(MayaEncoderFactory::createInstance());
}


PRT4MAYA_CODEC_EXPORTS_API void unregisterExtensionFactories(prtx::ExtensionManager* /*manager*/) {
}


PRT4MAYA_CODEC_EXPORTS_API int getVersionMajor() {
	return PRT_VERSION_MAJOR;
}


PRT4MAYA_CODEC_EXPORTS_API int getVersionMinor() {
	return PRT_VERSION_MINOR;
}


}
