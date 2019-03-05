#include "PRTModifierAction.h"
#include "PRTModifierCommand.h"
#include "PRTModifierNode.h"
#include "node/PRTMaterialNode.h"
#include "node/Utilities.h"

#include <maya/MFnPlugin.h>


namespace {
	const wchar_t*      PRT_EXT_SUBDIR = L"ext";
	const prt::LogLevel PRT_LOG_LEVEL = prt::LOG_DEBUG;
	const bool          ENABLE_LOG_CONSOLE = true;
	const bool          ENABLE_LOG_FILE = false;
} // namespace

// called when the plug-in is loaded into Maya.
MStatus initializePlugin(MObject obj)
{

	if (ENABLE_LOG_CONSOLE) {
		PRTModifierAction::theLogHandler = prt::ConsoleLogHandler::create(prt::LogHandler::ALL, prt::LogHandler::ALL_COUNT);
		prt::addLogHandler(PRTModifierAction::theLogHandler);
	}

	if (ENABLE_LOG_FILE) {
		const std::string logPath = PRTModifierAction::getPluginRoot() + prtu::getDirSeparator<char>() + "prt4maya.log";
		std::wstring wLogPath(logPath.length(), L' ');
		std::copy(logPath.begin(), logPath.end(), wLogPath.begin());
		PRTModifierAction::theFileLogHandler = prt::FileLogHandler::create(prt::LogHandler::ALL, prt::LogHandler::ALL_COUNT, wLogPath.c_str());
		prt::addLogHandler(PRTModifierAction::theFileLogHandler);
	}


	prtu::dbg("initialized prt logger");

	const std::string& pluginRoot = PRTModifierAction::getPluginRoot();
	std::wstring wPluginRoot(pluginRoot.length(), L' ');
	std::copy(pluginRoot.begin(), pluginRoot.end(), wPluginRoot.begin());

	const std::wstring prtExtPath = wPluginRoot + PRT_EXT_SUBDIR;
	prtu::wdbg(L"looking for prt extensions at %ls", prtExtPath.c_str());

	prt::Status status = prt::STATUS_UNSPECIFIED_ERROR;
	const wchar_t* prtExtPathPOD = prtExtPath.c_str();
	PRTModifierAction::thePRT = prt::init(&prtExtPathPOD, 1, PRT_LOG_LEVEL, &status);

	if (PRTModifierAction::thePRT == nullptr || status != prt::STATUS_OK)
		return MS::kFailure;

	PRTModifierAction::theCache = prt::CacheObject::create(prt::CacheObject::CACHE_TYPE_DEFAULT);

	MFnPlugin plugin(obj, "Esri R&D Center Zurich", "1.0", "Any");

	MCHECK(plugin.registerCommand("prtAssign", PRTModifierCommand::creator));

	MCHECK(plugin.registerNode("prt",
		PRTModifierNode::id,
		PRTModifierNode::creator,
		PRTModifierNode::initialize));

	MCHECK(plugin.registerNode("prtMaterial", PRTMaterialNode::id, &PRTMaterialNode::creator, &PRTMaterialNode::initialize, MPxNode::kDependNode));
	MCHECK(plugin.registerUI("prt4mayaCreateUI", "prt4mayaDeleteUI"));

	return MStatus::kSuccess;
}

// called when the plug-in is unloaded from Maya.
MStatus uninitializePlugin(MObject obj)
{

	PRTModifierAction::theCache->destroy();
	PRTModifierAction::thePRT->destroy();

	if (ENABLE_LOG_CONSOLE) {
		prt::removeLogHandler(PRTModifierAction::theLogHandler);
		PRTModifierAction::theLogHandler->destroy();
	}
	if (ENABLE_LOG_FILE) {
		prt::removeLogHandler(PRTModifierAction::theFileLogHandler);
		PRTModifierAction::theFileLogHandler->destroy();
	}

	MStatus   status;
	MFnPlugin plugin(obj);

	MCHECK(plugin.deregisterCommand("prtAssign"));
	MCHECK(plugin.deregisterNode(PRTModifierNode::id));
	MCHECK(plugin.deregisterNode(PRTMaterialNode::id));

	return status;
}
