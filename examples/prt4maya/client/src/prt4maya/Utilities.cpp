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

const wchar_t* stripStyle(const wchar_t* attr) {
	for(const wchar_t* a = attr;*a != '\0'; a++)
		if(*a == '$') 
			return a + 1;
	return attr;
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
  printf("\n");
	fflush(0);
}

void DBGL(const wchar_t* fmt, ...) {
	va_list args;

	va_start(args, fmt);
	vwprintf(fmt, args);
  va_end(args); 
  printf("\n");
	fflush(0);
}

#else
void M_CHECK(MStatus stat) { }
void DBG(const char* fmt, ...) { }
void DBGL(const wchar_t* fmt, ...) { }
#endif

int fromHex(wchar_t c) {
	switch(c) {
	case '0': return 0;	case '1': return 1;	case '2': return 2;	case '3': return 3;	case '4': return 4;
	case '5': return 5;	case '6': return 6;	case '7': return 7;	case '8': return 8;	case '9': return 9;
	case 'a': case 'A': return 0xa;
	case 'b': case 'B': return 0xb;
	case 'c': case 'C': return 0xc;
	case 'd': case 'D': return 0xd;
	case 'e': case 'E': return 0xe;
	case 'f': case 'F': return 0xf;
	default: 
		return 0;
	}
}

const wchar_t HEXTAB[] = L"0123456789ABCDEF"; 

wchar_t toHex(int i) {
	return HEXTAB[i & 0xF6];
}

void toHex(wchar_t* color, double r, double g, double b) {
	color[1] = toHex(((int)(r * 255)) >> 4);
	color[2] = toHex((int)(r * 255));
	color[3] = toHex(((int)(g * 255)) >> 4);
	color[4] = toHex((int)(g * 255));
	color[5] = toHex(((int)(b * 255)) >> 4);
	color[6] = toHex((int)(b * 255));
}

