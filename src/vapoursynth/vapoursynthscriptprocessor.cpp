#include "vapoursynthscriptprocessor.h"

#include "../common/helpers.h"

#include <cassert>
#include <vector>
#include <cmath>
#include <utility>
#include <memory>

#include <QImage>
#include <QSettings>
#include <QProcessEnvironment>

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
	slotResetSettings();
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

	for(std::pair<const int, NodePair> & mapItem : m_nodePairForOutputIndex)
	{
		NodePair & nodePair = mapItem.second;
		if(nodePair.pOutputNode)
			m_cpVSAPI->freeNode(nodePair.pOutputNode);
		if(nodePair.pPreviewNode)
			m_cpVSAPI->freeNode(nodePair.pPreviewNode);
	}
	m_nodePairForOutputIndex.clear();

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
	int a_outputIndex, bool a_needPreview)
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

	NodePair & nodePair = getNodePair(a_outputIndex, a_needPreview);
	if(!nodePair.pOutputNode)
		return false;

	const VSVideoInfo * cpVideoInfo =
		m_cpVSAPI->getVideoInfo(nodePair.pOutputNode);
	assert(cpVideoInfo);

	if(a_frameNumber >= cpVideoInfo->numFrames)
	{
		m_error = trUtf8("Requested frame number %1 is outside the frame "
			"range.").arg(a_outputIndex);
		emit signalWriteLogMessage(mtCritical, m_error);
		return false;
	}

	if(a_needPreview && (!nodePair.pPreviewNode))
		return false;

	FrameTicket newFrameTicket(a_frameNumber, a_outputIndex,
		nodePair.pOutputNode, a_needPreview, nodePair.pPreviewNode);

	if((int)m_frameTicketsInProcess.size() < m_cpCoreInfo->numThreads)
	{
		m_frameTicketsInProcess.push_back(newFrameTicket);
		if(a_needPreview)
			m_cpVSAPI->getFrameAsync(a_frameNumber, newFrameTicket.pPreviewNode,
				frameReady, this);
		m_cpVSAPI->getFrameAsync(a_frameNumber, newFrameTicket.pOutputNode,
			frameReady, this);
	}
	else
	{
		m_frameTicketsQueue.push_back(newFrameTicket);
	}

	sendFrameQueueChangeSignal();
	return true;
}

// END OF void VapourSynthScriptProcessor::requestFrameAsync(int a_frameNumber,
//		int a_outputIndex, bool a_needPreview)
//==============================================================================

