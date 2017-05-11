#include "utils.h"
#include "logging.h"

#include "prt/StringUtils.h"

#include "CLI11.hpp"
#include "boost/algorithm/string.hpp"

#include <iostream>
#include <fstream>


namespace {

#ifdef _WIN32
	const std::string FILE_SCHEMA = "file:/";
#else
	const std::string FILE_SCHEMA = "file:";
#endif

const char* ENCODER_ID_OBJ = "com.esri.prt.codecs.OBJEncoder";

} // namespace


namespace pcu {

/**
 * Helper function to convert a list of "<key>:<type>=<value>" strings into a prt::AttributeMap
 */
AttributeMapPtr createAttributeMapFromTypedKeyValues(const std::vector<std::string>& args) {
	AttributeMapBuilderPtr bld{prt::AttributeMapBuilder::create()};
	for (const std::string& a: args) {
		const std::wstring wa = toUTF16FromOSNarrow(a);
		std::vector<std::wstring> splits;
		boost::split(splits, wa, boost::algorithm::is_any_of(":="), boost::token_compress_on);
		if (splits.size() == 3) {
			if (splits[1] == L"string") {
				bld->setString(splits[0].c_str(), splits[2].c_str());
			}
			else if (splits[1] == L"float") {
				try {
					double d = std::stod(splits[2]);
					bld->setFloat(splits[0].c_str(), d);
				} catch (std::exception& e) {
					std::wcerr << L"cannot set float attribute " << splits[0] << ": " << e.what() << std::endl;
				}
			}
			else if (splits[1] == L"int") {
				try {
					int32_t v = std::stoi(splits[2]);
					bld->setInt(splits[0].c_str(), v);
				} catch (std::exception& e) {
					std::wcerr << L"cannot set int attribute " << splits[0] << ": " << e.what() << std::endl;
				}
			}
			else if (splits[1] == L"bool") {
				bool v;
				std::wistringstream istr(splits[2]);
				istr >> std::boolalpha >> v;
				if (!istr.fail())
					bld->setBool(splits[0].c_str(), v);
				else
					std::wcerr << L"cannot set bool attribute " << splits[0] << std::endl;
			}
		}
		else
			std::wcout << L"warning: ignored key/value item: " << wa << std::endl;
	}
	return AttributeMapPtr{bld->createAttributeMapAndReset()};
}

/**
 * Parse the command line arguments and setup the inputArgs struct.
 */
bool initInputArgs(int argc, char *argv[], InputArgs& inputArgs) {
	// determine current path
	boost::filesystem::path executablePath = boost::filesystem::system_complete(boost::filesystem::path(argv[0]));
	inputArgs.mWorkDir = executablePath.parent_path().parent_path();

	// setup default values
	inputArgs.mEncoderID  = ENCODER_ID_OBJ;
	inputArgs.mLogLevel   = 2;
	inputArgs.mOutputPath = inputArgs.mWorkDir / "output";

	CLI::callback_t convertShapeAttrs = [&inputArgs](std::vector<std::string> argShapeAttrs) {
		inputArgs.mInitialShapeAttrs = createAttributeMapFromTypedKeyValues(argShapeAttrs);
		return true;
	};

	CLI::callback_t convertEncOpts = [&inputArgs](std::vector<std::string> argEncOpts) {
		inputArgs.mEncoderOpts = createAttributeMapFromTypedKeyValues(argEncOpts);
		return true;
	};

	CLI::App app{"prt4cmd - command line example for the CityEngine Procedural RunTime"};
	auto optHelp = app.add_flag("-h,--help", "This very help screen.");
	auto optVer  = app.add_flag("-v,--version", "Show CityEngine SDK version.");
	app.add_option("-l,--log-level", inputArgs.mLogLevel, "Set log filter level: 1 = debug, 2 = info, 3 = warning, 4 = error, 5 = fatal, >5 = no output");
	app.add_option("-o,--output", inputArgs.mOutputPath, "Set the output path for the callbacks.");
	app.add_option("-e,--encoder", inputArgs.mEncoderID, "The encoder ID, e.g. 'com.esri.prt.codecs.OBJEncoder'.");
	app.add_option("-p,--rule-package", inputArgs.mRulePackage, "Set the rule package path.");
	app.add_option("-a,--shape-attr", convertShapeAttrs, "Set a initial shape attribute (syntax is <name>:<type>=<value>, type = {string,float,int,bool}).");
	app.add_option("-g,--shape-geo", inputArgs.mInitialShapeGeo, "(Optional) Path to a file with shape geometry");
	app.add_option("-z,--encoder-option", convertEncOpts, "Set a encoder option (syntax is <name>:<type>=<value>, type = {string,float,int,bool}).");
	app.add_option("-i,--info", inputArgs.mInfoFile, "Write XML Extension Information to file");
	app.add_option("-s,--license-server", inputArgs.mLicHost, "License Server Host Name, example: 27000@myserver.example.com");
	app.add_option("-f,--license-feature", inputArgs.mLicFeature, "License Feature to use, one of CityEngBasFx, CityEngBas, CityEngAdvFx, CityEngAdv");
	//app.add_option("shape-ref", -1);

	try {
  	  app.parse(argc, argv);
	} catch (const CLI::ParseError &e) {
		std::cerr << e.what() << std::endl;
    	return false;
	}

	if (optHelp->count() == 1) {
		std::cout << "the help" << std::endl;
		return false;
	}

	if (optVer->count() == 1) {
		std::cout << prt::getVersion()->mFullName << std::endl;
		return false;
	}

	// make sure the path to the initial shape geometry is a valid URI
	if (!inputArgs.mInitialShapeGeo.empty()) {
		boost::filesystem::path isGeoPath = inputArgs.mInitialShapeGeo;
		if (boost::filesystem::exists(isGeoPath)) {
			inputArgs.mInitialShapeGeo = toFileURI(isGeoPath);
		}
		else {
			std::cerr << L"path to initial shape uri is not valid: " << inputArgs.mInitialShapeGeo << std::endl;
			return false;
		}
	}

	return true;
}


/**
 * String conversion functions
 */

template<typename inC, typename outC, typename FUNC>
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

template<typename C, typename FUNC>
std::basic_string<C> callAPI(FUNC f, size_t initialSize) {
	std::vector<C> buffer(initialSize, ' ');

	size_t actualSize = initialSize;
	f(buffer.data(), &actualSize, nullptr);
	buffer.resize(actualSize);

	if (initialSize < actualSize)
		f(buffer.data(), &actualSize, nullptr);

	return std::basic_string<C>{buffer.data()};
}

std::string objectToXML(prt::Object const* obj) {
	auto toXMLFunc = std::bind(&prt::Object::toXML, obj, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	return callAPI<char>(toXMLFunc, 4096);
}

void codecInfoToXML(InputArgs& inputArgs) {
	std::wstring encIDsStr{ callAPI<wchar_t>(prt::listEncoderIds, 1024) };
	std::wstring decIDsStr{ callAPI<wchar_t>(prt::listDecoderIds, 1024) };

	std::vector<std::wstring> encIDs, decIDs;

	boost::trim_if(encIDsStr, boost::is_any_of("; "));
	boost::split(encIDs, encIDsStr, boost::is_any_of(L";"), boost::token_compress_on);

	boost::trim_if(decIDsStr, boost::is_any_of("; "));
	boost::split(decIDs, decIDsStr, boost::is_any_of(L";"), boost::token_compress_on);

	std::ofstream xml(inputArgs.mInfoFile.string());
	xml << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n\n";

	xml << "<Codecs build=\"" << prt::getVersion()->mVersion
		<< "\" buildDate=\"" << prt::getVersion()->mBuildDate
		<< "\" buildConfig=\"" << prt::getVersion()->mBuildConfig
		<< "\">\n";

	xml << "<Encoders>\n";
	for (const std::wstring& encID: encIDs) {
		prt::Status s = prt::STATUS_UNSPECIFIED_ERROR;
		EncoderInfoPtr encInfo{prt::createEncoderInfo(encID.c_str(), &s)};
		if (s == prt::STATUS_OK && encInfo)
			xml << objectToXML(encInfo.get()) << std::endl;
		else
			std::wcout << L"encoder not found for ID: " << encID << std::endl;
	}
	xml << "</Encoders>\n";

	xml << "<Decoders>\n";
	for (const std::wstring& decID: decIDs) {
		prt::Status s = prt::STATUS_UNSPECIFIED_ERROR;
		DecoderInfoPtr decInfo{prt::createDecoderInfo(decID.c_str(), &s)};
		if (s == prt::STATUS_OK && decInfo)
			xml << objectToXML(decInfo.get()) << std::endl;
		else
			std::wcout << L"decoder not found for ID: " << decID << std::endl;
	}
	xml << "</Decoders>\n";

	xml << "</Codecs>\n";
	xml.close();

	LOG_INF << "Dumped codecs info to " << inputArgs.mInfoFile;
}

std::string toFileURI(const boost::filesystem::path& p) {
	std::string utf8Path = toUTF8FromOSNarrow(p.generic_string());
	std::string u8PE = percentEncode(utf8Path);
	return FILE_SCHEMA + u8PE;
}

AttributeMapPtr createValidatedOptions(const std::wstring& encID, const AttributeMapPtr& unvalidatedOptions) {
	EncoderInfoPtr encInfo{prt::createEncoderInfo(encID.c_str())};
	const prt::AttributeMap* validatedOptions = nullptr;
	encInfo->createValidatedOptionsAndStates(unvalidatedOptions.get(), &validatedOptions);
	return AttributeMapPtr(validatedOptions);
}

std::string getSharedLibraryPrefix() {
#if defined(_WIN32)
	return "";
#elif defined(__APPLE__)
	return "lib";
#elif defined(__linux__)
	return "lib";
#else
#	error unsupported build platform
#endif
}

std::string getSharedLibrarySuffix() {
#if defined(_WIN32)
	return ".dll";
#elif defined(__APPLE__)
	return ".dylib";
#elif defined(__linux__)
	return ".so";
#else
#	error unsupported build platform
#endif
}

} // namespace pcu
