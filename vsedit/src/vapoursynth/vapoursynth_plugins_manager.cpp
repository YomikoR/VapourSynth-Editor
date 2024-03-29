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
	SettingsManager * a_pSettingsManager, const VSAPI * a_cpVSAPI, QObject * a_pParent):
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
	slotRefill(a_cpVSAPI);
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

void VapourSynthPluginsManager::getCorePlugins(const VSAPI * a_cpVSAPI)
{
	if(!a_cpVSAPI)
		return;

	VSCore * pCore = a_cpVSAPI->createCore(0);
	if(!pCore)
	{
		emit signalWriteLogMessage(mtCritical, "VapourSynth plugins manager: "
			"Failed to create VapourSynth core!");
		return;
	}

	VSPlugin * pPlugin = a_cpVSAPI->getNextPlugin(nullptr, pCore);
	while(pPlugin)
	{
		m_pluginsList.emplace_back();
		VSData::Plugin & pluginData = m_pluginsList.back();
		pluginData.filepath = a_cpVSAPI->getPluginPath(pPlugin);
		pluginData.id = a_cpVSAPI->getPluginID(pPlugin);
		pluginData.pluginNamespace = a_cpVSAPI->getPluginNamespace(pPlugin);
		pluginData.name = a_cpVSAPI->getPluginName(pPlugin);

		VSPluginFunction * pPluginFunction =
			a_cpVSAPI->getNextPluginFunction(nullptr, pPlugin);
		while(pPluginFunction)
		{
			const char * pluginFunctionName =
				a_cpVSAPI->getPluginFunctionName(pPluginFunction);
			const char * pluginFunctionArgs =
				a_cpVSAPI->getPluginFunctionArguments(pPluginFunction);

			VSData::Function function = parseFunctionSignature(pluginFunctionName,
				pluginFunctionArgs);
			pluginData.functions.push_back(function);

			pPluginFunction = a_cpVSAPI->getNextPluginFunction(pPluginFunction, pPlugin);
		}
		pPlugin = a_cpVSAPI->getNextPlugin(pPlugin, pCore);
	}

	a_cpVSAPI->freeCore(pCore);
}

// END OF void VapourSynthPluginsManager::getCorePlugins(const VSAPI * a_cpVSAPI)
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

void VapourSynthPluginsManager::slotRefill(const VSAPI * a_cpVSAPI)
{
	slotClear();
	getCorePlugins(a_cpVSAPI);
	slotSort();
}

// END OF void VapourSynthPluginsManager::slotRefill(const VSAPI * a_cpVSAPI)
//==============================================================================
