#include "vapoursynthscriptprocessor.h"

#include "../common/helpers.h"
#include "../settings/settingsmanager.h"

#include <cassert>
#include <vector>
#include <cmath>
#include <utility>
#include <memory>

#include <QImage>
#include <QSettings>
#include <QProcessEnvironment>

//==============================================================================

FrameTicket::FrameTicket(int a_frameNumber, int a_outputIndex,
	VSNodeRef * a_pNode):
	frameNumber(a_frameNumber)
	, outputIndex(a_outputIndex)
	, pNode(a_pNode)
	, discard(false)
{
	assert(pNode);
}

//==============================================================================

bool FrameTicket::operator<(const FrameTicket & a_other) const
{
	if(this == &a_other)
		return false;

	if(pNode < a_other.pNode)
		return true;

	if(pNode == a_other.pNode)
	{
		if(frameNumber < a_other.frameNumber)
			return true;
	}

	return false;
}

//==============================================================================

bool FrameTicket::operator==(const FrameTicket & a_other) const
{
	return ((pNode == a_other.pNode) && (frameNumber == a_other.frameNumber));
}

//==============================================================================

void VS_CC vsMessageHandler(int a_msgType, const char * a_message,
	void * a_pUserData)
{
	VapourSynthScriptProcessor * pScriptProcessor =
		static_cast<VapourSynthScriptProcessor *>(a_pUserData);
	pScriptProcessor->handleVSMessage(a_msgType, a_message);
}

// END OF void VS_CC vsMessageHandler(int a_msgType, const char * a_message,
//	void * a_pUserData)
//==============================================================================

void VS_CC frameReady(void * a_pUserData,
	const VSFrameRef * a_cpFrameRef, int a_frameNumber,
	VSNodeRef * a_pNodeRef, const char * a_errorMessage)
{
	VapourSynthScriptProcessor * pScriptProcessor =
		static_cast<VapourSynthScriptProcessor *>(a_pUserData);
	assert(pScriptProcessor);
	QString errorMessage(a_errorMessage);
	QMetaObject::invokeMethod(pScriptProcessor,
		"slotReceiveFrameAndProcessQueue",
		Qt::QueuedConnection,
		Q_ARG(const VSFrameRef *, a_cpFrameRef),
		Q_ARG(int, a_frameNumber),
		Q_ARG(VSNodeRef *, a_pNodeRef),
		Q_ARG(QString, errorMessage));
}

// END OF void VS_CC frameReady(void * a_pUserData,
//	const VSFrameRef * a_cpFrameRef, int a_frameNumber,
//	VSNodeRef * a_pNodeRef, const char * a_errorMessage)
//==============================================================================

VapourSynthScriptProcessor::VapourSynthScriptProcessor(
	SettingsManager * a_pSettingsManager, QObject * a_pParent):
	QObject(a_pParent)
	, m_pSettingsManager(a_pSettingsManager)
	, m_script()
	, m_scriptName()
	, m_error()
	, m_vsScriptInitialized(false)
	, m_initialized(false)
	, m_cpVSAPI(nullptr)
	, m_pVSScript(nullptr)
	, m_cpVideoInfo(nullptr)
	, m_cpCoreInfo(nullptr)
	, m_vsScriptLibrary(this)
{
	initLibrary();
}

// END OF VapourSynthScriptProcessor::VapourSynthScriptProcessor(
//		QObject * a_pParent)
//==============================================================================

VapourSynthScriptProcessor::~VapourSynthScriptProcessor()
{
	finalize();
	freeLibrary();
}

// END OF VapourSynthScriptProcessor::~VapourSynthScriptProcessor()
//==============================================================================

