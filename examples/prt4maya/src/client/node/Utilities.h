/**
 * Esri CityEngine SDK Maya Plugin Example
 *
 * This example demonstrates the main functionality of the Procedural Runtime API.
 * Esri R&D Center Zurich, Switzerland
 *
 * See http://github.com/ArcGIS/esri-cityengine-sdk for instructions.
 */

#pragma once

#include "maya/MStatus.h"
#include "maya/MFloatPointArray.h"



#define DO_DBG 1
#define MCHECK(_stat_) {if(MS::kSuccess != _stat_) { prtu::dbg("maya err at %s:%d: %s %d\n", __FILE__, __LINE__, _stat_.errorString().asChar(), _stat_.statusCode());}}


namespace prtu {


const char* filename(const char* path);

void dbg(const char* fmt, ...);
void wdbg(const wchar_t* fmt, ...);

template<typename C> C getDirSeparator();
template<> char getDirSeparator();
template<> wchar_t getDirSeparator();

template<typename C> const std::basic_string<C>& getSharedLibraryPrefix();
template<> const std::wstring& getSharedLibraryPrefix();
template<> const std::string& getSharedLibraryPrefix();

template<typename C> const std::basic_string<C>& getSharedLibrarySuffix();
template<> const std::wstring& getSharedLibrarySuffix();
template<> const std::string& getSharedLibrarySuffix();

int fromHex(wchar_t c);
wchar_t toHex(int i);
void toHex(wchar_t* color, double r, double g, double b);
MString toCleanId(const MString& name);

int32_t computeSeed(MFloatPointArray& vertices);
int32_t computeSeed(const double* vertices, size_t count);

} // namespace prtu
