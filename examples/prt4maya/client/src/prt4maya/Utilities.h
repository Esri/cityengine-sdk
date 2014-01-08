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

const char* filename(const char* path);

void M_CHECK(MStatus stat);
void DBG(const char* fmt, ...);
void DBGL(const wchar_t* fmt, ...);

std::wstring getSharedLibraryPrefix();
std::wstring getSharedLibrarySuffix();

#endif /* UTILITIES_H_ */