bool VapourSynthScriptProcessor::initialize(const QString& a_script,
	const QString& a_scriptName)
{
	if(m_initialized)
	{
		m_error = trUtf8("Script processor is already in use.");
		emit signalWriteLogMessage(mtCritical, m_error);
		return false;
	}

	if(!initLibrary())
		return false;

	int opresult = vssInit();
	if(!opresult)
	{
		m_error = trUtf8("Failed to initialize VapourSynth");
		emit signalWriteLogMessage(mtCritical, m_error);
		return false;
	}
	m_vsScriptInitialized = true;

	m_cpVSAPI = vssGetVSApi();
	if(!m_cpVSAPI)
	{
		m_error = trUtf8("Failed to get VapourSynth API!");
		emit signalWriteLogMessage(mtCritical, m_error);
		finalize();
		return false;
	}

	m_cpVSAPI->setMessageHandler(::vsMessageHandler,
		static_cast<void *>(this));

	opresult = vssEvaluateScript(&m_pVSScript,
		a_script.toUtf8().constData(), a_scriptName.toUtf8().constData(),
		efSetWorkingDir);

	if(opresult)
	{
		m_error = trUtf8("Failed to evaluate the script");
		const char * vsError = vssGetError(m_pVSScript);
		if(vsError)
			m_error += QString(":\n") + vsError;
		else
			m_error += '.';

		emit signalWriteLogMessage(mtCritical, m_error);
		finalize();
		return false;
	}

	VSCore * pCore = vssGetCore(m_pVSScript);
	m_cpCoreInfo = m_cpVSAPI->getCoreInfo(pCore);

	if(m_cpCoreInfo->core < 29)
	{
		m_error = trUtf8("VapourSynth R29+ required for preview.");
		emit signalWriteLogMessage(mtCritical, m_error);
		finalize();
		return false;
	}

	VSNodeRef * pOutputNode = vssGetOutput(m_pVSScript, 0);
	if(!pOutputNode)
	{
		m_error = trUtf8("Failed to get the script output node.");
		emit signalWriteLogMessage(mtCritical, m_error);
		finalize();
		return false;
	}

	m_cpVideoInfo = m_cpVSAPI->getVideoInfo(pOutputNode);

	m_cpVSAPI->freeNode(pOutputNode);

	m_error.clear();
	m_initialized = true;

	sendFrameQueueChangeSignal();

	return true;
}

// END OF bool VapourSynthScriptProcessor::initialize(const QString& a_script,
//		const QString& a_scriptName)
//==============================================================================

bool VapourSynthScriptProcessor::finalize()
{
	bool noFrameTicketsInProcess = flushFrameTicketsQueue();
	if(!noFrameTicketsInProcess)
	{
		m_error = trUtf8("Can not finalize the script processor while "
			"there are still frames in processing. Please wait for "
			"the processing to finish and repeat your action.");
		emit signalWriteLogMessage(mtCritical, m_error);
		return false;
	}

	m_cpVideoInfo = nullptr;
	m_cpCoreInfo = nullptr;

	if(m_pVSScript)
	{
		vssFreeScript(m_pVSScript);
		m_pVSScript = nullptr;
	}

	m_cpVSAPI = nullptr;

	if(m_vsScriptInitialized)
	{
		vssFinalize();
		m_vsScriptInitialized = false;
	}

	m_error.clear();
	m_initialized = false;

	return true;
}

// END OF bool VapourSynthScriptProcessor::finalize()
//==============================================================================

bool VapourSynthScriptProcessor::isInitialized() const
{
	return m_initialized;
}

// END OF bool VapourSynthScriptProcessor::isInitialized() const
//==============================================================================

QString VapourSynthScriptProcessor::error() const
{
	return m_error;
}

// END OF QString VapourSynthScriptProcessor::error() const
//==============================================================================

const VSVideoInfo * VapourSynthScriptProcessor::videoInfo(int a_outputIndex)
{
	if(!m_initialized)
		return nullptr;

	assert(m_cpVSAPI);
	assert(m_pVSScript);

	VSNodeRef * pNode = vssGetOutput(m_pVSScript, a_outputIndex);
	if(!pNode)
	{
		m_error = trUtf8("Couldn't resolve output node number %1.")
			.arg(a_outputIndex);
		emit signalWriteLogMessage(mtCritical, m_error);
		return nullptr;
	}

	const VSVideoInfo * cpVideoInfo = m_cpVSAPI->getVideoInfo(pNode);
	assert(cpVideoInfo);

	m_cpVSAPI->freeNode(pNode);

	return cpVideoInfo;
}