bool VapourSynthScriptProcessor::flushFrameTicketsQueue()
{
	// Check the processing queue.
	for(FrameTicket & ticket : m_frameTicketsInProcess)
		ticket.discard = true;

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

void VapourSynthScriptProcessor::slotResetSettings()
{
	m_yuvMatrix = m_pSettingsManager->getYuvMatrixCoefficients();

	m_chromaResamplingFilter = m_pSettingsManager->getChromaResamplingFilter();
	m_resamplingFilterParameterA = NAN;
	m_resamplingFilterParameterB = NAN;
	if(m_chromaResamplingFilter == ResamplingFilter::Bicubic)
	{
		m_resamplingFilterParameterA =
			m_pSettingsManager->getBicubicFilterParameterB();
		m_resamplingFilterParameterB =
			m_pSettingsManager->getBicubicFilterParameterC();
	}
	else if(m_chromaResamplingFilter == ResamplingFilter::Lanczos)
	{
		m_resamplingFilterParameterA =
			(double)m_pSettingsManager->getLanczosFilterTaps();
	}

	m_chromaPlacement = m_pSettingsManager->getChromaPlacement();

	for(std::pair<const int, NodePair> & mapItem : m_nodePairForOutputIndex)
	{
		NodePair & nodePair = mapItem.second;
		recreatePreviewNode(nodePair);
	}
}

// END OF void VapourSynthScriptProcessor::slotResetSettings()
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

	if(!a_errorMessage.isEmpty())
	{
		m_error = trUtf8("Error on frame %1 request:\n%2")
			.arg(a_frameNumber).arg(a_errorMessage);
		emit signalWriteLogMessage(mtCritical, m_error);
	}

	FrameTicket ticket(a_frameNumber, -1, nullptr);

	std::vector<FrameTicket>::iterator it = std::find_if(
		m_frameTicketsInProcess.begin(), m_frameTicketsInProcess.end(),
		[&](const FrameTicket & a_ticket)
		{
			return ((a_ticket.frameNumber == a_frameNumber) &&
				((a_ticket.pOutputNode == a_pNodeRef) ||
				(a_ticket.pPreviewNode == a_pNodeRef)));
		});
	if(it != m_frameTicketsInProcess.end())
	{
		if(it->pOutputNode == a_pNodeRef)
			it->cpOutputFrameRef = a_cpFrameRef;
		else if(it->pPreviewNode == a_pNodeRef)
			it->cpPreviewFrameRef = a_cpFrameRef;

		// Ticket is incomplete, and received frame is not null.
		// Don't remove the ticket from the queue yet.
		if((!it->isComplete()) && a_cpFrameRef)
			return;

		ticket = *it;
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

	// Ticket is either not found or removed from the queue.
	// It can only be removed from the queue if it was completed or
	// received a null frame reference.

	if(!a_cpFrameRef)
	{
		freeFrameTicket(ticket);
		return;
	}

	assert(ticket.isComplete());

	if(!ticket.discard)
	{
		emit signalDistributeFrame(ticket.frameNumber, ticket.outputIndex,
			ticket.cpOutputFrameRef, ticket.cpPreviewFrameRef);
	}

	freeFrameTicket(ticket);
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

		// In case preview node was hot-swapped.
		NodePair nodePair = getNodePair(ticket.outputIndex, ticket.needPreview);
		if(!nodePair.pOutputNode)
			continue;
		if(ticket.needPreview && (!nodePair.pPreviewNode))
			continue;

		ticket.pOutputNode = nodePair.pOutputNode;
		ticket.pPreviewNode = nodePair.pPreviewNode;

		if(ticket.needPreview)
			m_cpVSAPI->getFrameAsync(ticket.frameNumber, ticket.pPreviewNode,
				frameReady, this);
		m_cpVSAPI->getFrameAsync(ticket.frameNumber, ticket.pOutputNode,
			frameReady, this);

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

bool VapourSynthScriptProcessor::recreatePreviewNode(NodePair & a_nodePair)
{
	if(!a_nodePair.pOutputNode)
		return false;

	if(!m_cpVSAPI)
		return false;

	if(a_nodePair.pPreviewNode)
	{
		m_cpVSAPI->freeNode(a_nodePair.pPreviewNode);
		a_nodePair.pPreviewNode = nullptr;
	}

	const VSVideoInfo * cpVideoInfo =
		m_cpVSAPI->getVideoInfo(a_nodePair.pOutputNode);
	if(!cpVideoInfo)
		return false;
	const VSFormat * cpFormat = cpVideoInfo->format;

	if(cpFormat->id == pfCompatBGR32)
	{
		a_nodePair.pPreviewNode =
			m_cpVSAPI->cloneNodeRef(a_nodePair.pOutputNode);
		return true;
	}

	VSCore * pCore = vssGetCore(m_pVSScript);
	VSPlugin * pResizePlugin = m_cpVSAPI->getPluginById(
		"com.vapoursynth.resize", pCore);
	const char * resizeName = nullptr;

	VSMap * pArgumentMap = m_cpVSAPI->createMap();
	m_cpVSAPI->propSetNode(pArgumentMap, "clip", a_nodePair.pOutputNode,
		paReplace);
	m_cpVSAPI->propSetInt(pArgumentMap, "format", pfCompatBGR32, paReplace);

	switch(m_chromaResamplingFilter)
	{
	case ResamplingFilter::Point:
		resizeName = "Point";
		break;
	case ResamplingFilter::Bilinear:
		resizeName = "Bilinear";
		break;
	case ResamplingFilter::Bicubic:
		resizeName = "Bicubic";
		m_cpVSAPI->propSetFloat(pArgumentMap, "filter_param_a_uv",
			m_resamplingFilterParameterA, paReplace);
		m_cpVSAPI->propSetFloat(pArgumentMap, "filter_param_b_uv",
			m_resamplingFilterParameterB, paReplace);
		break;
	case ResamplingFilter::Lanczos:
		resizeName = "Lanczos";
		m_cpVSAPI->propSetFloat(pArgumentMap, "filter_param_a_uv",
			m_resamplingFilterParameterA, paReplace);
		break;
	case ResamplingFilter::Spline16:
		resizeName = "Spline16";
		break;
	case ResamplingFilter::Spline36:
		resizeName = "Spline36";
		break;
	default:
		assert(false);
	}

	m_cpVSAPI->propSetInt(pArgumentMap, "prefer_props", 1, paReplace);

	const char * matrixInS = nullptr;
	switch(m_yuvMatrix)
	{
	case YuvMatrixCoefficients::m709:
		matrixInS = "709";
		break;
	case YuvMatrixCoefficients::m470BG:
		matrixInS = "470bg";
		break;
	case YuvMatrixCoefficients::m170M:
		matrixInS = "170m";
		break;
	case YuvMatrixCoefficients::m2020_NCL:
		matrixInS = "2020ncl";
		break;
	case YuvMatrixCoefficients::m2020_CL:
		matrixInS = "2020cl";
		break;
	default:
		assert(false);
	}

	int matrixStringLength = (int)strlen(matrixInS);
	m_cpVSAPI->propSetData(pArgumentMap, "matrix_in_s",
		matrixInS, matrixStringLength, paReplace);

	if(m_yuvMatrix == YuvMatrixCoefficients::m2020_CL)
	{
		const char * transferIn = "709";
		const char * transferOut = "2020_10";

		m_cpVSAPI->propSetData(pArgumentMap, "transfer_in_s",
			transferIn, (int)strlen(transferIn), paReplace);
		m_cpVSAPI->propSetData(pArgumentMap, "transfer_s",
			transferOut, (int)strlen(transferOut), paReplace);
	}

	int64_t chromaLoc = 0;
	switch(m_chromaPlacement)
	{
	case ChromaPlacement::MPEG1:
		chromaLoc = 1;
		break;
	case ChromaPlacement::MPEG2:
		chromaLoc = 0;
		break;
	case ChromaPlacement::DV:
		chromaLoc = 2;
		break;
	default:
		assert(false);
	}
	m_cpVSAPI->propSetInt(pArgumentMap, "chromaloc",
		chromaLoc, paReplace);

	VSMap * pResultMap = m_cpVSAPI->invoke(pResizePlugin, resizeName,
		pArgumentMap);

	m_cpVSAPI->freeMap(pArgumentMap);

	const char * cpResultError = m_cpVSAPI->getError(pResultMap);

	if(cpResultError)
	{
		m_error = trUtf8("Failed to convert to RGB:\n");
		m_error += cpResultError;
		emit signalWriteLogMessage(mtCritical, m_error);
		m_cpVSAPI->freeMap(pResultMap);
		return false;
	}

	VSNodeRef * pPreviewNode = m_cpVSAPI->propGetNode(pResultMap, "clip", 0,
		nullptr);
	assert(pPreviewNode);
	a_nodePair.pPreviewNode = pPreviewNode;

	m_cpVSAPI->freeMap(pResultMap);

	return true;
}

// END OF bool VapourSynthScriptProcessor::recreatePreviewNode(
//		NodePair & a_nodePair)
//==============================================================================

void VapourSynthScriptProcessor::freeFrameTicket(FrameTicket & a_ticket)
{
	assert(m_cpVSAPI);
	a_ticket.discard = true;
	if(a_ticket.cpOutputFrameRef)
		m_cpVSAPI->freeFrame(a_ticket.cpOutputFrameRef);
	if(a_ticket.cpPreviewFrameRef)
		m_cpVSAPI->freeFrame(a_ticket.cpPreviewFrameRef);
}

// END OF void VapourSynthScriptProcessor::freeFrameTicket(
//		FrameTicket & a_ticket)
//==============================================================================

NodePair VapourSynthScriptProcessor::getNodePair(int a_outputIndex,
	bool a_needPreview)
{
	NodePair & nodePair = m_nodePairForOutputIndex[a_outputIndex];

	if(!nodePair.pOutputNode)
	{
		assert(!nodePair.pPreviewNode);

		nodePair.pOutputNode = vssGetOutput(m_pVSScript, a_outputIndex);
		if(!nodePair.pOutputNode)
		{
			m_error = trUtf8("Couldn't resolve output node number %1.")
				.arg(a_outputIndex);
			emit signalWriteLogMessage(mtCritical, m_error);
			return nodePair;
		}
	}

	if(a_needPreview && (!nodePair.pPreviewNode))
	{
		bool previewNodeCreated = recreatePreviewNode(nodePair);
		if(!previewNodeCreated)
		{
			m_error = trUtf8("Couldn't create preview node for output "
				"number %1.").arg(a_outputIndex);
			emit signalWriteLogMessage(mtCritical, m_error);
			return nodePair;
		}
	}

	return nodePair;
}

// END OF NodePair VapourSynthScriptProcessor::getNodePair(int a_outputIndex,
//		bool a_needPreview)
//==============================================================================
