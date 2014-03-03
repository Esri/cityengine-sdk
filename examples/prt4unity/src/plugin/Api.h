/**
 * Esri CityEngine SDK Unity Plugin Example
 *
 * This example demonstrates the main functionality of the Procedural Runtime API.
 * Esri R&D Center Zurich, Switzerland
 *
 * See http://github.com/ArcGIS/esri-cityengine-sdk for instructions.
 */

#pragma once

#ifndef _WINDOWS_
#	error Windows.h required but not included
#endif
#include <OleAuto.h>
#include <cstdint>

#ifdef PRT4UNITY_EXPORTS
#define PRT4UNITY_API __declspec(dllexport)
#else
#define PRT4UNITY_API __declspec(dllimport)
#endif

class Context;
typedef void (*LogMessageCallback)(const wchar_t* message, uint32_t level);

// public, exported C interface to the Context class, to be used by Unity scripts
extern "C" {
	PRT4UNITY_API bool Initialize();
	PRT4UNITY_API void Uninitialize();

	PRT4UNITY_API void SetLogMessageCallback(LogMessageCallback callback);

	PRT4UNITY_API Context* CreateContext();
	PRT4UNITY_API void DeleteContext(Context* ctx);

	PRT4UNITY_API bool SetRulePackage(Context* ctx, const wchar_t* filename, const wchar_t* unpackPath);
	PRT4UNITY_API uint32_t GetRuleFileCount(Context* ctx);
	PRT4UNITY_API BSTR GetRuleFile(Context* ctx, uint32_t index);

	PRT4UNITY_API bool SetRuleFile(Context* ctx, uint32_t index);
	PRT4UNITY_API uint32_t GetStartRuleCount(Context* ctx);
	PRT4UNITY_API const BSTR GetStartRuleName(Context* ctx, uint32_t index);

	PRT4UNITY_API bool SetStartRule(Context* ctx, uint32_t index);
	PRT4UNITY_API uint32_t GetAttributeCount(Context* ctx);
	PRT4UNITY_API uint32_t GetAttributeType(Context* ctx, uint32_t attrIndex);
	PRT4UNITY_API BSTR GetAttributeName(Context* ctx, uint32_t attrIndex);
	PRT4UNITY_API bool GetBoolAttributeValue(Context* ctx, uint32_t attrIndex, bool& value);
	PRT4UNITY_API bool GetFloatAttributeValue(Context* ctx, uint32_t attrIndex, double& value, double& minValue, double& maxValue);
	PRT4UNITY_API bool GetStringAttributeValue(Context* ctx, uint32_t attrIndex, const wchar_t*& value);
	PRT4UNITY_API bool GetDirectoryAttributeValue(Context* ctx, uint32_t attrIndex, const wchar_t*& value);
	PRT4UNITY_API bool GetFileAttributeValue(Context* ctx, uint32_t attrIndex, const wchar_t*& value, const wchar_t*& ext);
	PRT4UNITY_API bool GetColorAttributeValue(Context* ctx, uint32_t attrIndex, const wchar_t*& value);
	PRT4UNITY_API bool GetEnumAttributeValue(Context* ctx, uint32_t attrIndex, uint32_t& value);
	PRT4UNITY_API uint32_t GetEnumAttributeFieldCount(Context* ctx, uint32_t attrIndex);
	PRT4UNITY_API bool GetEnumAttributeField(Context* ctx, uint32_t attrIndex, uint32_t fieldIndex, const wchar_t*& value);

	PRT4UNITY_API bool SetBoolAttributeValue(Context* ctx, uint32_t attrIndex, bool value);
	PRT4UNITY_API bool SetFloatAttributeValue(Context* ctx, uint32_t attrIndex, double value);
	PRT4UNITY_API bool SetStringAttributeValue(Context* ctx, uint32_t attrIndex, const wchar_t* value);
	PRT4UNITY_API bool SetDirectoryAttributeValue(Context* ctx, uint32_t attrIndex, const wchar_t* value);
	PRT4UNITY_API bool SetFileAttributeValue(Context* ctx, uint32_t attrIndex, const wchar_t* value);
	PRT4UNITY_API bool SetColorAttributeValue(Context* ctx, uint32_t attrIndex, const wchar_t* value);
	PRT4UNITY_API bool SetEnumAttributeValue(Context* ctx, uint32_t attrIndex, uint32_t value);

	PRT4UNITY_API bool Generate(Context* ctx, const float* vertices, uint32_t numVertices, const uint32_t* indices, uint32_t numIndices, const wchar_t* specialMaterial);

	PRT4UNITY_API uint32_t GetMaterialCount(Context* ctx);
	PRT4UNITY_API bool GetMaterial(Context* ctx, uint32_t materialIndex, const wchar_t*& name, const float*& color, const wchar_t*& diffuseTexture);
	PRT4UNITY_API uint32_t GetMeshCount(Context* ctx);
	PRT4UNITY_API bool GetMesh(Context* ctx, uint32_t meshIndex, const wchar_t*& name, uint32_t& numVertices, const float*& vertices, const float*& normals, const float*& texcoords);
	PRT4UNITY_API uint32_t GetSubMeshCount(Context* ctx, uint32_t meshIndex);
	PRT4UNITY_API bool GetSubMesh(Context* ctx, uint32_t meshIndex, uint32_t subMeshIndex, const uint32_t*& indices, uint32_t& numIndices, uint32_t& materialIndex);

	PRT4UNITY_API void ReleaseMeshesAndMaterials(Context* ctx);
}