// END OF const VSVideoInfo * VapourSynthScriptProcessor::videoInfo() const
//==============================================================================

const VSAPI * VapourSynthScriptProcessor::api() const
{
	return m_cpVSAPI;
}

// END OF const VSVideoInfo * VapourSynthScriptProcessor::videoInfo() const
//==============================================================================

const VSFrameRef * VapourSynthScriptProcessor::requestFrame(int a_frameNumber,
	int a_outputIndex)
{
	if(!m_initialized)
		return nullptr;

	if(a_frameNumber < 0)
	{
		m_error = trUtf8("Requested frame number %1 is negative.")
			.arg(a_outputIndex);
		emit signalWriteLogMessage(mtCritical, m_error);
		return nullptr;
	}

	assert(m_cpVSAPI);

	VSNodeRef * pNode = vssGetOutput(m_pVSScript, a_outputIndex);
	if(!pNode)
	{
		m_error = trUtf8("Couldn't resolve output node number %1.")
			.arg(a_outputIndex);
		emit signalWriteLogMessage(mtCritical, m_error);
		return nullptr;
	}

	std::unique_ptr<VSNodeRef, std::function<void(VSNodeRef *)> >
		nodeDeleter(pNode, [&](VSNodeRef * a_pNode)
		{m_cpVSAPI->freeNode(a_pNode);});

	const VSVideoInfo * cpVideoInfo = m_cpVSAPI->getVideoInfo(pNode);
	assert(cpVideoInfo);

	if(a_frameNumber >= cpVideoInfo->numFrames)
	{
		m_error = trUtf8("Requested frame number %1 is outside the frame "
			"range.").arg(a_outputIndex);
		emit signalWriteLogMessage(mtCritical, m_error);
		return nullptr;
	}

	char getFrameErrorMessage[1024] = {0};
	const VSFrameRef * cpNewFrameRef = m_cpVSAPI->getFrame(a_frameNumber,
		pNode, getFrameErrorMessage, sizeof(getFrameErrorMessage) - 1);

	if(!cpNewFrameRef)
	{
		m_error = trUtf8("Error getting the frame number %1:\n%2")
			.arg(a_frameNumber).arg(QString::fromUtf8(getFrameErrorMessage));
		emit signalWriteLogMessage(mtCritical, m_error);
		return nullptr;
	}

	m_error.clear();
	return cpNewFrameRef;
}

// END OF bool VapourSynthScriptProcessor::requestFrame(int a_frameNumber,
//		int a_outputIndex)
//==============================================================================

bool VapourSynthScriptProcessor::requestFrameAsync(int a_frameNumber,
	int a_outputIndex)
{
	if(!m_initialized)
		return false;

	if(a_frameNumber < 0)
	{
		m_error = trUtf8("Requested frame number %1 is negative.")
			.arg(a_outputIndex);
		emit signalWriteLogMessage(mtCritical, m_error);
		return false;
	}

	assert(m_cpVSAPI);

	VSNodeRef * pNode = vssGetOutput(m_pVSScript, a_outputIndex);
	if(!pNode)
	{
		m_error = trUtf8("Couldn't resolve output node number %1.")
			.arg(a_outputIndex);
		emit signalWriteLogMessage(mtCritical, m_error);
		return false;
	}

	std::unique_ptr<VSNodeRef, std::function<void(VSNodeRef *)> >
		nodeDeleter(pNode, [&](VSNodeRef * a_pNode)
		{m_cpVSAPI->freeNode(a_pNode);});

	const VSVideoInfo * cpVideoInfo = m_cpVSAPI->getVideoInfo(pNode);
	assert(cpVideoInfo);

	if(a_frameNumber >= cpVideoInfo->numFrames)
	{
		m_error = trUtf8("Requested frame number %1 is outside the frame "
			"range.").arg(a_outputIndex);
		emit signalWriteLogMessage(mtCritical, m_error);
		return false;
	}

	FrameTicket newFrameTicket(a_frameNumber, a_outputIndex, pNode);

	if((int)m_frameTicketsInProcess.size() < m_cpCoreInfo->numThreads)
	{
		m_frameTicketsInProcess.push_back(newFrameTicket);
		m_cpVSAPI->getFrameAsync(a_frameNumber, pNode, frameReady, this);
	}
	else
	{
		m_frameTicketsQueue.push_back(newFrameTicket);
	}

	sendFrameQueueChangeSignal();
	return true;
}

