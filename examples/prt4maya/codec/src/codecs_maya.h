/**
 * Esri CityEngine SDK Maya Plugin Example
 *
 * This example demonstrates the main functionality of the Procedural Runtime API.
 * Esri R&D Center Zurich, Switzerland
 *
 * See http://github.com/ArcGIS/esri-cityengine-sdk for instructions.
 */

#ifndef PRT4MAYA_CODEC_H_
#define PRT4MAYA_CODEC_H_

#ifdef _WIN32
#	define PRT4MAYA_CODEC_EXPORTS_API __declspec(dllexport)
#else
#	define PRT4MAYA_CODEC_EXPORTS_API __attribute__ ((visibility ("default")))
#endif

#endif /* PRT4MAYA_CODEC_H_ */
