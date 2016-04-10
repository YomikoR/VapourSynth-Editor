#include <QDir>
#include <QLibrary>
#include <QFileInfoList>
#include <QSettings>
#include <QProcessEnvironment>
#include <algorithm>

#include "vapoursynthpluginsmanager.h"

//==============================================================================

const char CORE_PLUGINS_FILEPATH[] = "core";

//==============================================================================

void VS_CC fakeConfigPlugin(const char * a_identifier,
	const char * a_defaultNamespace, const char * a_name, int a_apiVersion,
	int a_readonly, VSPlugin * a_pPlugin)
{
	(void)a_apiVersion;
	(void)a_readonly;

	// Dirty hack encouraged by Myrsloik himself.
	VapourSynthPluginsManager * pManager =
		(VapourSynthPluginsManager *)a_pPlugin;

	QString id(a_identifier);
	for(const VSData::Plugin & plugin : pManager->m_pluginsList)
	{
		if(plugin.id == id)
		{
			pManager->m_pluginAlreadyLoaded = true;
			return;
		}
	}

	pManager->m_pluginsList.emplace_back();
	pManager->m_pluginsList.back().filepath = pManager->m_currentPluginPath;
	pManager->m_pluginsList.back().id = id;
	pManager->m_pluginsList.back().pluginNamespace =
		QString(a_defaultNamespace);
	pManager->m_pluginsList.back().name = QString(a_name);
}

// END OF void VS_CC fakeConfigPlugin(const char * a_identifier,
//		const char * a_defaultNamespace, const char * a_name, int a_apiVersion,
//		int a_readonly, VSPlugin * a_pPlugin)
//==============================================================================

void VS_CC fakeRegisterFunction(const char * a_name, const char * a_args,
	VSPublicFunction a_argsFunc, void * a_pFunctionData, VSPlugin * a_pPlugin)
{
    (void)a_argsFunc;
    (void)a_pFunctionData;

    // Dirty hack encouraged by Myrsloik himself.
	VapourSynthPluginsManager * pManager =
		(VapourSynthPluginsManager *)a_pPlugin;

	if(pManager->m_pluginAlreadyLoaded)
		return;

	VSData::Function function =
		VapourSynthPluginsManager::parseFunctionSignature(a_name, a_args);
	pManager->m_pluginsList.back().functions.push_back(function);
}

// END OF void VS_CC fakeRegisterFunction(const char * a_name,
//		const char * a_args, VSPublicFunction a_argsFunc,
//		void * a_pFunctionData, VSPlugin * a_pPlugin)
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
	bool loaded = vsLibrary.load();

#ifdef Q_OS_WIN
	if(!loaded)
	{
		QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE",
			QSettings::NativeFormat);
		libraryFullPath =
			settings.value("VapourSynth/VapourSynthDLL").toString();
		if(libraryFullPath.isEmpty())
		{
			libraryFullPath = settings.value(
				"Wow6432Node/VapourSynth/VapourSynthDLL").toString();
		}

		if(!libraryFullPath.isEmpty())
		{
			vsLibrary.setFileName(libraryFullPath);
			loaded = vsLibrary.load();
		}
	}

	if(!loaded)
	{
		QProcessEnvironment environment =
			QProcessEnvironment::systemEnvironment();
		QString basePath;

#ifdef Q_OS_WIN64
		basePath = environment.value("ProgramFiles(x86)");
		libraryFullPath = basePath + "\\VapourSynth\\core64\\vapoursynth.dll";
#else
		basePath = environment.value("ProgramFiles");
		libraryFullPath = basePath + "\\VapourSynth\\core32\\vapoursynth.dll";
#endif // Q_OS_WIN64

		vsLibrary.setFileName(libraryFullPath);
		loaded = vsLibrary.load();
	}
