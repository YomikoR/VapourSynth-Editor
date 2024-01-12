#include "vapoursynth_plugins_manager.h"

#include "../../../common-src/helpers.h"
#include "../../../common-src/settings/settings_manager.h"

#include <QDir>
#include <QLibrary>
#include <QFileInfoList>
#include <QSettings>
#include <QProcessEnvironment>
#include <algorithm>

//==============================================================================

const char CORE_PLUGINS_FILEPATH[] = "core";

//==============================================================================

VapourSynthPluginsManager::VapourSynthPluginsManager(
	SettingsManager * a_pSettingsManager, QObject * a_pParent):
	QObject(a_pParent)
	, m_pluginsList()
	, m_currentPluginPath()
	, m_pluginAlreadyLoaded(false)
	, m_pSettingsManager(a_pSettingsManager)
{
	if(a_pParent)
	{
		connect(this, SIGNAL(signalWriteLogMessage(int, const QString &)),
		a_pParent, SLOT(slotWriteLogMessage(int, const QString &)));
	}
	slotRefill();
}

// END OF VapourSynthPluginsManager::VapourSynthPluginsManager(
//		SettingsManager * a_pSettingsManager, QObject * a_pParent)
//==============================================================================

VapourSynthPluginsManager::~VapourSynthPluginsManager()
{
	slotClear();
}

// END OF VapourSynthPluginsManager::~VapourSynthPluginsManager()
//==============================================================================

void VapourSynthPluginsManager::getCorePlugins()
{
	QString libraryName("vapoursynth");
	QString libraryFullPath;
	QLibrary vsLibrary(libraryName);
	bool loaded = false;

	auto load_from_registry = [&]()
	{
#ifdef Q_OS_WIN
		QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE",
			QSettings::NativeFormat);
		libraryFullPath =
			settings.value("VapourSynth/VapourSynthDLL").toString();

		if(!libraryFullPath.isEmpty())
		{
			vsLibrary.unload();
			vsLibrary.setFileName(libraryFullPath);
			loaded = vsLibrary.load();
		}
#endif // Q_OS_WIN
	};

	auto load_from_path = [&]()
	{
		vsLibrary.unload();
		vsLibrary.setFileName(libraryName);
		loaded = vsLibrary.load();
	};

	auto load_from_list = [&]()
	{
		QStringList librarySearchPaths =
			m_pSettingsManager->getVapourSynthLibraryPaths();
		for(const QString & path : librarySearchPaths)
		{
			vsLibrary.unload();
			libraryFullPath = vsedit::resolvePathFromApplication(path) +
				QString("/") + libraryName;
			vsLibrary.setFileName(libraryFullPath);
			loaded = vsLibrary.load();
			if(loaded)
				break;
		}
	};

	if(m_pSettingsManager->getPreferVSLibrariesFromList())
	{
		if(!loaded) load_from_list();
		if(!loaded) load_from_registry();
		if(!loaded) load_from_path();
	}
	else
	{
		if(!loaded) load_from_registry();
		if(!loaded) load_from_path();
		if(!loaded) load_from_list();
	}

	if(!loaded)
	{
		emit signalWriteLogMessage(mtCritical, "VapourSynth plugins manager: "
			"Failed to load vapoursynth library!\n"
			"Please set up the library search paths in settings.");
		return;
	}

	VSGetVapourSynthAPI getVapourSynthAPI =
		(VSGetVapourSynthAPI)vsLibrary.resolve("getVapourSynthAPI");
	if(!getVapourSynthAPI)
	{ // Win32 fallback
		getVapourSynthAPI =
			(VSGetVapourSynthAPI)vsLibrary.resolve("_getVapourSynthAPI@4");
	}
	if(!getVapourSynthAPI)
	{
		emit signalWriteLogMessage(mtCritical, "VapourSynth plugins manager: "
			"Failed to get entry in vapoursynth library!");
		vsLibrary.unload();
		return;
	}

	const VSAPI * cpVSAPI = getVapourSynthAPI(VS_MAKE_VERSION(4, 0));
	if(!cpVSAPI)
	{
		emit signalWriteLogMessage(mtCritical, "VapourSynth plugins manager: "
			"Failed to get VapourSynth API!");
		vsLibrary.unload();
		return;
	}

	VSCore * pCore = cpVSAPI->createCore(0);
	if(!pCore)
	{
		emit signalWriteLogMessage(mtCritical, "VapourSynth plugins manager: "
			"Failed to create VapourSynth core!");
		vsLibrary.unload();
		return;
	}

	VSPlugin * pPlugin = cpVSAPI->getNextPlugin(nullptr, pCore);
	while(pPlugin)
	{
		m_pluginsList.emplace_back();
		VSData::Plugin & pluginData = m_pluginsList.back();
		pluginData.filepath = cpVSAPI->getPluginPath(pPlugin);
		pluginData.id = cpVSAPI->getPluginID(pPlugin);
		pluginData.pluginNamespace = cpVSAPI->getPluginNamespace(pPlugin);
		pluginData.name = cpVSAPI->getPluginName(pPlugin);

		VSPluginFunction * pPluginFunction =
			cpVSAPI->getNextPluginFunction(nullptr, pPlugin);
		while(pPluginFunction)
		{
			const char * pluginFunctionName =
				cpVSAPI->getPluginFunctionName(pPluginFunction);
			const char * pluginFunctionArgs =
				cpVSAPI->getPluginFunctionArguments(pPluginFunction);

			VSData::Function function = parseFunctionSignature(pluginFunctionName,
				pluginFunctionArgs);
			pluginData.functions.push_back(function);

			pPluginFunction = cpVSAPI->getNextPluginFunction(pPluginFunction, pPlugin);
		}
		pPlugin = cpVSAPI->getNextPlugin(pPlugin, pCore);
	}

	cpVSAPI->freeCore(pCore);
	vsLibrary.unload();
}

