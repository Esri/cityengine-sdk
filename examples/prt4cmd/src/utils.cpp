/**
 * CityEngine SDK CLI Example
 *
 * This example demonstrates the main functionality of the Procedural Runtime API.
 *
 * See README.md in https://github.com/Esri/cityengine-sdk for build instructions.
 *
 * Copyright (c) 2012-2025 Esri R&D Center Zurich
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "utils.h"
#include "logging.h"

#include "prt/StringUtils.h"

#include "CLI11.hpp"

#include <algorithm>
#include <iostream>

namespace {

#ifdef _WIN32
const std::string FILE_SCHEMA = "file:/";
#else
const std::string FILE_SCHEMA = "file:";
#endif

const char* ENCODER_ID_OBJ = "com.esri.prt.codecs.OBJEncoder";

template <typename C>
void tokenize(const std::basic_string<C>& str, std::vector<std::basic_string<C>>& tokens,
              const std::basic_string<C>& delimiters, bool allowEmpty = false) {
	size_t start = 0;
	size_t found = 0;
	while ((found = str.find_first_of(delimiters, start)) != std::basic_string<C>::npos) {
		if (allowEmpty || start < found)
			tokens.emplace_back(str.cbegin() + start, str.cbegin() + found);
		start = found + 1;
	}
	if (allowEmpty || start < str.size())
		tokens.emplace_back(str.cbegin() + start, str.cend());
}

template <typename C>
std::vector<const C*> toPtrVec(const std::vector<std::basic_string<C>>& sv) {
	std::vector<const C*> pv(sv.size());
	std::transform(sv.begin(), sv.end(), pv.begin(), [](const std::basic_string<C>& s) { return s.c_str(); });
	return pv;
}

#if defined(_WIN32)
#	include <Windows.h>
#elif defined(__linux__)
#	include <unistd.h>
#endif

std::filesystem::path getExecutablePath() {
#if defined(_WIN32)
	HMODULE hModule = GetModuleHandle(nullptr);
	if (hModule != NULL) {
		char path[MAX_PATH];
		const DWORD pathSize = GetModuleFileName(hModule, path, sizeof(path));
		if (pathSize > 0 && pathSize < MAX_PATH)
			return {path, path + pathSize};
		else
			return {};
	}
	else
		return {};
#elif defined(__linux__)
	const std::string proc = "/proc/" + std::to_string(getpid()) + "/exe";
	char path[1024];
	const size_t len = sizeof(path);
	const ssize_t bytes = readlink(proc.c_str(), path, len);
	if (bytes > 0 && bytes < (ssize_t)len)
		return {path, path + bytes};
	else
		return {};
#else
#	error unsupported build platform
#endif
}

} // namespace

namespace pcu {

/**
 * Helper function to convert a list of "<key>:<type>=<value>" strings into a prt::AttributeMap
 */
