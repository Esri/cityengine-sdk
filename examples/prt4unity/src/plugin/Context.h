/**
 * Esri CityEngine SDK Unity Plugin Example
 *
 * This example demonstrates the main functionality of the Procedural Runtime API.
 * Esri R&D Center Zurich, Switzerland
 *
 * See http://github.com/ArcGIS/esri-cityengine-sdk for instructions.
 */

#pragma once

#include "LogHandler.h"

#include "prt/API.h"
#include "prt/AttributeMap.h"
#include <vector>
#include <string>

/*
 *	Main class for using PRT, encapsulating all necessary state
 */
class Context {
private:
	const prt::AttributeMap* mUnityEncOpts;
	const prt::AttributeMap* mAttrEncOpts;

	const prt::ResolveMap* mResolveMap;								// resolve map for current rule package
	std::vector<std::wstring> mRuleFiles;							// rule files in current rule package
	size_t mRuleFileIndex;											// current rule file
	const prt::RuleFileInfo* mRuleFileInfo;							// info for current rule file
	std::vector<const prt::RuleFileInfo::Entry*> mStartRules;		// start rules in current rule file
	size_t mStartRuleIndex;											// current start rule

private:	// attributes
	struct Attribute {
		enum Type {
			ATTR_INVALID		= 0,
			ATTR_BOOL			= 1 << 0,
			ATTR_FLOAT			= 1 << 1,
			ATTR_STRING			= 1 << 2,
			ATTR_ENUM			= 1 << 3,
			ATTR_ENUM_BOOL		= ATTR_ENUM | ATTR_BOOL,
			ATTR_ENUM_FLOAT		= ATTR_ENUM | ATTR_FLOAT,
			ATTR_ENUM_STRING	= ATTR_ENUM | ATTR_STRING,
			ATTR_DIRECTORY		= 1 << 4 | ATTR_STRING,
			ATTR_FILE			= 1 << 5 | ATTR_STRING,
			ATTR_COLOR			= 1 << 6 | ATTR_STRING
		};

		Type mType;
		std::wstring mName;
	};

	struct BoolAttribute : public Attribute {
		bool mValue;
	};

	struct FloatAttribute : public Attribute {
		double mValue;
		double mMin;
		double mMax;
	};

	struct StringAttribute : public Attribute {
		std::wstring mValue;
	};

	struct FileAttribute : public StringAttribute {
		std::wstring mExt;
	};

	struct EnumAttribute : public Attribute {
		const prt::Annotation* mAnnotation;
		uint32_t mIndex;
	};

	std::vector<Attribute*> mAttributes;							// attributes used for generate

private:	// meshes and materials
	struct Mesh {
		struct SubMesh {
			uint32_t mFirstIndex;
			uint32_t mNumIndices;
			uint32_t mMaterial;
		};

		std::wstring mName;
		std::vector<float> mVertices;
		std::vector<float> mNormals;
		std::vector<float> mTexcoords;
		std::vector<uint32_t> mIndices;
		std::vector<SubMesh> mSubMeshes;
	};

	struct Material {
		std::wstring mName;
		float mColor[3];
		std::wstring mDiffuseTexture;
	};

	std::vector<Mesh*> mMeshes;										// meshes output by last generate
	std::vector<Material*> mMaterials;								// materials output by last generate

private:
	class OutputHandler;
	friend class OutputHandler;

private:
	static UnityLogHandler*     theLogHandler;
	static prt::FileLogHandler* theFileLogHandler;
	static const prt::Object*   theLicHandle;
	static prt::CacheObject*    theCache;

public:
	Context();
	~Context();

	bool setRulePackage(const wchar_t* filename, const wchar_t* unpackPath);
	const std::vector<std::wstring>& getRuleFiles() const;

	bool setRuleFile(size_t index);
	const std::vector<const prt::RuleFileInfo::Entry*>& getStartRules() const;

	bool setStartRule(size_t index);

	size_t getAttributeCount() const;
	Attribute::Type getAttributeType(size_t attrIndex) const;
	const wchar_t* getAttributeName(size_t attrIndex) const;

	bool getBoolAttributeValue(size_t attrIndex, bool& value) const;
	bool getFloatAttributeValue(size_t attrIndex, double& value, double& minValue, double& maxValue) const;
	bool getStringAttributeValue(size_t attrIndex, const wchar_t*& value) const;
	bool getDirectoryAttributeValue(size_t attrIndex, const wchar_t*& value) const;
	bool getFileAttributeValue(size_t attrIndex, const wchar_t*& value, const wchar_t*& ext) const;
	bool getColorAttributeValue(size_t attrIndex, const wchar_t*& value) const;
	bool getEnumAttributeValue(size_t attrIndex, uint32_t& value) const;
	size_t getEnumAttributeFieldCount(size_t attrIndex) const;
	bool getEnumAttributeField(size_t attrIndex, size_t fieldIndex, const wchar_t*& key) const;

	bool setBoolAttributeValue(size_t attrIndex, bool value);
	bool setFloatAttributeValue(size_t attrIndex, double value);
	bool setStringAttributeValue(size_t attrIndex, const wchar_t* value);
	bool setDirectoryAttributeValue(size_t attrIndex, const wchar_t* value);
	bool setFileAttributeValue(size_t attrIndex, const wchar_t* value);
	bool setColorAttributeValue(size_t attrIndex, const wchar_t* value);
	bool setEnumAttributeValue(size_t attrIndex, uint32_t value);

	bool generate(const float* vertices, uint32_t numVertices, const uint32_t* indices, uint32_t numIndices, const wchar_t* specialMaterial);

	size_t getMaterialCount() const;
	bool getMaterial(size_t materialIndex, const wchar_t*& name, const float*& color, const wchar_t*& diffuseTexture) const;

	size_t getMeshCount() const;
	bool getMesh(size_t meshIndex, const wchar_t*& name, uint32_t& numVertices, const float*& vertices, const float*& normals, const float*& texcoords) const;
	size_t getSubMeshCount(size_t meshIndex) const;
	bool getSubMesh(size_t meshIndex, size_t subMeshIndex, const uint32_t*& indices, uint32_t& numIndices, uint32_t& materialIndex) const;

	void releaseMeshesAndMaterials();

public:
	static std::wstring getPrtLibRoot();
	static std::wstring getPluginRoot();
	static bool initialize();
	static void uninitialize();
	static void setLogMessageCallback(UnityLogHandler::LogMessageCallback callback);
	static void postUnityLogMessage(const wchar_t* message, uint32_t level);

private:
	void clearAttributes();
	void clearMeshes();
	void clearMaterials();

	void addBoolAttribute(const std::wstring& name, bool value);
	void addFloatAttribute(const std::wstring& name, double value, double minValue, double maxValue);
	void addStringAttribute(const std::wstring& name, const std::wstring& value);
	void addEnumAttribute(const std::wstring& name, const prt::Annotation* annotation, bool value);
	void addEnumAttribute(const std::wstring& name, const prt::Annotation* annotation, double value);
	void addEnumAttribute(const std::wstring& name, const prt::Annotation* annotation, const std::wstring& value);
	void addEnumAttribute(Attribute::Type type, const std::wstring& name, const prt::Annotation* annotation, uint32_t index);
	void addDirectoryAttribute(const std::wstring& name, const std::wstring& value);
	void addFileAttribute(const std::wstring& name, const std::wstring& value, const std::wstring& ext);
	void addColorAttribute(const std::wstring& name, const std::wstring& value);
};
