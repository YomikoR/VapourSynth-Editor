#ifndef VS_SCRIPT_LIBRARY_H_INCLUDED
#define VS_SCRIPT_LIBRARY_H_INCLUDED

#include "../version_info.h"
#include <vapoursynth/VSScript4.h>

#include <QObject>
#include <QLibrary>
#include <map>

class SettingsManagerCore;

//==============================================================================

typedef const VSSCRIPTAPI * (VS_CC * FNP_getVSSAPI)(int);

//==============================================================================

class VSScriptLibrary : public QObject
{
	Q_OBJECT

public:

	VSScriptLibrary(SettingsManagerCore * a_pSettingsManager,
		QObject * a_pParent = nullptr);

	virtual ~VSScriptLibrary();

	bool initialize();

	bool finalize();

	bool isInitialized() const;

	const VSAPI * getVSAPI();

	VSScript * createScript(VSCore * a_pCore = nullptr);

	int evaluateScript(VSScript * a_pScript, const char * a_scriptText,
		const char * a_scriptFilename);

	const char * getError(VSScript * a_pScript);

	VSCore * createCore(int a_flag = 0);

	VSNode * getOutput(VSScript * a_pScript, int a_index);

	bool freeScript(VSScript * a_pScript);

	bool clearCoreCaches(VSCore * a_pCore);

	QString VSAPIInfo();
	QString VSSAPIInfo();

signals:

	void signalWriteLogMessage(int a_messageType, const QString & a_message);

private:

	bool initLibrary();

	void freeLibrary();

	void handleVSMessage(int a_messageType, const QString & a_message);

	friend void VS_CC vsMessageHandler(int a_msgType,
		const char * a_message, void * a_pUserData);

	SettingsManagerCore * m_pSettingsManager;

	QLibrary m_vsScriptLibrary;

	FNP_getVSSAPI vssGetAPI;

	bool m_initialized;

	const VSSCRIPTAPI * m_cpVSSAPI;

	const VSAPI * m_cpVSAPI;

	std::map<VSCore *, VSLogHandle *> m_VSCoreLogHandles;

	int m_VSAPIMajor;
	int m_VSAPIMinor;
	int m_VSSAPIMajor;
	int m_VSSAPIMinor;
};

//==============================================================================

#endif // VS_SCRIPT_LIBRARY_H_INCLUDED