AttributeMapPtr createAttributeMapFromTypedKeyValues(const std::vector<std::string>& args) {
	AttributeMapBuilderPtr bld{prt::AttributeMapBuilder::create()};
	for (const std::string& a : args) {
		const std::wstring wa = toUTF16FromOSNarrow(a);
		std::vector<std::wstring> tokens;
		tokenize<wchar_t>(wa, tokens, L":=");
		if (tokens.size() == 3) {
			if (tokens[1] == L"string") {
				bld->setString(tokens[0].c_str(), tokens[2].c_str());
			}
			else if (tokens[1] == L"float") {
				try {
					double d = std::stod(tokens[2]);
					bld->setFloat(tokens[0].c_str(), d);
				}
				catch (std::exception& e) {
					std::wcerr << L"cannot set float attribute " << tokens[0] << ": " << e.what() << std::endl;
				}
			}
			else if (tokens[1] == L"int") {
				try {
					int32_t v = std::stoi(tokens[2]);
					bld->setInt(tokens[0].c_str(), v);
				}
				catch (std::exception& e) {
					std::wcerr << L"cannot set int attribute " << tokens[0] << ": " << e.what() << std::endl;
				}
			}
			else if (tokens[1] == L"bool") {
				bool v;
				std::wistringstream istr(tokens[2]);
				istr >> std::boolalpha >> v;
				if (!istr.fail())
					bld->setBool(tokens[0].c_str(), v);
				else
					std::wcerr << L"cannot set bool attribute " << tokens[0] << std::endl;
			}
			else if (tokens[1] == L"string[]") {
				std::vector<std::wstring> elems;
				tokenize<wchar_t>(tokens[2], elems, L",", true);
				const std::vector<const wchar_t*> stringArray = toPtrVec(elems);
				bld->setStringArray(tokens[0].c_str(), stringArray.data(), stringArray.size());
			}
			else if (tokens[1] == L"float[]") {
				std::vector<std::wstring> elems;
				tokenize<wchar_t>(tokens[2], elems, L",", true);
				std::vector<double> floatArray;
				floatArray.reserve(elems.size());
				for (const std::wstring& elem : elems) {
					try {
						floatArray.push_back(std::stod(elem));
					}
					catch (const std::exception& e) {
						std::wcerr << L"cannot set float array attribute " << tokens[0] << ": " << e.what()
						           << std::endl;
						break;
					}
				}
				if (floatArray.size() == elems.size())
					bld->setFloatArray(tokens[0].c_str(), floatArray.data(), floatArray.size());
			}
			else if (tokens[1] == L"int[]") {
				std::vector<std::wstring> elems;
				tokenize<wchar_t>(tokens[2], elems, L",", true);
				std::vector<int32_t> intArray;
				intArray.reserve(elems.size());
				for (const std::wstring& elem : elems) {
					try {
						intArray.push_back(std::stoi(elem));
					}
					catch (const std::exception& e) {
						std::wcerr << L"cannot set int array attribute " << tokens[0] << ": " << e.what() << std::endl;
						break;
					}
				}
				if (intArray.size() == elems.size())
					bld->setIntArray(tokens[0].c_str(), intArray.data(), intArray.size());
			}
			else if (tokens[1] == L"bool[]") {
				std::vector<std::wstring> elems;
				tokenize<wchar_t>(tokens[2], elems, L",", true);
				std::vector<uint8_t> boolArray;
				boolArray.reserve(elems.size());
				bool v;
				for (const std::wstring& elem : elems) {
					std::wistringstream istr(elem);
					istr >> std::boolalpha >> v;
					if (istr.fail()) {
						std::wcerr << L"cannot set bool array attribute " << tokens[0] << std::endl;
						break;
					}
					boolArray.push_back(v ? 1 : 0);
				}
				if (boolArray.size() == elems.size())
					bld->setBoolArray(tokens[0].c_str(), (const bool*)boolArray.data(), boolArray.size());
			}
			else
				std::wcout << L"warning: ignored key/value item: " << wa << std::endl;
		}
		else
			std::wcout << L"warning: ignored key/value item: " << wa << std::endl;
	}
	return AttributeMapPtr{bld->createAttributeMapAndReset()};
}

/**
 * Parse the command line arguments and setup the inputArgs struct.
 */
