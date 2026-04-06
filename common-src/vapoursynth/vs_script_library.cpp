#include "vs_script_library.h"

#include "../settings/settings_manager_core.h"
#include "../helpers.h"

#include <QSettings>
#include <QProcess>
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
	, m_VSAPIMajor(VSE_VS_API_VER_MAJOR)
	, m_VSAPIMinor(VSE_VS_API_VER_MINOR)
	, m_VSSAPIMajor(VSE_VSS_API_VER_MAJOR)
	, m_VSSAPIMinor(VSE_VSS_API_VER_MINOR)
{
	Q_ASSERT(m_pSettingsManager);
}

// END OF VSScriptLibrary::VSScriptLibrary(
//		SettingsManagerCore * a_pSettingsManager, QObject * a_pParent)
//==============================================================================

VSScriptLibrary::~VSScriptLibrary()
{
	m_VSCoreLogHandles.clear();
	finalize();
}

// END OF VSScriptLibrary::~VSScriptLibrary()
//==============================================================================

bool VSScriptLibrary::initialize()
{
	if(m_initialized)
		return true;

	bool libraryInitialized = initLibrary2();
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

VSScript * VSScriptLibrary::createScript(VSCore * a_pCore)
{
	if(!initialize())
		return nullptr;

	VSScript * pScript = m_cpVSSAPI->createScript(a_pCore);
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

VSCore *VSScriptLibrary::createCore(int a_flag)
{
	if(!initialize())
		return nullptr;

	VSCore * pCore = m_cpVSAPI->createCore(a_flag);
	if(!pCore)
	{
		QString errorString = tr("Failed to get VapourSynth Core!");
		emit signalWriteLogMessage(mtCritical, errorString);
		finalize();
		return nullptr;
	}

	if(m_VSCoreLogHandles.find(pCore) == m_VSCoreLogHandles.end())
		m_VSCoreLogHandles[pCore] = m_cpVSAPI->addLogHandler(
			vsMessageHandler, nullptr, this, pCore);

    return pCore;
}

std::vector<int> VSScriptLibrary::getOutputIndices(VSScript *a_pScript) const
{
#if(VSSCRIPT_API_MAJOR == 4) && (VSSCRIPT_API_MINOR >= 2)
	if(m_initialized && a_pScript && vssVersionCompare(4, 2) >= 0)
	{
		int size = m_cpVSSAPI->getAvailableOutputNodes(a_pScript, 0, nullptr);
		if(size <= 0)
			return std::vector<int>();
		std::vector<int> idx(size);
		m_cpVSSAPI->getAvailableOutputNodes(a_pScript, size, idx.data());
		return idx;
	}
	else
		return std::vector<int>();
#else
	return std::vector<int>();
#endif
}

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

bool VSScriptLibrary::clearCoreCaches(VSCore * a_pCore)
{
#if(VAPOURSYNTH_API_MAJOR == 4) && (VAPOURSYNTH_API_MINOR >= 1)
	if(vsVersionCompare(4, 1) >= 0)
	{
		if(!m_initialized)
			return false;

		m_cpVSAPI->clearCoreCaches(a_pCore);
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

// END OF bool VSScriptLibrary::freeScript(VSScript * a_pScript)
//==============================================================================

bool VSScriptLibrary::initLibrary2()
{
	if(m_vsScriptLibrary.isLoaded())
	{
		Q_ASSERT(vssGetAPI);
		return true;
	}

	QString libraryName = "vsscript";

#ifdef Q_OS_WIN
	QString libraryNameOld = "vsscript";
	QString libraryName2 = "vsscript.dll";
	bool libraryName2CS = false;
	int libraryName2Chop = 4;
#elif defined(Q_OS_MACOS)
	QString libraryNameOld = "vapoursynth-script";
	QString libraryName2 = "libvsscript.4.dylib";
	bool libraryName2CS = true;
	int libraryName2Chop = 8;
#else
	QString libraryNameOld = "vapoursynth-script";
	QString libraryName2 = "libvsscript.so.4";
	bool libraryName2CS = true;
	int libraryName2Chop = 5;
#endif

	QString libraryDir;
	QString libraryFullPath = QString();
	bool loaded = false;

	QString path = QString::fromLocal8Bit(qgetenv("PATH"));
	QString path_backup = path;

	QFunctionPointer * ppGetAPI = (QFunctionPointer *)&vssGetAPI;
	QFunctionPointer * ppLastError = (QFunctionPointer *)&vssGetAPILastError;

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

	auto load_vssapi = [&] ()
	{
		*ppGetAPI = m_vsScriptLibrary.resolve("getVSScriptAPI");
		if(!*ppGetAPI)
			return;
		for(; m_VSSAPIMinor >= 0; --m_VSSAPIMinor)
		{
			int apiVer = VS_MAKE_VERSION(m_VSSAPIMajor, m_VSSAPIMinor);
			m_cpVSSAPI = vssGetAPI(apiVer);
			if(m_cpVSSAPI)
				break;
		}
		if(m_cpVSSAPI)
			return;
		else
		{
			QString errMsg = QString("Library found in %1 but failed to get VSScript API!").arg(libPath);
			const char * errVSSMsg = nullptr;
			*ppLastError = m_vsScriptLibrary.resolve("getVSScriptAPILastError");
			if(*ppLastError)
				errVSSMsg = vssGetAPILastError();
			if(errVSSMsg)
				emit signalWriteLogMessage(mtWarning, QString("%1\n%2").arg(errMsg).arg(errVSSMsg));
			else
				emit signalWriteLogMessage(mtWarning, errMsg);
			return;
		}
	};

	auto load_from_list = [&] ()
	{
		QStringList librarySearchPaths =
			m_pSettingsManager->getVapourSynthLibraryPaths();
		for(const QString & libPath : librarySearchPaths)
		{
			m_vsScriptLibrary.unload();
			libraryDir = vsedit::resolvePathFromApplication(libPath);
			libraryFullPath = libraryDir + QString("/") + libraryName;
			m_vsScriptLibrary.setFileName(libraryFullPath);
			set_path();
			loaded = m_vsScriptLibrary.load();
			reset_path();
			if(loaded)
			{
				load_vssapi();
				loaded = m_cpVSSAPI == nullptr;
			}
		}
	};

	auto load_from_python = [&] ()
	{
		auto venv = qgetenv("VIRTUAL_ENV");
		if(!venv.isEmpty())
		{
			emit signalWriteLogMessage(mtInformation, QString(
				"You are in a Python virtual environment with path %1")
				.arg(QString::fromLocal8Bit(venv)));
		}
		QProcess vssProc;
#ifdef Q_OS_WIN
		vssProc.startCommand("python -c \"import vapoursynth;"
#else
		vssProc.startCommand("/usr/bin/env python3 -c \"import vapoursynth;"
#endif
			"print(vapoursynth.get_vsscript())\"");
		if(vssProc.waitForFinished(3000))
		{
			QString ret = QString::fromLocal8Bit(
				vssProc.readAllStandardOutput()).trimmed();
			if(ret.count('\n') == 0 && ret.endsWith(libraryName2,
				libraryName2CS ? Qt::CaseSensitive : Qt::CaseInsensitive))
			{
#ifdef Q_OS_WIN
				ret.chop(libraryName2Chop);
#endif
				libraryFullPath = ret;
			}
		}

		if(libraryFullPath.isEmpty())
			return;

		m_vsScriptLibrary.unload();
		m_vsScriptLibrary.setFileName(libraryFullPath);
		loaded = m_vsScriptLibrary.load();

		if(loaded)
		{
			load_vssapi();
			loaded = m_cpVSSAPI == nullptr;
		}
	};

	auto load_from_env = [&] ()
	{
		auto envPath = qgetenv("VSSCRIPT_PATH");
		if(!envPath.isEmpty())
			libraryFullPath = QString::fromLocal8Bit(envPath);

		if(libraryFullPath.isEmpty())
			return;

		m_vsScriptLibrary.unload();
		m_vsScriptLibrary.setFileName(libraryFullPath);
		loaded = m_vsScriptLibrary.load();
		if(loaded)
		{
			load_vssapi();
			loaded = m_cpVSSAPI == nullptr;
		}
	};

	auto load_from_registry = [&] ()
	{
#ifdef Q_OS_WIN
		QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE",
			QSettings::NativeFormat);
		libraryFullPath =
			settings.value("VapourSynth/VSScriptDLL").toString();

		if(libraryFullPath.isEmpty())
			return;

			m_vsScriptLibrary.unload();
		m_vsScriptLibrary.setFileName(libraryFullPath);
		loaded = m_vsScriptLibrary.load();
		if(loaded)
		{
			load_vssapi();
			loaded = m_cpVSSAPI == nullptr;
		}
#endif
	};

	auto load_from_path = [&]()
	{
		m_vsScriptLibrary.unload();
		m_vsScriptLibrary.setFileName(libraryNameOld);
		loaded = m_vsScriptLibrary.load();
		if(loaded)
		{
			load_vssapi();
			loaded = m_cpVSSAPI == nullptr;
		}
	};

	if(m_pSettingsManager->getPreferVSLibrariesFromList())
	{
		if(!loaded) load_from_list();
		if(!loaded) load_from_python();
		if(!loaded) load_from_env();
		if(!loaded) load_from_registry();
		if(!loaded) load_from_path();
	}
	else
	{
		if(!loaded) load_from_python();
		if(!loaded) load_from_env();
		if(!loaded) load_from_list();
		if(!loaded) load_from_registry();
		if(!loaded) load_from_path();
	}

	if(!m_cpVSSAPI)
	{
		QString errorStr = QString("Failed to get VSScript API!");
		emit signalWriteLogMessage(mtCritical, errorStr);
		freeLibrary();
		return false;
	}

	return true;
}

// END OF bool VSScriptLibrary::initLibrary2()
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
