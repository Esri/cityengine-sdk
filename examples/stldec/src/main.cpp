/**
 * CityEngine SDK Custom STL Decoder Example
 *
 * This example demonstrates the usage of the PRTX interface
 * to write custom decoders.
 *
 * See README.md in https://github.com/Esri/cityengine-sdk for build instructions.
 *
 * Written by Simon Haegler
 * Esri R&D Center Zurich, Switzerland
 *
 * Copyright 2012-2024 (c) Esri R&D Center Zurich
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

#include "STLDecoder.h"
#include "prtx/ExtensionManager.h"
#include <iostream>


#ifdef _WIN32
#	define STLDEC_EXPORTS_API __declspec(dllexport)
#else
#	define STLDEC_EXPORTS_API __attribute__ ((visibility ("default")))
#endif


extern "C" {


STLDEC_EXPORTS_API void registerExtensionFactories(prtx::ExtensionManager* manager) {
	try {
		manager->addFactory(STLDecoderFactory::instance());
	} catch (const std::exception& e) {
		std::cerr << __FUNCTION__ << " caught exception: " <<  e.what() << std::endl;
	} catch (...) {
		std::cerr << __FUNCTION__ << " caught unknown exception: " << std::endl;
	}
}


STLDEC_EXPORTS_API void unregisterExtensionFactories(prtx::ExtensionManager* /*manager*/) {
}


STLDEC_EXPORTS_API int getVersionMajor() {
	return PRT_VERSION_MAJOR;
}


STLDEC_EXPORTS_API int getVersionMinor() {
	return PRT_VERSION_MINOR;
}


} // extern "C"
