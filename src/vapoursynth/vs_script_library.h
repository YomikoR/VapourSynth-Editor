#ifndef VS_SCRIPT_LIBRARY_H_INCLUDED
#define VS_SCRIPT_LIBRARY_H_INCLUDED

#include <QObject>
#include <QLibrary>
#include <vapoursynth/VSScript.h>

class SettingsManager;

//==============================================================================

typedef int (VS_CC *FNP_vssInit)(void);
typedef const VSAPI * (VS_CC *FNP_vssGetVSApi)(void);
typedef int (VS_CC *FNP_vssEvaluateScript)(VSScript ** a_ppScript,
	const char * a_scriptText, const char * a_scriptFilename, int a_flags);
typedef const char * (VS_CC *FNP_vssGetError)(VSScript * a_pScript);
typedef VSCore * (VS_CC *FNP_vssGetCore)(VSScript * a_pScript);
typedef VSNodeRef * (VS_CC *FNP_vssGetOutput)(VSScript * a_pScript,
	int a_index);
typedef void (VS_CC *FNP_vssFreeScript)(VSScript * a_pScript);
typedef int (VS_CC *FNP_vssFinalize)(void);

//==============================================================================

class VSScriptLibrary : public QObject
{
	Q_OBJECT

public:

	VSScriptLibrary(SettingsManager * a_pSettingsManager,
		QObject * a_pParent = nullptr);

	virtual ~VSScriptLibrary();

	bool initialize();

	bool finalize();

	bool isInitialized() const;

	const VSAPI * getVSAPI();

	int evaluateScript(VSScript ** a_ppScript, const char * a_scriptText,
		const char * a_scriptFilename, int a_flags);

	const char * getError(VSScript * a_pScript);

	VSCore * getCore(VSScript * a_pScript);

	VSNodeRef * getOutput(VSScript * a_pScript, int a_index);

	bool freeScript(VSScript * a_pScript);

signals:

	void signalWriteLogMessage(int a_messageType, const QString & a_message);

private:

	bool initLibrary();

	void freeLibrary();

	void handleVSMessage(int a_messageType, const QString & a_message);

	friend void VS_CC vsMessageHandler(int a_msgType,
		const char * a_message, void * a_pUserData);

	SettingsManager * m_pSettingsManager;

	QLibrary m_vsScriptLibrary;

	FNP_vssInit vssInit;
	FNP_vssGetVSApi vssGetVSApi;
	FNP_vssEvaluateScript vssEvaluateScript;
	FNP_vssGetError vssGetError;
	FNP_vssGetCore vssGetCore;
	FNP_vssGetOutput vssGetOutput;
	FNP_vssFreeScript vssFreeScript;
	FNP_vssFinalize vssFinalize;

	bool m_vsScriptInitialized;

	bool m_initialized;

	const VSAPI * m_cpVSAPI;
};

//==============================================================================

#endif // VS_SCRIPT_LIBRARY_H_INCLUDED