InputArgs::InputArgs(int argc, char* argv[]) : mStatus(RunStatus::FAILED) {
	// determine current path
	const std::filesystem::path executable = getExecutablePath();
	mInstallRootPath = executable.parent_path().parent_path(); // cmake installs the exe to install/bin

	// setup default values
	mEncoderID = ENCODER_ID_OBJ;
	mLogLevel = 2;
	mOutputPath = mInstallRootPath / "output";
	mInitialShapeAttrs = createAttributeMapFromTypedKeyValues({}); // PRT requires this in case no arguments are given

	// setup arg handling callbacks
	const CLI::callback_t convertRulePackagePath = [this](std::vector<std::string> arg) {
		if (arg.empty())
			return false;
		std::filesystem::path p = arg.front();
		if (!p.is_absolute())
			p = std::filesystem::current_path() / p;
		mRulePackageURI = pcu::toFileURI(p.generic_string());
		return true;
	};
	const CLI::callback_t convertShapeAttrs = [this](const std::vector<std::string>& argShapeAttrs) {
		mInitialShapeAttrs = createAttributeMapFromTypedKeyValues(argShapeAttrs);
		return true;
	};
	const CLI::callback_t convertEncOpts = [this](const std::vector<std::string>& argEncOpts) {
		mEncoderOpts = createAttributeMapFromTypedKeyValues(argEncOpts);
		return true;
	};
	const CLI::callback_t convertOutputPath = [this](std::vector<std::string> arg) {
		if (arg.empty())
			return false;
		std::filesystem::path output = arg.front();
		if (output.is_absolute())
			mOutputPath = output;
		else
			mOutputPath = mInstallRootPath / output;
		return true;
	};
	const CLI::callback_t convertInitialShapeGeoPath = [this](std::vector<std::string> arg) {
		if (arg.empty())
			return false;
		std::filesystem::path p = arg.front();
		if (!p.is_absolute())
			p = std::filesystem::current_path() / p;
		mInitialShapeGeoURI = pcu::toFileURI(p.generic_string()); // we let prt deal with invalid file paths etc
		return true;
	};
	const CLI::callback_t convertInfoFilePath = [this](std::vector<std::string> arg) {
		if (arg.empty())
			return false;
		mInfoFile = arg.front();
		if (!mInfoFile.is_absolute())
			mInfoFile = std::filesystem::current_path() / mInfoFile;
		return true;
	};

	// setup options
	CLI::App app{"prt4cmd - command line example for the CityEngine Procedural RunTime"};
	// clang-format off
	const auto optVer =  app.add_flag  ("-v,--version",                                     "Show CityEngine SDK version.");
	                     app.add_option("-l,--log-level",       mLogLevel,                  "Set log filter level:\n1 = debug, "
	                                                                                        "2 = info, 3 = warning, 4 = error, "
	                                                                                        "5 = fatal, >5 = no output");
	                     app.add_option("-o,--output",          convertOutputPath,          "Set the output path for the callbacks.");
	                     app.add_option("-e,--encoder",         mEncoderID,                 "The encoder ID, e.g. 'com.esri.prt.codecs.OBJEncoder'.");
	const auto optRPK =  app.add_option("-p,--rule-package",    convertRulePackagePath,     "Set the rule package path.");
	const auto optAttr=  app.add_option("-a,--shape-attr",      convertShapeAttrs,          "Set one initial shape attribute with "
	                                                                                        "syntax <name>:<type>=<value>\n"
	                                                                                        "type = {string,float,int,bool,"
	                                                                                        "string[],float[],int[],bool[]}\n"
                                                                                            "Array elements must be comma-separated.\n"
	                                                                                        "Can be specified multiple times.");
	                     app.add_option("-g,--shape-geo",       convertInitialShapeGeoPath, "(Optional) Path to a file with shape geometry.");
	const auto encOpts = app.add_option("-z,--encoder-option",  convertEncOpts,             "Set one encoder option with syntax "
	                                                                                        "<name>:<type>=<value>\n"
	                                                                                        "type = {string,float,int,bool,"
	                                                                                        "string[],float[],int[],bool[]}\n"
	                                                                                        "Can be specified multiple times.");
	const auto optInfo = app.add_option("-i,--info",            convertInfoFilePath,        "Write XML Extension Information to file.");
	// clang-format on

	// setup option requirements
	optInfo->excludes(optRPK);
	optAttr->expected(0, -1);
	encOpts->expected(0, -1);

	// parse options
	try {
		if (argc <= 1)
			throw CLI::CallForHelp();
		app.parse(argc, argv);
	}
	catch (const CLI::Error& e) {
		app.exit(e);
		return;
	}

	// basic validation of input args
	if (optVer->count() == 1) {
		std::cout << prt::getVersion()->mFullName << std::endl;
		mStatus = RunStatus::DONE;
	}
	else if (optInfo->count() == 0 && optRPK->count() == 0) {
		std::cerr << "error: at least one of '" << optRPK->get_name() << "' or '" << optInfo->get_name()
		          << "' is required." << std::endl;
	}
	else if (optRPK->count() > 0 && !std::filesystem::exists(mOutputPath)) {
		if (std::filesystem::create_directories(mOutputPath))
			mStatus = RunStatus::CONTINUE;
		else
			std::cerr << "error: failed to create output directory at " << mOutputPath << std::endl;
	}
	else
		mStatus = RunStatus::CONTINUE;
}

/**
 * String conversion functions
 */

template <typename inC, typename outC, typename FUNC>
std::basic_string<outC> callAPI(FUNC f, const std::basic_string<inC>& s) {
	std::vector<outC> buffer(s.size());
	size_t size = buffer.size();
	f(s.c_str(), buffer.data(), &size, nullptr);
	if (size > buffer.size()) {
		buffer.resize(size);
		f(s.c_str(), buffer.data(), &size, nullptr);
	}
	return std::basic_string<outC>{buffer.data()};
}

std::string toOSNarrowFromUTF16(const std::wstring& osWString) {
	return callAPI<wchar_t, char>(prt::StringUtils::toOSNarrowFromUTF16, osWString);
}

