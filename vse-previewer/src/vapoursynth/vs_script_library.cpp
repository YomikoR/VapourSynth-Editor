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
	QObject * a_pParent, QString a_libPath):
	QObject(a_pParent)
	, m_pSettingsManager(a_pSettingsManager)
	, m_vsScriptLibrary(this)
	, m_initialized(false)
	, m_cpVSSAPI(nullptr)
	, m_cpVSAPI(nullptr)
	, m_pArguments(nullptr)
	, m_pLogHandle(nullptr)
	, m_VSAPIMajor(VSE_VS_API_VER_MAJOR)
	, m_VSAPIMinor(VSE_VS_API_VER_MINOR)
	, m_VSSAPIMajor(VSE_VSS_API_VER_MAJOR)
	, m_VSSAPIMinor(VSE_VSS_API_VER_MINOR)
	, m_forcedLibrarySearchPath(a_libPath)
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

	for(; m_VSAPIMinor >= 0; --m_VSAPIMinor)
	{
		int apiVer = VS_MAKE_VERSION(m_VSAPIMajor, m_VSAPIMinor);
		m_cpVSAPI = m_cpVSSAPI->getVSAPI(apiVer);
		if(m_cpVSAPI)
			break;
	}

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
	if(m_pArguments && m_cpVSAPI)
	{
		m_cpVSAPI->freeMap(m_pArguments);
		m_pArguments = nullptr;
	}

	for(size_t i = 0; i < m_scripts.size(); ++i)
	{
		if(m_scripts[i])
			m_cpVSSAPI->freeScript(m_scripts[i]);
	}

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

void VSScriptLibrary::setArguments(const std::map<std::string, std::string> &a_args)
{
	if(!initialize())
		return;

	if(m_pArguments)
	{
		m_cpVSAPI->clearMap(m_pArguments);
		m_pArguments = nullptr;
	}

	if(a_args.size() > 0)
	{
		m_pArguments = m_cpVSAPI->createMap();
		for (const auto &p : a_args)
		{
			m_cpVSAPI->mapSetData(m_pArguments, p.first.c_str(),
				p.second.c_str(), (int)p.second.size(), dtUtf8, maAppend);
		}
	}
}

// END OF const VSAPI * VSScriptLibrary::getVSAPI()
//==============================================================================

VSScript * VSScriptLibrary::createScript()
{
	if(!initialize())
		return nullptr;

	VSCore * pCore = m_cpVSAPI->createCore(m_coreCreationFlag);

	VSCoreInfo info;
	m_cpVSAPI->getCoreInfo(pCore, &info);
	if(info.core < 58)
	{
		QString errorString = "VapourSynth version 58 or later is required.";
		emit signalWriteLogMessage(mtCritical, errorString);
		finalize();
		return nullptr;
	}

	if(!m_pLogHandle)
		m_pLogHandle = m_cpVSAPI->addLogHandler(vsMessageHandler, nullptr, this, pCore);

	VSScript * pScript = m_cpVSSAPI->createScript(pCore);
	if(pScript)
		m_cpVSSAPI->evalSetWorkingDir(pScript, 1);

	if(m_pArguments)
		m_cpVSSAPI->setVariables(pScript, m_pArguments);

	m_scripts.push_back(pScript);

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

	return pCore;
}

std::vector<int> VSScriptLibrary::getOutputIndices(VSScript * a_pScript) const
{
	if(!m_initialized || !a_pScript)
		return std::vector<int>();

	int size = m_cpVSSAPI->getAvailableOutputNodes(a_pScript, 0, nullptr);
	if(size <= 0)
		return std::vector<int>();
	
	std::vector<int> idx(size);
	m_cpVSSAPI->getAvailableOutputNodes(a_pScript, size, idx.data());
	return idx;
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

bool VSScriptLibrary::clearCoreCaches([[maybe_unused]] VSScript *a_pScript)
{
#if(VAPOURSYNTH_API_MAJOR == 4) && (VAPOURSYNTH_API_MINOR >= 1)
	if(m_VSAPIMajor == 4 && m_VSAPIMinor >= 1)
	{
		if(!m_initialized)
			return false;

		VSCore * pCore = m_cpVSSAPI->getCore(a_pScript);
		m_cpVSAPI->clearCoreCaches(pCore);
		return true;
	}
#endif
    return false;
}

QString VSScriptLibrary::VSAPIInfo()
{
	if(!m_initialized)
		return QString();
    return QString("R%1.%2").arg(m_VSAPIMajor).arg(m_VSAPIMinor);
}

QString VSScriptLibrary::VSSAPIInfo()
{
	if(!m_initialized)
		return QString();
    return QString("R%1.%2").arg(m_VSSAPIMajor).arg(m_VSSAPIMinor);
}

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

	QString libraryDir;
	QString libraryFullPath;
	bool loaded = false;
	m_vsScriptLibrary.setLoadHints(QLibrary::ExportExternalSymbolsHint);

	QString path = QString::fromLocal8Bit(qgetenv("PATH"));
	QString path_backup = path;

	auto set_path = [&]()
	{
#ifdef Q_OS_WIN
		path = libraryDir + ";" + path;
		qputenv("PATH", path.toLocal8Bit());
#endif
	};

	auto reset_path = [&]()
	{
#ifdef Q_OS_WIN
		path = path_backup;
		qputenv("PATH", path.toLocal8Bit());
#endif
	};

	auto load_from_registry = [&]()
	{
#ifdef Q_OS_WIN
		QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE",
			QSettings::NativeFormat);
		libraryFullPath =
			settings.value("VapourSynth/VSScriptDLL").toString();

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
		for(const QString & libSearchPath : librarySearchPaths)
		{
			m_vsScriptLibrary.unload();
			libraryDir = vsedit::resolvePathFromApplication(libSearchPath);
			libraryFullPath = libraryDir + QString("/") + libraryName;
			m_vsScriptLibrary.setFileName(libraryFullPath);
			set_path();
			loaded = m_vsScriptLibrary.load();
			reset_path();
			if(loaded)
				break;
		}
	};

	auto load_forced = [&]()
	{
		m_vsScriptLibrary.unload();
		libraryDir = vsedit::resolvePathFromApplication(
			m_forcedLibrarySearchPath);
		libraryFullPath = libraryDir + QString("/") + libraryName;
		m_vsScriptLibrary.setFileName(libraryFullPath);
		set_path();
		loaded = m_vsScriptLibrary.load();
		reset_path();
	};

	if(!m_forcedLibrarySearchPath.isEmpty())
	{
		load_forced();
	}
	else if(m_pSettingsManager->getPreferVSLibrariesFromList())
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

	m_cpVSSAPI = vssGetAPI(VS_MAKE_VERSION(m_VSSAPIMajor, m_VSSAPIMinor));
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