// END OF void VapourSynthPluginsManager::getCorePlugins()
//==============================================================================

void VapourSynthPluginsManager::pollPaths(const QStringList & a_pluginsPaths)
{
	for(const QString & dirPath : a_pluginsPaths)
	{
		QString absolutePath = vsedit::resolvePathFromApplication(dirPath);
		QFileInfoList fileInfoList = QDir(absolutePath).entryInfoList();
		for(const QFileInfo & fileInfo : fileInfoList)
		{
			QString filePath = fileInfo.absoluteFilePath();
			QLibrary plugin(filePath);
			VSInitPlugin initVSPlugin =
				(VSInitPlugin)plugin.resolve("VapourSynthPluginInit");
			if(!initVSPlugin)
			{ // Win32 fallback
				initVSPlugin =
					(VSInitPlugin)plugin.resolve("_VapourSynthPluginInit@12");
			}
			if(!initVSPlugin)
				continue;
			m_currentPluginPath = filePath;

			initVSPlugin(reinterpret_cast<VSPlugin *>(this), &m_VSPAPI);
			plugin.unload();
			m_pluginAlreadyLoaded = false;
		}
	}
}

// END OF void VapourSynthPluginsManager::pollPaths(
//		const QStringList & a_pluginsPaths)
//==============================================================================

QStringList VapourSynthPluginsManager::functions() const
{
	QStringList functionsList;
	for(const VSData::Plugin & plugin : m_pluginsList)
	{
		QString functionNamespace = plugin.pluginNamespace + ".";
		for(const VSData::Function & function : plugin.functions)
			functionsList << functionNamespace + function.toString();
	}

	return functionsList;
}

// END OF QStringList VapourSynthPluginsManager::functions() const
//==============================================================================

VSPluginsList VapourSynthPluginsManager::pluginsList() const
{
	return m_pluginsList;
}

// END OF VSPluginsList VapourSynthPluginsManager::pluginsList() const
//==============================================================================

VSData::Function VapourSynthPluginsManager::parseFunctionSignature(
	const QString & a_name, const QString & a_arguments)
{
	VSData::Function function;
	function.name = a_name;
	QStringList argumentsList = a_arguments.split(';', Qt::SkipEmptyParts);
	if(argumentsList.size() == 0)
		return function;

	// This is true for arguments lists returned by VSAPI.
	if(argumentsList[0] == a_name)
		argumentsList.removeFirst();

	for(const QString& argumentString : argumentsList)
	{
		QStringList argumentParts = argumentString.split(':');
		int partsNumber = argumentParts.size();
		if(partsNumber < 2)
			continue;

		function.arguments.emplace_back();
		VSData::FunctionArgument & argument = function.arguments.back();
		argument.name = argumentParts[0];
		argument.type = argumentParts[1];

		for(int i = 2; i < partsNumber; ++i)
		{
			if(argumentParts[i] == "opt")
				argument.optional = true;
			else if(argumentParts[i] == "empty")
				argument.empty = true;
		}
	}

	return function;
}

// END OF VSData::Function VapourSynthPluginsManager::parseFunctionSignature(
//		const QString & a_name, const QString & a_arguments)
//==============================================================================

void VapourSynthPluginsManager::slotClear()
{
	m_pluginsList.clear();
}

// END OF void VapourSynthPluginsManager::slotClear()
//==============================================================================

void VapourSynthPluginsManager::slotSort()
{
	std::stable_sort(m_pluginsList.begin(), m_pluginsList.end());
	for(VSData::Plugin & plugin : m_pluginsList)
		std::stable_sort(plugin.functions.begin(), plugin.functions.end());
}

// END OF void VapourSynthPluginsManager::slotSort()
//==============================================================================

void VapourSynthPluginsManager::slotRefill()
{
	slotClear();
	getCorePlugins();
	QStringList pluginsPaths = m_pSettingsManager->getVapourSynthPluginsPaths();
	pollPaths(pluginsPaths);
	slotSort();
}

// END OF void VapourSynthPluginsManager::slotRefill()
//==============================================================================