std::wstring toUTF16FromOSNarrow(const std::string& osString) {
	return callAPI<char, wchar_t>(prt::StringUtils::toUTF16FromOSNarrow, osString);
}

std::wstring toUTF16FromUTF8(const std::string& utf8String) {
	return callAPI<char, wchar_t>(prt::StringUtils::toUTF16FromUTF8, utf8String);
}

std::string toUTF8FromOSNarrow(const std::string& osString) {
	std::wstring utf16String = toUTF16FromOSNarrow(osString);
	return callAPI<wchar_t, char>(prt::StringUtils::toUTF8FromUTF16, utf16String);
}

std::string percentEncode(const std::string& utf8String) {
	return callAPI<char, char>(prt::StringUtils::percentEncode, utf8String);
}

/**
 * codec info functions
 */

template <typename C, typename FUNC>
std::basic_string<C> callAPI(FUNC f, size_t initialSize) {
	std::vector<C> buffer(initialSize, ' ');

	size_t actualSize = initialSize;
	f(buffer.data(), &actualSize, nullptr);
	buffer.resize(actualSize);

	if (initialSize < actualSize)
		f(buffer.data(), &actualSize, nullptr);

	return std::basic_string<C>{buffer.data()};
}

std::string objectToXML(const prt::Object* obj) {
	auto toXMLFunc =
	        std::bind(&prt::Object::toXML, obj, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	return callAPI<char>(toXMLFunc, 4096);
}

RunStatus codecInfoToXML(const std::filesystem::path& infoFilePath) {
	const std::wstring encIDsStr{callAPI<wchar_t>(prt::listEncoderIds, 1024)};
	const std::wstring decIDsStr{callAPI<wchar_t>(prt::listDecoderIds, 1024)};

	std::vector<std::wstring> encIDs, decIDs;
	tokenize<wchar_t>(encIDsStr, encIDs, L";");
	tokenize<wchar_t>(decIDsStr, decIDs, L";");

	try {
		std::ofstream xml(infoFilePath);
		xml << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n\n";

		xml << "<Codecs build=\"" << prt::getVersion()->mVersion << "\" buildDate=\"" << prt::getVersion()->mBuildDate
		    << "\" buildConfig=\"" << prt::getVersion()->mBuildConfig << "\">\n";

		xml << "<Encoders>\n";
		for (const std::wstring& encID : encIDs) {
			prt::Status s = prt::STATUS_UNSPECIFIED_ERROR;
			const EncoderInfoPtr encInfo{prt::createEncoderInfo(encID.c_str(), &s)};
			if (s == prt::STATUS_OK && encInfo)
				xml << objectToXML(encInfo.get()) << std::endl;
			else
				LOG_ERR << L"encoder not found for ID: " << encID << std::endl;
		}
		xml << "</Encoders>\n";

		xml << "<Decoders>\n";
		for (const std::wstring& decID : decIDs) {
			prt::Status s = prt::STATUS_UNSPECIFIED_ERROR;
			const DecoderInfoPtr decInfo{prt::createDecoderInfo(decID.c_str(), &s)};
			if (s == prt::STATUS_OK && decInfo)
				xml << objectToXML(decInfo.get()) << std::endl;
			else
				LOG_ERR << L"decoder not found for ID: " << decID << std::endl;
		}
		xml << "</Decoders>\n";

		xml << "</Codecs>\n";
		xml.close();

		LOG_INF << "Dumped codecs info to " << infoFilePath;
	}
	catch (std::exception& e) {
		LOG_ERR << "Exception while dumping codec info: " << e.what();
		return RunStatus::FAILED;
	}

	return RunStatus::DONE;
}

URI toFileURI(const std::string& p) {
	const std::string utf8Path = toUTF8FromOSNarrow(p);
	const std::string u8PE = percentEncode(utf8Path);
	return FILE_SCHEMA + u8PE;
}

AttributeMapPtr createValidatedOptions(const std::wstring& encID, const AttributeMapPtr& unvalidatedOptions) {
	const EncoderInfoPtr encInfo{prt::createEncoderInfo(encID.c_str())};
	const prt::AttributeMap* validatedOptions = nullptr;
	encInfo->createValidatedOptionsAndStates(unvalidatedOptions.get(), &validatedOptions);
	return AttributeMapPtr(validatedOptions);
}

} // namespace pcu
