/**
 * Esri CityEngine SDK Maya Plugin Example
 *
 * This example demonstrates the main functionality of the Procedural Runtime API.
 * Esri R&D Center Zurich, Switzerland
 *
 * See http://github.com/ArcGIS/esri-cityengine-sdk for instructions.
 */

#include "Utilities.h"
#include <maya/MString.h>

const char* filename(const char* path) {
	while(*(--path) != '\\');
	return path + 1;
}

std::wstring getSharedLibraryPrefix() {
#if defined(_WIN32)
	return L"";
#elif defined(__APPLE__)
	return L"lib";
#elif defined(linux)
	return L"lib";
#else
#	error unsupported build platform
#endif
}


std::wstring getSharedLibrarySuffix() {
#if defined(_WIN32)
	return L".dll";
#elif defined(__APPLE__)
	return L".dylib";
#elif defined(linux)
	return L".so";
#else
#	error unsupported build platform
#endif
}

#if DO_DBG == 1

void M_CHECK(MStatus stat) {
	if(MS::kSuccess != stat) {
		std::wcerr << L"err: " << stat.errorString().asWChar() << L" (code: " << stat.statusCode() << L")" << std::endl;
		throw stat;
	}
}

void DBG(const char* fmt, ...) {
	va_list args;

	va_start(args, fmt);
	vprintf(fmt, args);
  va_end(args); 
 
	fflush(0);
}

void DBGL(const wchar_t* fmt, ...) {
	va_list args;

	va_start(args, fmt);
	vwprintf(fmt, args);
  va_end(args); 
	fflush(0);
}

#else
void M_CHECK(MStatus stat) { }
void DBG(const char* fmt, ...) { }
void DBGL(const wchar_t* fmt, ...) { }
#endif
