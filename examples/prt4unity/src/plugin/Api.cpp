/**
 * Esri CityEngine SDK Unity Plugin Example
 *
 * This example demonstrates the main functionality of the Procedural Runtime API.
 * Esri R&D Center Zurich, Switzerland
 *
 * See http://github.com/ArcGIS/esri-cityengine-sdk for instructions.
 */

#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include "Api.h"
#include "Context.h"

namespace {
	BSTR MakeBSTR(const std::wstring& str) {
		return SysAllocStringLen(str.c_str(), str.size());
	}
}

bool Initialize() {
	return Context::initialize();
}

void Uninitialize() {
	Context::uninitialize();
}

void SetLogMessageCallback(LogMessageCallback callback) {
	Context::setLogMessageCallback(callback);
}

Context* CreateContext() {
	return new Context();
}

void DeleteContext(Context* ctx) {
	delete ctx;
}

bool SetRulePackage(Context* ctx, const wchar_t* filename, const wchar_t* unpackPath) {
	return ctx->setRulePackage(filename, unpackPath);
}

uint32_t GetRuleFileCount(Context* ctx) {
	return static_cast<uint32_t>(ctx->getRuleFiles().size());
}

BSTR GetRuleFile(Context* ctx, uint32_t index) {
	return MakeBSTR(ctx->getRuleFiles()[index]);
}

bool SetRuleFile(Context* ctx, uint32_t index) {
	return ctx->setRuleFile(index);
}

uint32_t GetStartRuleCount(Context* ctx) {
	return static_cast<uint32_t>(ctx->getStartRules().size());
}

const BSTR GetStartRuleName(Context* ctx, uint32_t index) {
	return MakeBSTR(ctx->getStartRules()[index]->getName());
}

bool SetStartRule(Context* ctx, uint32_t index) {
	return ctx->setStartRule(index);
}

uint32_t GetAttributeCount(Context* ctx) {
	return static_cast<uint32_t>(ctx->getAttributeCount());
}

uint32_t GetAttributeType(Context* ctx, uint32_t attrIndex) {
	return static_cast<uint32_t>(ctx->getAttributeType(attrIndex));
}

BSTR GetAttributeName(Context* ctx, uint32_t attrIndex) {
	return MakeBSTR(ctx->getAttributeName(attrIndex));
}

bool GetBoolAttributeValue(Context* ctx, uint32_t attrIndex, bool& value) {
	return ctx->getBoolAttributeValue(attrIndex, value);
}

bool GetFloatAttributeValue(Context* ctx, uint32_t attrIndex, double& value, double& minValue, double& maxValue) {
	return ctx->getFloatAttributeValue(attrIndex, value, minValue, maxValue);
}

bool GetStringAttributeValue(Context* ctx, uint32_t attrIndex, const wchar_t*& value) {
	return ctx->getStringAttributeValue(attrIndex, value);
}

bool GetDirectoryAttributeValue(Context* ctx, uint32_t attrIndex, const wchar_t*& value) {
	return ctx->getDirectoryAttributeValue(attrIndex, value);
}

bool GetFileAttributeValue(Context* ctx, uint32_t attrIndex, const wchar_t*& value, const wchar_t*& ext) {
	return ctx->getFileAttributeValue(attrIndex, value, ext);
}

bool GetColorAttributeValue(Context* ctx, uint32_t attrIndex, const wchar_t*& value) {
	return ctx->getColorAttributeValue(attrIndex, value);
}

bool GetEnumAttributeValue(Context* ctx, uint32_t attrIndex, uint32_t& value) {
	return ctx->getEnumAttributeValue(attrIndex, value);
}

uint32_t GetEnumAttributeFieldCount(Context* ctx, uint32_t attrIndex) {
	return static_cast<uint32_t>(ctx->getEnumAttributeFieldCount(attrIndex));
}

bool GetEnumAttributeField(Context* ctx, uint32_t attrIndex, uint32_t fieldIndex, const wchar_t*& value) {
	return ctx->getEnumAttributeField(attrIndex, fieldIndex, value);
}

bool SetBoolAttributeValue(Context* ctx, uint32_t attrIndex, bool value) {
	return ctx->setBoolAttributeValue(attrIndex, value);
}

bool SetFloatAttributeValue(Context* ctx, uint32_t attrIndex, double value) {
	return ctx->setFloatAttributeValue(attrIndex, value);
}

bool SetStringAttributeValue(Context* ctx, uint32_t attrIndex, const wchar_t* value) {
	return ctx->setStringAttributeValue(attrIndex, value);
}

bool SetDirectoryAttributeValue(Context* ctx, uint32_t attrIndex, const wchar_t* value) {
	return ctx->setDirectoryAttributeValue(attrIndex, value);
}

bool SetFileAttributeValue(Context* ctx, uint32_t attrIndex, const wchar_t* value) {
	return ctx->setFileAttributeValue(attrIndex, value);
}

bool SetColorAttributeValue(Context* ctx, uint32_t attrIndex, const wchar_t* value) {
	return ctx->setColorAttributeValue(attrIndex, value);
}

bool SetEnumAttributeValue(Context* ctx, uint32_t attrIndex, uint32_t value) {
	return ctx->setEnumAttributeValue(attrIndex, value);
}

bool Generate(Context* ctx, const float* vertices, uint32_t numVertices, const uint32_t* indices, uint32_t numIndices, const wchar_t* specialMaterial) {
	return ctx->generate(vertices, numVertices, indices, numIndices, specialMaterial);
}

uint32_t GetMaterialCount(Context* ctx) {
	return static_cast<uint32_t>(ctx->getMaterialCount());
}

bool GetMaterial(Context* ctx, uint32_t materialIndex, const wchar_t*& name, const float*& color, const wchar_t*& diffuseTexture) {
	return ctx->getMaterial(materialIndex, name, color, diffuseTexture);
}

uint32_t GetMeshCount(Context* ctx) {
	return static_cast<uint32_t>(ctx->getMeshCount());
}

bool GetMesh(Context* ctx, uint32_t meshIndex, const wchar_t*& name, uint32_t& numVertices, const float*& vertices, const float*& normals, const float*& texcoords) {
	return ctx->getMesh(meshIndex, name, numVertices, vertices, normals, texcoords);
}

uint32_t GetSubMeshCount(Context* ctx, uint32_t meshIndex) {
	return static_cast<uint32_t>(ctx->getSubMeshCount(meshIndex));
}

bool GetSubMesh(Context* ctx, uint32_t meshIndex, uint32_t subMeshIndex, const uint32_t*& indices, uint32_t& numIndices, uint32_t& materialIndex) {
	return ctx->getSubMesh(meshIndex, subMeshIndex, indices, numIndices, materialIndex);
}

void ReleaseMeshesAndMaterials(Context* ctx) {
	return ctx->releaseMeshesAndMaterials();
}
