/**
 * Esri CityEngine SDK Maya Plugin Example
 *
 * This example demonstrates the main functionality of the Procedural Runtime API.
 * Esri R&D Center Zurich, Switzerland
 *
 * See http://github.com/ArcGIS/esri-cityengine-sdk for instructions.
 */

#ifndef CODECS_MAYA_H_
#define CODECS_MAYA_H_

#ifdef _WIN32
#	define CODECS_MAYA_EXPORTS_API __declspec(dllexport)
#else
#	define CODECS_MAYA_EXPORTS_API __attribute__ ((visibility ("default")))
#endif

#endif /* CODECS_MAYA_H_ */
