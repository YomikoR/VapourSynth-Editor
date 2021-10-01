#include "vs_script_library.h"

#include "../settings/settings_manager_core.h"
#include "../helpers.h"

#include <QSettings>
#include <QProcessEnvironment>

//==============================================================================

void VS_CC vsMessageHandler(int a_msgType, const char * a_message,
	void * a_pUserData)
{
	VSScriptLibrary * pVSScriptLibrary =
		static_cast<VSScriptLibrary *>(a_pUserData);
	pVSScriptLibrary->handleVSMessage(a_msgType, a_message);
}

// END OF void VS_CC vsMessageHandler(int a_msgType, const char * a_message,
//		void * a_pUserData)
//==============================================================================

VSScriptLibrary::VSScriptLibrary(SettingsManagerCore * a_pSettingsManager,
	QObject * a_pParent):
	QObject(a_pParent)
	, m_pSettingsManager(a_pSettingsManager)
	, m_vsScriptLibrary(this)
	, m_vsScriptInitialized(false)
	, m_initialized(false)
	, m_cpVSAPI(nullptr)
{
	Q_ASSERT(m_pSettingsManager);
}

// END OF VSScriptLibrary::VSScriptLibrary(
//		SettingsManagerCore * a_pSettingsManager, QObject * a_pParent)
//==============================================================================

VSScriptLibrary::~VSScriptLibrary()
{
	finalize();
}

// END OF VSScriptLibrary::~VSScriptLibrary()
//==============================================================================

bool VSScriptLibrary::initialize()
{
	if(m_initialized)
		return true;

	bool libraryInitialized = initLibrary();
	if(!libraryInitialized)
		return false;

	int opresult = vssInit();
	if(!opresult)
	{
		QString errorString = tr("Failed to initialize "
			"VapourSynth environment!");
		emit signalWriteLogMessage(mtCritical, errorString);
		finalize();
		return false;
	}
	m_vsScriptInitialized = true;

	m_cpVSAPI = vssGetVSApi();
	if(!m_cpVSAPI)
	{
		QString errorString = tr("Failed to get VapourSynth API!");
		emit signalWriteLogMessage(mtCritical, errorString);
		finalize();
		return false;
	}

	m_cpVSAPI->setMessageHandler(::vsMessageHandler,
		static_cast<void *>(this));

	m_initialized = true;

	return true;
}

// END OF bool VSScriptLibrary::initialize()
//==============================================================================

bool VSScriptLibrary::finalize()
{
	m_cpVSAPI = nullptr;

	if(m_vsScriptInitialized)
	{
		vssFinalize();
		m_vsScriptInitialized = false;
	}

	freeLibrary();
	m_initialized = false;

	return true;
}

// END OF bool VSScriptLibrary::finalize()
//==============================================================================

bool VSScriptLibrary::isInitialized() const
{
	return m_initialized;
}

// END OF bool VSScriptLibrary::isInitialized() const
//==============================================================================

const VSAPI * VSScriptLibrary::getVSAPI()
{
	if(!initialize())
		return nullptr;

	return m_cpVSAPI;
}

// END OF const VSAPI * VSScriptLibrary::getVSAPI()
//==============================================================================

int VSScriptLibrary::evaluateScript(VSScript ** a_ppScript,
	const char * a_scriptText, const char * a_scriptFilename, int a_flags)
{
	if(!initialize())
		return 1;

	return vssEvaluateScript(a_ppScript, a_scriptText,
		a_scriptFilename, a_flags);
}

// END OF int VSScriptLibrary::evaluateScript(VSScript ** a_ppScript,
//		const char * a_scriptText, const char * a_scriptFilename, int a_flags)
//==============================================================================

const char * VSScriptLibrary::getError(VSScript * a_pScript)
{
	if(!initialize())
		return nullptr;

	return vssGetError(a_pScript);
}

// END OF const char * VSScriptLibrary::getError(VSScript * a_pScript)
//==============================================================================

VSCore * VSScriptLibrary::getCore(VSScript * a_pScript)
{
	if(!initialize())
		return nullptr;

	return vssGetCore(a_pScript);
}

// END OF VSCore * VSScriptLibrary::getCore(VSScript * a_pScript)
//==============================================================================

VSNodeRef * VSScriptLibrary::getOutput(VSScript * a_pScript, int a_index)
{
	if(!initialize())
		return nullptr;

	return vssGetOutput(a_pScript, a_index);
}

// END OF VSNodeRef * VSScriptLibrary::getOutput(VSScript * a_pScript,
//		int a_index)
//==============================================================================

bool VSScriptLibrary::freeScript(VSScript * a_pScript)
{
	if(!initialize())
		return false;

	vssFreeScript(a_pScript);

	return true;
}

// END OF bool VSScriptLibrary::freeScript(VSScript * a_pScript)
//==============================================================================

