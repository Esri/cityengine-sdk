#include "utils.h"
#include "logging.h"

#include "prt/StringUtils.h"

#include "boost/program_options.hpp"
#include "boost/algorithm/string.hpp"

#include <iostream>
#include <fstream>


namespace {

#ifdef _WIN32
	const std::wstring FILE_SCHEMA = L"file:/";
#else
	const std::wstring FILE_SCHEMA = L"file:";
#endif

} // namespace


namespace pcu {

/**
 * Helper function to convert a list of "<key>:<type>=<value>" strings into a prt::AttributeMap
 */
AttributeMapPtr createAttributeMapFromTypedKeyValues(const std::vector<std::wstring>& args) {
	AttributeMapBuilderPtr bld{prt::AttributeMapBuilder::create()};
	for (const std::wstring& a: args) {
		std::vector<std::wstring> splits;
		boost::split(splits, a, boost::algorithm::is_any_of(":="), boost::token_compress_on);
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
			std::wcout << L"warning: ignored key/value item: " << a << std::endl;
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

	std::vector<std::wstring> argShapeAttributes, argEncOpts;

	boost::program_options::options_description desc("Available Options");
	desc.add_options()("help,h", "This very help screen.")("version,v", "Show CityEngine SDK version.");
	desc.add_options()(
			"log-level,l",
			boost::program_options::value<int>(&inputArgs.mLogLevel)->default_value(2),
			"Set log filter level: 1 = debug, 2 = info, 3 = warning, 4 = error, 5 = fatal, >5 = no output"
	);
	desc.add_options()(
			"output,o",
			boost::program_options::value<boost::filesystem::path>(&inputArgs.mOutputPath)->default_value(inputArgs.mWorkDir / "output"),
			"Set the output path for the callbacks."
	);
	desc.add_options()(
			"encoder,e",
			boost::program_options::wvalue<std::wstring>(&inputArgs.mEncoderID)->default_value(inputArgs.mEncoderID, toOSNarrowFromUTF16(inputArgs.mEncoderID)), "The encoder ID, e.g. 'com.esri.prt.codecs.OBJEncoder'.");
	desc.add_options()(
			"rule-package,p",
			boost::program_options::value<std::string>(&inputArgs.mRulePackage),
			"Set the rule package path."
	);
	desc.add_options()(
			"shape-attr,a",
			boost::program_options::wvalue<std::vector<std::wstring> >(&argShapeAttributes),
			"Set a initial shape attribute (syntax is <name>:<type>=<value>, type = {string,float,int,bool})."
	);
	desc.add_options()(
			"shape-geo,g",
			boost::program_options::wvalue<std::wstring>(&inputArgs.mInitialShapeGeo),
			"(Optional) Path to a file with shape geometry");
	desc.add_options()(
			"encoder-option,z",
			boost::program_options::wvalue<std::vector<std::wstring> >(&argEncOpts),
			"Set a encoder option (syntax is <name>:<type>=<value>, type = {string,float,int,bool})."
	);
	desc.add_options()(
			"info,i",
			boost::program_options::value<boost::filesystem::path>(&inputArgs.mInfoFile), "Write XML Extension Information to file"
	);
	desc.add_options()(
			"license-server,s",
			boost::program_options::value<std::string>(&inputArgs.mLicHost),
			"License Server Host Name, example: 27000@myserver.example.com"
	);
	desc.add_options()(
			"license-feature,f",
			boost::program_options::value<std::string>(&inputArgs.mLicFeature),
			"License Feature to use, one of CityEngBasFx, CityEngBas, CityEngAdvFx, CityEngAdv"
	);

	boost::program_options::positional_options_description posDesc;
	posDesc.add("shape-ref", -1);

	boost::program_options::variables_map vm;
	boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(desc).positional(posDesc).run(), vm);
	boost::program_options::notify(vm);

	if (argc == 1 || vm.count("help")) {
		std::cout << desc << std::endl;
		return false;
	}

	if (vm.count("version")) {
		std::cout << prt::getVersion()->mFullName << std::endl;
		return false;
	}

	// make sure the path to the initial shape geometry is a valid URI
	if (!inputArgs.mInitialShapeGeo.empty()) {
		boost::filesystem::path isGeoPath = toOSNarrowFromUTF16(inputArgs.mInitialShapeGeo); // workaround for boost 1.41
		if (boost::filesystem::exists(isGeoPath)) {
			inputArgs.mInitialShapeGeo = toFileURI(isGeoPath);
		}
		else {
			std::wcerr << L"path to initial shape uri is not valid: " << inputArgs.mInitialShapeGeo << std::endl;
			return false;
		}
	}

	inputArgs.mInitialShapeAttrs = createAttributeMapFromTypedKeyValues(argShapeAttributes);
	inputArgs.mEncoderOpts = createAttributeMapFromTypedKeyValues(argEncOpts);

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

std::string toUTF8FromOSNarrow(const std::string& osString) {
	std::wstring utf16String = toUTF16FromOSNarrow(osString);
	return callAPI<wchar_t, char>(prt::StringUtils::toUTF8FromUTF16, utf16String);
}

std::wstring percentEncode(const std::string& utf8String) {
	std::string pe{ callAPI<char, char>(prt::StringUtils::percentEncode, utf8String) };
	return callAPI<char, wchar_t>(prt::StringUtils::toUTF16FromUTF8, pe);
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

std::wstring toFileURI(const boost::filesystem::path& p) {
	std::string utf8Path = toUTF8FromOSNarrow(p.generic_string());
	std::wstring pecString = percentEncode(utf8Path);
	return FILE_SCHEMA + pecString;
}

AttributeMapPtr createValidatedOptions(const wchar_t* encID, const AttributeMapPtr& unvalidatedOptions) {
	EncoderInfoPtr encInfo{prt::createEncoderInfo(encID)};
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
