/**
 * Esri CityEngine SDK Unity Plugin Example
 *
 * This example demonstrates the main functionality of the Procedural Runtime API.
 * Esri R&D Center Zurich, Switzerland
 *
 * See http://github.com/ArcGIS/esri-cityengine-sdk for instructions.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Context.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
	switch(ul_reason_for_call) {
		case DLL_PROCESS_ATTACH:
			Context::initialize();
			return TRUE;
		case DLL_PROCESS_DETACH:
			// Note: do not call Context::deinitialize() as the dependent DLLs have already been unloaded
			return TRUE;
		default:
			return TRUE;
	}
}
