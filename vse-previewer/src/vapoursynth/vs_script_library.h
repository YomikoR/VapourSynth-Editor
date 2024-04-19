#ifndef VS_SCRIPT_LIBRARY_H_INCLUDED
#define VS_SCRIPT_LIBRARY_H_INCLUDED

#include <vapoursynth/VSScript4.h>

#include <QObject>
#include <QLibrary>

#include <vector>

class SettingsManagerCore;

//==============================================================================

typedef const VSSCRIPTAPI * (VS_CC * FNP_getVSSAPI)(int);

//==============================================================================

class VSScriptLibrary : public QObject
{
	Q_OBJECT

public:

	VSScriptLibrary(SettingsManagerCore * a_pSettingsManager,
		QObject * a_pParent = nullptr, QString a_libPath = QString());

	virtual ~VSScriptLibrary();

	bool initialize();

	bool finalize();

	bool isInitialized() const;

	const VSAPI * getVSAPI();

	void setArguments(const std::map<std::string, std::string> &a_args);

	VSScript * createScript();

	int evaluateScript(VSScript * a_pScript, const char * a_scriptText,
		const char * a_scriptFilename);

	const char * getError(VSScript * a_pScript);

	VSCore * getCore(VSScript * a_pScript);

	VSNode * getOutput(VSScript * a_pScript, int a_index);

	//bool freeScript(VSScript * a_pScript);

	bool clearCoreCaches(VSScript * a_pScript);

	int getDefaultOutputIndex() const { return m_defaultOutputIndex; }
	void setDefaultOutputIndex(int a_index) { m_defaultOutputIndex = a_index; }

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

	VSMap * m_pArguments;

	VSLogHandle * m_pLogHandle;

	int m_VSAPIMajor;
	int m_VSAPIMinor;
	int m_VSSAPIMajor;
	int m_VSSAPIMinor;

	QString m_forcedLibrarySearchPath;

	int m_defaultOutputIndex = 0;

	std::vector<VSScript *> m_scripts;
};

//==============================================================================

#endif // VS_SCRIPT_LIBRARY_H_INCLUDED