bool VSScriptLibrary::initLibrary()
{
	if(m_vsScriptLibrary.isLoaded())
	{
		Q_ASSERT(vssInit);
		Q_ASSERT(vssGetVSApi);
		Q_ASSERT(vssEvaluateScript);
		Q_ASSERT(vssGetError);
		Q_ASSERT(vssGetCore);
		Q_ASSERT(vssGetOutput);
		Q_ASSERT(vssFreeScript);
		Q_ASSERT(vssFinalize);
		return true;
	}

	QString libraryName(
#ifdef Q_OS_WIN
		"vsscript"
#else
		"vapoursynth-script"
#endif // Q_OS_WIN
	);

	QString libraryFullPath;
	m_vsScriptLibrary.setFileName(libraryName);
	m_vsScriptLibrary.setLoadHints(QLibrary::ExportExternalSymbolsHint);
	bool loaded = m_vsScriptLibrary.load();

#ifdef Q_OS_WIN
	if(!loaded)
	{
		QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE",
			QSettings::NativeFormat);
		libraryFullPath =
			settings.value("VapourSynth/VSScriptDLL").toString();
		if(libraryFullPath.isEmpty())
		{
			libraryFullPath = settings.value(
				"Wow6432Node/VapourSynth/VSScriptDLL").toString();
		}

		if(!libraryFullPath.isEmpty())
		{
			m_vsScriptLibrary.setFileName(libraryFullPath);
			loaded = m_vsScriptLibrary.load();
		}
	}

	if(!loaded)
	{
		QProcessEnvironment environment =
			QProcessEnvironment::systemEnvironment();
		QString basePath;

#ifdef Q_OS_WIN64
		basePath = environment.value("ProgramFiles(x86)");
		libraryFullPath = basePath + "\\VapourSynth\\core64\\vsscript.dll";
#else
		basePath = environment.value("ProgramFiles");
		libraryFullPath = basePath + "\\VapourSynth\\core32\\vsscript.dll";
#endif // Q_OS_WIN64

		m_vsScriptLibrary.setFileName(libraryFullPath);
		loaded = m_vsScriptLibrary.load();
	}
#endif // Q_OS_WIN

	if(!loaded)
	{
		QStringList librarySearchPaths =
			m_pSettingsManager->getVapourSynthLibraryPaths();
		for(const QString & path : librarySearchPaths)
		{
			libraryFullPath = vsedit::resolvePathFromApplication(path) +
				QString("/") + libraryName;
			m_vsScriptLibrary.setFileName(libraryFullPath);
			loaded = m_vsScriptLibrary.load();
			if(loaded)
				break;
		}
	}

	if(!loaded)
	{
		emit signalWriteLogMessage(mtCritical,
			"Failed to load vapoursynth script library!\n"
			"Please set up the library search paths in settings.");
		return false;
	}

	struct Entry
	{
		QFunctionPointer * ppFunction;
		const char * name;
		const char * fallbackName;
	};

	Entry vssEntries[] =
	{
		  {(QFunctionPointer *)&vssInit, "vsscript_init",
			"_vsscript_init@0"}
		, {(QFunctionPointer *)&vssGetVSApi, "vsscript_getVSApi",
			"_vsscript_getVSApi@0"}
		, {(QFunctionPointer *)&vssEvaluateScript, "vsscript_evaluateScript",
			"_vsscript_evaluateScript@16"}
		, {(QFunctionPointer *)&vssGetError, "vsscript_getError",
			"_vsscript_getError@4"}
		, {(QFunctionPointer *)&vssGetCore, "vsscript_getCore",
			"_vsscript_getCore@4"}
		, {(QFunctionPointer *)&vssGetOutput, "vsscript_getOutput",
			"_vsscript_getOutput@8"}
		, {(QFunctionPointer *)&vssFreeScript, "vsscript_freeScript",
			"_vsscript_freeScript@4"}
		, {(QFunctionPointer *)&vssFinalize, "vsscript_finalize",
			"_vsscript_finalize@0"}
	};

	for(Entry & entry : vssEntries)
	{
		Q_ASSERT(entry.ppFunction);
		*entry.ppFunction = m_vsScriptLibrary.resolve(entry.name);
		if(!*entry.ppFunction)
		{ // Win32 fallback
			*entry.ppFunction = m_vsScriptLibrary.resolve(entry.fallbackName);
		}
		if(!*entry.ppFunction)
		{
			QString errorString = tr("Failed to get entry %1() "
				"in vapoursynth script library!").arg(entry.name);
			emit signalWriteLogMessage(mtCritical, errorString);
			freeLibrary();
			return false;
		}
	}

	return true;
}

// END OF bool VSScriptLibrary::initLibrary()
//==============================================================================

void VSScriptLibrary::freeLibrary()
{
	vssInit = nullptr;
	vssGetVSApi = nullptr;
	vssEvaluateScript = nullptr;
	vssGetError = nullptr;
	vssGetCore = nullptr;
	vssGetOutput = nullptr;
	vssFreeScript = nullptr;
	vssFinalize = nullptr;

	if(m_vsScriptLibrary.isLoaded())
		m_vsScriptLibrary.unload();
}

// END OF void VSScriptLibrary::freeLibrary()
//==============================================================================

void VSScriptLibrary::handleVSMessage(int a_messageType,
	const QString & a_message)
{
	emit signalWriteLogMessage(a_messageType, a_message);
}

// END OF void VSScriptLibrary::handleVSMessage(int a_messageType,
//		const QString & a_message)
//==============================================================================
