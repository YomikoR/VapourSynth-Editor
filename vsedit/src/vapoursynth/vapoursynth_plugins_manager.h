#ifndef VAPOURSYNTHPLUGINSMANAGER_H
#define VAPOURSYNTHPLUGINSMANAGER_H

#include "vs_plugin_data.h"

#include <vapoursynth/VapourSynth4.h>

#include <QObject>
#include <QStringList>

class SettingsManager;

class VapourSynthPluginsManager : public QObject
{
	Q_OBJECT

public:

	VapourSynthPluginsManager(SettingsManager * a_pSettingsManager,
		QObject * a_pParent = nullptr);

	virtual ~VapourSynthPluginsManager();

	void getCorePlugins();

	void pollPaths(const QStringList & a_pluginsPaths);

	QStringList functions() const;

	VSPluginsList pluginsList() const;

	static VSData::Function parseFunctionSignature(const QString & a_name,
		const QString & a_arguments);

	friend void VS_CC fakeConfigPlugin(const char * a_identifier,
		const char * a_defaultNamespace, const char * a_name,
		int a_apiVersion, int a_readonly, VSPlugin * a_pPlugin);

	friend void VS_CC fakeRegisterFunction(const char * a_name,
		const char * a_args, VSPublicFunction a_argsFunc,
		void * a_pFunctionData, VSPlugin * a_pPlugin);

public slots:

	void slotClear();

	void slotSort();

	void slotRefill();

signals:

	void signalWriteLogMessage(int a_messageType,
		const QString & a_message);

private:

	VSPluginsList m_pluginsList;

	QString m_currentPluginPath;

	bool m_pluginAlreadyLoaded;

	SettingsManager * m_pSettingsManager;

	VSPLUGINAPI m_VSPAPI;
};

#endif // VAPOURSYNTHPLUGINSMANAGER_H