#endif // Q_OS_WIN

	if(!loaded)
	{
		QStringList librarySearchPaths =
			m_pSettingsManager->getVapourSynthLibraryPaths();
		for(const QString & path : librarySearchPaths)
		{
			libraryFullPath = path + QString("/") + libraryName;
			vsLibrary.setFileName(libraryFullPath);
			loaded = vsLibrary.load();
			if(loaded)
				break;
		}
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

	const VSAPI * cpVSAPI = getVapourSynthAPI(VAPOURSYNTH_API_VERSION);
	if(!cpVSAPI)
	{
		emit signalWriteLogMessage(mtCritical, "VapourSynth plugins manager: "
			"Failed to get VapourSynth API!");
		vsLibrary.unload();
		return;
	}

	const int CORE_THREADS_NUMBER = 1;
	VSCore * pCore = cpVSAPI->createCore(CORE_THREADS_NUMBER);
	if(!pCore)
	{
		emit signalWriteLogMessage(mtCritical, "VapourSynth plugins manager: "
			"Failed to create VapourSynth core!");
		vsLibrary.unload();
		return;
	}

	VSMap * pPluginsMap = cpVSAPI->getPlugins(pCore);
	if(!pPluginsMap)
	{
		emit signalWriteLogMessage(mtCritical, "VapourSynth plugins manager: "
			"Failed to get core plugins!");
		cpVSAPI->freeCore(pCore);
		vsLibrary.unload();
		return;
	}

	int pluginsCount = cpVSAPI->propNumKeys(pPluginsMap);
	for(int i = 0; i < pluginsCount; ++i)
	{
		QString filepath = CORE_PLUGINS_FILEPATH;

        const char * pluginKey = cpVSAPI->propGetKey(pPluginsMap, i);
        if(!pluginKey)
		{
			emit signalWriteLogMessage(mtCritical, QString("VapourSynth "
				"plugins manager: Failed to get ID for the plugin number %1.")
				.arg(i));
			continue;
		}

        const char * pluginStr = cpVSAPI->propGetData(pPluginsMap,
			pluginKey, 0, nullptr);
        if(!pluginStr)
        {
            emit signalWriteLogMessage(mtCritical, QString("VapourSynth "
                "plugins manager: Failed to get ID for the plugin number %1.")
                .arg(i));
            continue;
        }

        QString id(QString::fromUtf8(pluginStr).split(';')[1]);
        QByteArray pluginID(id.toUtf8());

		for(const VSData::Plugin & existingPluginData : m_pluginsList)
		{
			if(existingPluginData.id == id)
				continue;
		}

        VSPlugin * pPlugin = cpVSAPI->getPluginById(pluginID, pCore);
		if(!pPlugin)
		{
			emit signalWriteLogMessage(mtCritical, QString("VapourSynth "
				"plugins manager: Failed to get pointer to the plugin %1!")
				.arg(id));
			continue;
		}

		int error = 0;
        const char * pluginInfo = cpVSAPI->propGetData(pPluginsMap, pluginKey,
			0, &error);
		if(error || !pluginInfo)
		{
			const char * errorString = cpVSAPI->getError(pPluginsMap);
			QString logString = QString("VapourSynth plugins manager: "
				"failed to get  information for the plugin ID %1.\nError: %2")
				.arg(id).arg(errorString);
			emit signalWriteLogMessage(mtCritical, logString);
			continue;
		}
		QString pluginInfoString(pluginInfo);

		m_pluginsList.emplace_back();
		VSData::Plugin & pluginData = m_pluginsList.back();
		pluginData.filepath = filepath;
		pluginData.id = id;
		QStringList parsedPluginInfo = pluginInfoString.split(';');
		pluginData.pluginNamespace = parsedPluginInfo[0];
		pluginData.name = parsedPluginInfo[2];

		// Get functions from the plugin.
		VSMap * pFunctionsMap = cpVSAPI->getFunctions(pPlugin);

		int functionsCount = cpVSAPI->propNumKeys(pFunctionsMap);
		for(int j = 0; j < functionsCount; ++j)
		{
			const char * functionName = cpVSAPI->propGetKey(pFunctionsMap, j);
			if(!functionName)
			{
				QString logString = QString("VapourSynth plugins manager: "
					"failed to get information for the function number %1 "
					"in plugin %2.").arg(j).arg(id);
				emit signalWriteLogMessage(mtCritical, logString);
				continue;
			}
			error = 0;
			const char * functionInfo = cpVSAPI->propGetData(pFunctionsMap,
				functionName, 0, &error);
			if(error || !functionInfo)
			{
				const char * errorString = cpVSAPI->getError(pPluginsMap);
				QString logString = QString("VapourSynth plugins manager: "
					"failed to get information for the function %1.%2()\n"
					"Error: %3").arg(pluginData.pluginNamespace)
					.arg(functionName).arg(errorString);
				emit signalWriteLogMessage(mtCritical, logString);
				continue;
			}
			VSData::Function function = parseFunctionSignature(functionName,
				functionInfo);
			pluginData.functions.push_back(function);
		}

		cpVSAPI->freeMap(pFunctionsMap);
	}

	cpVSAPI->freeMap(pPluginsMap);
	cpVSAPI->freeCore(pCore);
	vsLibrary.unload();
}

// END OF void VapourSynthPluginsManager::getCorePlugins()
//==============================================================================

void VapourSynthPluginsManager::pollPaths(const QStringList & a_pluginsPaths)
{
	for(const QString & dirPath : a_pluginsPaths)
	{
		QFileInfoList fileInfoList = QDir(dirPath).entryInfoList();
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
			// Dirty hack encouraged by Myrsloik himself.
			initVSPlugin(fakeConfigPlugin, fakeRegisterFunction,
				(VSPlugin *) this);
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
	QStringList argumentsList = a_arguments.split(';', QString::SkipEmptyParts);
	if(argumentsList.size() == 0)
		return function;

	// This is true for arguments lists returned by VSAPI.
	if(argumentsList[0] == a_name)
		argumentsList.removeFirst();

	for(const QString& argumentString : argumentsList)
	{
		QStringList argumentParts = argumentString.split(':');
		size_t partsNumber = argumentParts.size();
		if(partsNumber < 2)
			continue;

		function.arguments.emplace_back();
		VSData::FunctionArgument & argument = function.arguments.back();
		argument.name = argumentParts[0];
		argument.type = argumentParts[1];

		for(size_t i = 2; i < partsNumber; ++i)
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
