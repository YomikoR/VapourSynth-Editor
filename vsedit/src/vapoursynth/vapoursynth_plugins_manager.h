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
		const VSAPI * a_cpVSAPI, QObject * a_pParent = nullptr);

	virtual ~VapourSynthPluginsManager();

	void getCorePlugins(const VSAPI * a_cpVSAPI = nullptr);

	QStringList functions() const;

	VSPluginsList pluginsList() const;

	static VSData::Function parseFunctionSignature(const QString & a_name,
		const QString & a_arguments);

public slots:

	void slotClear();

	void slotSort();

	void slotRefill(const VSAPI * a_cpVSAPI);

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