// END OF void VapourSynthScriptProcessor::requestFrameAsync(int a_frameNumber,
//		int a_outputIndex)
//==============================================================================

bool VapourSynthScriptProcessor::flushFrameTicketsQueue()
{
	// Check the processing queue.
	for(FrameTicket & ticket : m_frameTicketsInProcess)
	{
		ticket.discard = true;
	}

	size_t queueSize = m_frameTicketsQueue.size();
	m_frameTicketsQueue.clear();
	if(queueSize)
		sendFrameQueueChangeSignal();

	return m_frameTicketsInProcess.empty();
}

// END OF bool VapourSynthScriptProcessor::flushFrameTicketsQueue()
//==============================================================================

void VapourSynthScriptProcessor::slotReceiveFrameAndProcessQueue(
	const VSFrameRef * a_cpFrameRef, int a_frameNumber, VSNodeRef * a_pNodeRef,
	QString a_errorMessage)
{
	receiveFrame(a_cpFrameRef, a_frameNumber, a_pNodeRef, a_errorMessage);
	processFrameTicketsQueue();
}

// END OF void void VapourSynthScriptProcessor::slotReceiveFrameAndProcessQueue(
//		const VSFrameRef * a_cpFrameRef, int a_frameNumber,
//		VSNodeRef * a_pNodeRef, QString a_errorMessage)
//==============================================================================

void VapourSynthScriptProcessor::handleVSMessage(int a_messageType,
	const QString & a_message)
{
	emit signalWriteLogMessage(a_messageType, a_message);
}

// END OF void VapourSynthScriptProcessor::handleVSMessage(int a_messageType,
//		const QString & a_message)
//==============================================================================

void VapourSynthScriptProcessor::receiveFrame(
	const VSFrameRef * a_cpFrameRef, int a_frameNumber,
	VSNodeRef * a_pNodeRef, const QString & a_errorMessage)
{
	assert(m_cpVSAPI);

	std::unique_ptr<const VSFrameRef, std::function<void(const VSFrameRef *)> >
		frameDeleter(a_cpFrameRef, [&](const VSFrameRef * la_cpFrameRef)
		{m_cpVSAPI->freeFrame(la_cpFrameRef);});


	bool discard = false;

	FrameTicket ticket(a_frameNumber, -1, a_pNodeRef);

	std::vector<FrameTicket>::iterator it = std::find(
		m_frameTicketsInProcess.begin(), m_frameTicketsInProcess.end(), ticket);
	if(it != m_frameTicketsInProcess.end())
	{
		ticket.outputIndex = it->outputIndex;
		discard = it->discard;
		m_frameTicketsInProcess.erase(it);
		sendFrameQueueChangeSignal();
	}
	else
	{
		QString warning = trUtf8("Warning: received frame not registered in "
			"processing. Frame number: %1; Node: %2\n")
			.arg(a_frameNumber).arg((intptr_t)a_pNodeRef);
		emit signalWriteLogMessage(mtCritical, warning);
	}

	if(!a_errorMessage.isEmpty())
	{
		m_error = trUtf8("Error on frame %1 request:\n%2")
			.arg(a_frameNumber).arg(a_errorMessage);
		emit signalWriteLogMessage(mtCritical, m_error);
	}

	if((!a_cpFrameRef) || discard)
		return;

	emit signalDistributeFrame(a_frameNumber, ticket.outputIndex, a_cpFrameRef);
}

