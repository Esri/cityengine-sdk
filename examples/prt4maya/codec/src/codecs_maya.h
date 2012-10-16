/*
 * codecs_maya.h
 *
 *  Created on: 23 Feb 2012
 *      Author: dec
 */

#ifndef CODECS_MAYA_H_
#define CODECS_MAYA_H_

#ifdef _WIN32
#define CODECS_MAYA_EXPORTS_API __declspec(dllexport)
#else
#define CODECS_MAYA_EXPORTS_API __attribute__ ((visibility ("default")))
#endif

#endif /* CODECS_MAYA_H_ */
