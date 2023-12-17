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
		reinterpret_cast<VSScriptLibrary *>(a_pUserData);
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
	, m_initialized(false)
	, m_cpVSSAPI(nullptr)
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

	m_cpVSAPI = m_cpVSSAPI->getVSAPI(VAPOURSYNTH_API_VERSION);
	if(!m_cpVSAPI)
	{
		QString errorString = tr("Failed to get VapourSynth API!");
		emit signalWriteLogMessage(mtCritical, errorString);
		finalize();
		return false;
	}

	m_initialized = true;

	return true;
}

// END OF bool VSScriptLibrary::initialize()
//==============================================================================

bool VSScriptLibrary::finalize()
{
	m_cpVSAPI = nullptr;

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

VSScript * VSScriptLibrary::createScript()
{
	if(!initialize())
		return nullptr;

	VSScript * pScript = m_cpVSSAPI->createScript(nullptr);
	if(pScript)
		m_cpVSSAPI->evalSetWorkingDir(pScript, 1);

	return pScript;
}

// END OF VSScript * VSScriptLibrary::createScript()
//==============================================================================

int VSScriptLibrary::evaluateScript(VSScript * a_pScript,
	const char * a_scriptText, const char * a_scriptFilename)
{

	if(!initialize())
		return 1;

	return m_cpVSSAPI->evaluateBuffer(a_pScript, a_scriptText,
		a_scriptFilename);
}

// END OF int VSScriptLibrary::evaluateScript(VSScript * a_ppScript,
//		const char * a_scriptText, const char * a_scriptFilename)
//==============================================================================

const char * VSScriptLibrary::getError(VSScript * a_pScript)
{
	if(!initialize())
		return nullptr;

	return m_cpVSSAPI->getError(a_pScript);
}

// END OF const char * VSScriptLibrary::getError(VSScript * a_pScript)
//==============================================================================

VSCore * VSScriptLibrary::getCore(VSScript * a_pScript)
{
	if(!initialize())
		return nullptr;

	VSCore *pCore = m_cpVSSAPI->getCore(a_pScript);
	if (!pCore) {
		QString errorString = tr("Failed to get VapourSynth Core!");
		emit signalWriteLogMessage(mtCritical, errorString);
		finalize();
		return nullptr;
	}

	VSCoreInfo info;
	m_cpVSAPI->getCoreInfo(pCore, &info);
	if(info.core < 58)
	{
		QString errorString = "VapourSynth version 58 or later is required.";
		emit signalWriteLogMessage(mtCritical, errorString);
		finalize();
		return nullptr;
	}

	m_cpVSAPI->addLogHandler(vsMessageHandler, nullptr, this, pCore);
	return pCore;
}

// END OF VSCore * VSScriptLibrary::getCore(VSScript * a_pScript)
//==============================================================================

VSNode * VSScriptLibrary::getOutput(VSScript * a_pScript, int a_index)
{
	if(!initialize())
		return nullptr;

	return m_cpVSSAPI->getOutputNode(a_pScript, a_index);
}

// END OF VSNode * VSScriptLibrary::getOutput(VSScript * a_pScript,
//		int a_index)
//==============================================================================

bool VSScriptLibrary::freeScript(VSScript * a_pScript)
{
	if(!initialize())
		return false;

	m_cpVSSAPI->freeScript(a_pScript);

	return true;
}

// END OF bool VSScriptLibrary::freeScript(VSScript * a_pScript)
//==============================================================================

bool VSScriptLibrary::initLibrary()
{
	if(m_vsScriptLibrary.isLoaded())
	{
		Q_ASSERT(vssGetAPI);
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
	bool loaded = false;
	m_vsScriptLibrary.setLoadHints(QLibrary::ExportExternalSymbolsHint);

	auto load_from_registry = [&]()
	{
#ifdef Q_OS_WIN
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
			m_vsScriptLibrary.unload();
			m_vsScriptLibrary.setFileName(libraryFullPath);
			loaded = m_vsScriptLibrary.load();
		}
#endif // Q_OS_WIN
	};

	auto load_from_path = [&]()
	{
		m_vsScriptLibrary.unload();
		m_vsScriptLibrary.setFileName(libraryName);
		loaded = m_vsScriptLibrary.load();
	};

	auto load_from_list = [&]()
	{
		QStringList librarySearchPaths =
			m_pSettingsManager->getVapourSynthLibraryPaths();
		for(const QString & path : librarySearchPaths)
		{
			m_vsScriptLibrary.unload();
			libraryFullPath = vsedit::resolvePathFromApplication(path) +
				QString("/") + libraryName;
			m_vsScriptLibrary.setFileName(libraryFullPath);
			loaded = m_vsScriptLibrary.load();
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
		  {(QFunctionPointer *)&vssGetAPI, "getVSScriptAPI",
			"getVSScriptAPI"}
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

	m_cpVSSAPI = vssGetAPI(VS_MAKE_VERSION(4, 0));
	if(!m_cpVSSAPI)
	{
		QString errorStr = tr("Failed to get VSScript API!");
		emit signalWriteLogMessage(mtCritical, errorStr);
		freeLibrary();
		return false;
	}

	return true;
}

// END OF bool VSScriptLibrary::initLibrary()
//==============================================================================

void VSScriptLibrary::freeLibrary()
{
	vssGetAPI = nullptr;

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