// END OF void VapourSynthScriptProcessor::receiveFrame(
//		const VSFrameRef * a_cpFrameRef, int a_frameNumber,
//		VSNodeRef * a_pNodeRef, const QString & a_errorMessage)
//==============================================================================

bool VapourSynthScriptProcessor::initLibrary()
{
	if(m_vsScriptLibrary.isLoaded())
	{
		assert(vssInit);
		assert(vssGetVSApi);
		assert(vssEvaluateScript);
		assert(vssGetError);
		assert(vssGetCore);
		assert(vssGetOutput);
		assert(vssFreeScript);
		assert(vssFinalize);
		return true;
	}

#ifdef Q_OS_WIN
	QString libraryName("vsscript");
#else
	QString libraryName("vapoursynth-script");
#endif // Q_OS_WIN

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
			libraryFullPath = path + QString("/") + libraryName;
			m_vsScriptLibrary.setFileName(libraryFullPath);
			loaded = m_vsScriptLibrary.load();
			if(loaded)
				break;
		}
	}

	if(!loaded)
	{
		emit signalWriteLogMessage(mtCritical, "VapourSynth script processor: "
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
		assert(entry.ppFunction);
		*entry.ppFunction = m_vsScriptLibrary.resolve(entry.name);
		if(!*entry.ppFunction)
		{ // Win32 fallback
			*entry.ppFunction = m_vsScriptLibrary.resolve(entry.fallbackName);
		}
		if(!*entry.ppFunction)
		{
			QString errorString = trUtf8("VapourSynth script processor: "
				"Failed to get entry %1() in vapoursynth script library!")
				.arg(entry.name);
			emit signalWriteLogMessage(mtCritical, errorString);
			freeLibrary();
			return false;
		}
	}

	return true;
}

// END OF bool VapourSynthScriptProcessor::initLibrary()
//==============================================================================

void VapourSynthScriptProcessor::freeLibrary()
{
	finalize();

	vssInit = nullptr;
	vssGetVSApi = nullptr;
	vssEvaluateScript = nullptr;
	vssGetError = nullptr;
	vssGetCore = nullptr;
	vssGetOutput = nullptr;
	vssFreeScript = nullptr;
	vssFinalize = nullptr;

	m_vsScriptLibrary.unload();
}

// END OF void VapourSynthScriptProcessor::freeLibrary()
//==============================================================================

void VapourSynthScriptProcessor::processFrameTicketsQueue()
{
	assert(m_cpVSAPI);

	size_t oldInQueue = m_frameTicketsQueue.size();
	size_t oldInProcess = m_frameTicketsInProcess.size();

	while(((int)m_frameTicketsInProcess.size() < m_cpCoreInfo->numThreads) &&
		(!m_frameTicketsQueue.empty()))
	{
		FrameTicket ticket = std::move(m_frameTicketsQueue.front());
		m_frameTicketsQueue.pop_front();
		ticket.pNode = vssGetOutput(m_pVSScript, ticket.outputIndex);
		m_cpVSAPI->getFrameAsync(ticket.frameNumber, ticket.pNode,
			frameReady, this);
		m_cpVSAPI->freeNode(ticket.pNode);
		m_frameTicketsInProcess.push_back(ticket);
	}

	size_t inQueue = m_frameTicketsQueue.size();
	size_t inProcess = m_frameTicketsInProcess.size();
	if((inQueue != oldInQueue) || (oldInProcess != inProcess))
		sendFrameQueueChangeSignal();
}

// END OF void VapourSynthScriptProcessor::processFrameTicketsQueue()
//==============================================================================

void VapourSynthScriptProcessor::sendFrameQueueChangeSignal()
{
	size_t inQueue = m_frameTicketsQueue.size();
	size_t inProcess = m_frameTicketsInProcess.size();
	size_t maxThreads = m_cpCoreInfo->numThreads;
	emit signalFrameQueueStateChanged(inQueue, inProcess, maxThreads);
}

// END OF void VapourSynthScriptProcessor::sendFrameQueueChangeSignal()
//==============================================================================
