/**
 * Esri CityEngine SDK Maya Plugin Example
 *
 * This example demonstrates the main functionality of the Procedural Runtime API.
 * Esri R&D Center Zurich, Switzerland
 *
 * See http://github.com/ArcGIS/esri-cityengine-sdk for instructions.
 */

#ifndef UTILITIES_H_
#define UTILITIES_H_

#include <maya/MStatus.h>

#define DO_DBG 1
#define MCHECK(_stat_) {if(MS::kSuccess != _stat_) {DBG("maya err at %s:%d: %s %d\n", __FILE__, __LINE__, _stat_.errorString().asChar(), _stat_.statusCode());}}

const char* filename(const char* path);

void DBG(const char* fmt, ...);
void DBGL(const wchar_t* fmt, ...);

std::wstring getSharedLibraryPrefix();
std::wstring getSharedLibrarySuffix();
int fromHex(wchar_t c);
wchar_t toHex(int i);
void toHex(wchar_t* color, double r, double g, double b);

#endif /* UTILITIES_H_ */
