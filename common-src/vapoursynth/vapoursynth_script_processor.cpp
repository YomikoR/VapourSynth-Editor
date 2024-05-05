#include "vapoursynth_script_processor.h"

#include "../helpers.h"
#include "vs_script_library.h"
#include "vs_pack_rgb.h"
#include "vs_set_matrix.h"

#include <vapoursynth/VSHelper4.h>
#include <vapoursynth/VSConstants4.h>

#include <vector>
#include <cmath>
#include <utility>
#include <memory>
#include <functional>
#include <QPixmap>
#include <QColormap>

//==============================================================================

void VS_CC frameReady(void * a_pUserData,
	const VSFrame * a_cpFrame, int a_frameNumber,
	VSNode * a_pNode, const char * a_errorMessage)
{
	VapourSynthScriptProcessor * pScriptProcessor =
		static_cast<VapourSynthScriptProcessor *>(a_pUserData);
	Q_ASSERT(pScriptProcessor);
	QString errorMessage(a_errorMessage);
	QMetaObject::invokeMethod(pScriptProcessor,
		"slotReceiveFrameAndProcessQueue",
		Qt::QueuedConnection,
		Q_ARG(const VSFrame *, a_cpFrame),
		Q_ARG(int, a_frameNumber),
		Q_ARG(VSNode *, a_pNode),
		Q_ARG(QString, errorMessage));
}

// END OF void VS_CC frameReady(void * a_pUserData,
//	const VSFrame * a_cpFrame, int a_frameNumber,
//	VSNode * a_pNode, const char * a_errorMessage)
//==============================================================================

VapourSynthScriptProcessor::VapourSynthScriptProcessor(
	SettingsManagerCore * a_pSettingsManager,
	VSScriptLibrary * a_pVSScriptLibrary,
	QObject * a_pParent):
	QObject(a_pParent)
	, m_pSettingsManager(a_pSettingsManager)
	, m_pVSScriptLibrary(a_pVSScriptLibrary)
	, m_script()
	, m_scriptName()
	, m_error()
	, m_initialized(false)
	, m_cpVSAPI(nullptr)
	, m_pVSScript(nullptr)
	, m_pCore(nullptr)
	, m_nodeInfo()
	, m_finalizing(false)
{
	Q_ASSERT(m_pSettingsManager);
	Q_ASSERT(m_pVSScriptLibrary);

	slotResetSettings();
}

// END OF VapourSynthScriptProcessor::VapourSynthScriptProcessor(
//		SettingsManagerCore * a_pSettingsManager,
//		VSScriptLibrary * a_pVSScriptLibrary, QObject * a_pParent)
//==============================================================================

VapourSynthScriptProcessor::~VapourSynthScriptProcessor()
{
	finalize();
}

// END OF VapourSynthScriptProcessor::~VapourSynthScriptProcessor()
//==============================================================================

bool VapourSynthScriptProcessor::initialize(const QString& a_script,
	const QString& a_scriptName, int a_outputIndex, ProcessReason a_reason)
{
	if(m_initialized || m_finalizing)
	{
		m_error = tr("Script processor is already in use.");
		emit signalWriteLogMessage(mtCritical, m_error);
		return false;
	}

	if(!m_pVSScript || !m_pCore)
	{
		m_pCore = m_pVSScriptLibrary->createCore();
		m_pVSScript = m_pVSScriptLibrary->createScript(m_pCore);
	}
	if(!m_pVSScript)
	{
		m_error = tr("Failed to create VSScript handle!");
		emit signalWriteLogMessage(mtCritical, m_error);
		finalize();
		return false;
	}

	m_cpVSAPI = m_pVSScriptLibrary->getVSAPI();
	if(!m_cpVSAPI)
	{
		finalize();
		return false;
	}

	m_cpVSAPI->getCoreInfo(m_pCore, &m_cpCoreInfo);

	int opresult = m_pVSScriptLibrary->evaluateScript(m_pVSScript,
		a_script.toUtf8().constData(), a_scriptName.toUtf8().constData());

	if(opresult)
	{
		m_error = tr("Failed to evaluate the script");
		const char * vsError = m_pVSScriptLibrary->getError(m_pVSScript);
		if(vsError)
			m_error += QString(":\n") + vsError;
		else
			m_error += '.';

		emit signalWriteLogMessage(mtCritical, m_error);
		finalize();
		return false;
	}

	VSNode * pOutputNode = m_pVSScriptLibrary->getOutput(
		m_pVSScript, a_outputIndex);
	if(!pOutputNode)
	{
		m_error = tr("Failed to get output node #%1.")
			.arg(a_outputIndex);
		emit signalWriteLogMessage(mtCritical, m_error);
		finalize();
		return false;
	}

	m_nodeInfo = VSNodeInfo(pOutputNode, m_cpVSAPI);

#ifdef Q_OS_WIN // AUDIO
	if(a_reason == ProcessReason::Encode && m_nodeInfo.isAudio())
#else
	if((a_reason == ProcessReason::Preview
		|| a_reason == ProcessReason::Encode) && m_nodeInfo.isAudio())
#endif
	{
		m_cpVSAPI->freeNode(pOutputNode);
		m_error = tr("Output node #%1 is audio. "
#ifdef Q_OS_WIN // AUDIO
			"Encoding audio are not supported.")
#else
			"Previewing and encoding audio are not supported.")
#endif
			.arg(a_outputIndex);
		emit signalWriteLogMessage(
			a_outputIndex == 0 ? mtCritical : mtWarning, m_error);
		finalize();
		return false;
	}

	if(m_nodeInfo.isVideo() &&
		m_nodeInfo.getAsVideo()->format.colorFamily == cfUndefined)
	{
		emit signalWriteLogMessage(mtWarning,
			tr("Video output node #%1 has Undefined format.")
			.arg(a_outputIndex));
	}

	m_cpVSAPI->freeNode(pOutputNode);

	m_script = a_script;
	m_scriptName = a_scriptName;

	m_error.clear();
	m_initialized = true;

	sendFrameQueueChangeSignal();

	return true;
}

// END OF bool VapourSynthScriptProcessor::initialize(const QString& a_script,
//		const QString& a_scriptName, int a_outputIndex, ProcessReason a_reason)
//==============================================================================

bool VapourSynthScriptProcessor::finalize()
{
	m_finalizing = true;
	bool noFrameTicketsInProcess = flushFrameTicketsQueue();
	if(!noFrameTicketsInProcess)
		return false;

	for(std::pair<const int, NodePair> & mapItem : m_nodePairForOutputIndex)
	{
		NodePair & nodePair = mapItem.second;
		if(nodePair.pOutputNode)
			m_cpVSAPI->freeNode(nodePair.pOutputNode);
		if(nodePair.pPreviewNode)
			m_cpVSAPI->freeNode(nodePair.pPreviewNode);
	}
	m_nodePairForOutputIndex.clear();

	if(m_pVSScript)
	{
		m_pVSScriptLibrary->freeScript(m_pVSScript);
		m_pVSScript = nullptr;
	}

	m_cpVSAPI = nullptr;

	m_script.clear();
	m_scriptName.clear();

	m_initialized = false;
	m_finalizing = false;

	emit signalFinalized();

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

VSNodeInfo VapourSynthScriptProcessor::nodeInfo(int a_outputIndex)
{
	if(!m_initialized)
		return VSNodeInfo();

	Q_ASSERT(m_cpVSAPI);
	Q_ASSERT(m_pVSScript);

	VSNode * pNode =
		m_pVSScriptLibrary->getOutput(m_pVSScript, a_outputIndex);
	if(!pNode)
	{
		m_error = tr("Couldn't resolve output node #%1.")
			.arg(a_outputIndex);
		emit signalWriteLogMessage(
			a_outputIndex == 0 ? mtCritical : mtWarning, m_error);
		return VSNodeInfo();
	}

	VSNodeInfo nodeInfo(pNode, m_cpVSAPI);

	m_cpVSAPI->freeNode(pNode);

	return nodeInfo;
}

// END OF VSNodeInfo VapourSynthScriptProcessor::nodeInfo(int a_outputIndex)
//==============================================================================

bool VapourSynthScriptProcessor::requestFrameAsync(int a_frameNumber,
	int a_outputIndex, bool a_needPreview)
{
	if(!m_initialized)
		return false;

	if(a_frameNumber < 0)
	{
		m_error = tr("Requested frame number %1 is negative.")
			.arg(a_frameNumber);
		emit signalWriteLogMessage(mtCritical, m_error);
		return false;
	}

	Q_ASSERT(m_cpVSAPI);

	NodePair & nodePair = getNodePair(a_outputIndex, a_needPreview);
	if(!nodePair.pOutputNode)
		return false;

	int numFrames;
	int mediaType = m_cpVSAPI->getNodeType(nodePair.pOutputNode);
	if(mediaType == mtAudio)
	{
		auto *info = m_cpVSAPI->getAudioInfo(nodePair.pOutputNode);
		Q_ASSERT(info);
		numFrames = info->numFrames;
	}
	else
	{
		auto *info = m_cpVSAPI->getVideoInfo(nodePair.pOutputNode);
		Q_ASSERT(info);
		numFrames = info->numFrames;
	}

	if(a_frameNumber >= numFrames)
	{
		m_error = tr("Requested frame number %1 is outside the frame "
			"range.").arg(a_outputIndex);
		emit signalWriteLogMessage(mtCritical, m_error);
		return false;
	}

	if(a_needPreview && (!nodePair.pPreviewNode))
		return false;

	FrameTicket newFrameTicket(a_frameNumber, a_outputIndex,
		nodePair.pOutputNode, a_needPreview, nodePair.pPreviewNode);

	m_frameTicketsQueue.push_back(newFrameTicket);
	sendFrameQueueChangeSignal();
	processFrameTicketsQueue();

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

const QString & VapourSynthScriptProcessor::script() const
{
	return m_script;
}

// END OF const QString & VapourSynthScriptProcessor::script() const
//==============================================================================

const QString & VapourSynthScriptProcessor::scriptName() const
{
	return m_scriptName;
}

// END OF const QString & VapourSynthScriptProcessor::scriptName() const
//==============================================================================

void VapourSynthScriptProcessor::setScriptName(const QString & a_scriptName)
{
	m_scriptName = a_scriptName;
}

// END OF void VapourSynthScriptProcessor::setScriptName(
//		const QString & a_scriptName)
//==============================================================================

void VapourSynthScriptProcessor::slotReceiveFrameAndProcessQueue(
	const VSFrame * a_cpFrame, int a_frameNumber, VSNode * a_pNode,
	QString a_errorMessage)
{
	receiveFrame(a_cpFrame, a_frameNumber, a_pNode, a_errorMessage);
	processFrameTicketsQueue();
}

// END OF void VapourSynthScriptProcessor::slotReceiveFrameAndProcessQueue(
//		const VSFrame * a_cpFrame, int a_frameNumber,
//		VSNode * a_pNode, QString a_errorMessage)
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

	m_ditherType = m_pSettingsManager->getDitherType();

	for(std::pair<const int, NodePair> & mapItem : m_nodePairForOutputIndex)
	{
		NodePair & nodePair = mapItem.second;
		if(nodePair.pPreviewNode)
			recreatePreviewNode(nodePair);
	}
}

// END OF void VapourSynthScriptProcessor::slotResetSettings()
//==============================================================================

void VapourSynthScriptProcessor::receiveFrame(
	const VSFrame * a_cpFrame, int a_frameNumber,
	VSNode * a_pNode, const QString & a_errorMessage)
{
	Q_ASSERT(m_cpVSAPI);

	if(!a_errorMessage.isEmpty())
	{
		m_error = tr("Error on frame %1 request:\n%2")
			.arg(a_frameNumber).arg(a_errorMessage);
		emit signalWriteLogMessage(mtCritical, m_error);
	}

	FrameTicket ticket(a_frameNumber, -1, nullptr);

	std::vector<FrameTicket>::iterator it = std::find_if(
		m_frameTicketsInProcess.begin(), m_frameTicketsInProcess.end(),
		[&](const FrameTicket & a_ticket)
		{
			return ((a_ticket.frameNumber == a_frameNumber) &&
				((a_ticket.pOutputNode == a_pNode) ||
				(a_ticket.pPreviewNode == a_pNode)));
		});

	if(it != m_frameTicketsInProcess.end())
	{
		// Save frame references and free node references in ticket at once.
		if(it->pOutputNode == a_pNode)
		{
			it->cpOutputFrame = a_cpFrame;
			m_cpVSAPI->freeNode(it->pOutputNode);
			it->pOutputNode = nullptr;

			if(it->needPreview)
			{
				Q_ASSERT(it->pPreviewNode);
				if(a_cpFrame)
				{
					m_cpVSAPI->getFrameAsync(it->frameNumber, it->pPreviewNode,
						frameReady, this);
				}
				else
				{
					m_cpVSAPI->freeNode(it->pPreviewNode);
					it->pPreviewNode = nullptr;
				}
			}
		}
		else if(it->pPreviewNode == a_pNode)
		{
			it->cpPreviewFrame = a_cpFrame;
			m_cpVSAPI->freeNode(it->pPreviewNode);
			it->pPreviewNode = nullptr;
		}

		// Since we nullify the nodes in ticket - we can check if it can be
		// removed from the queue by checking the nodes.
		if((it->pOutputNode != nullptr) ||
			(it->needPreview && (it->pPreviewNode != nullptr)))
			return;

		ticket = *it;
		m_frameTicketsInProcess.erase(it);
		sendFrameQueueChangeSignal();
	}
	else
	{
		QString warning = tr("Warning: received frame not registered in "
			"processing. Frame number: %1; Node: %2\n")
			.arg(a_frameNumber).arg((intptr_t)a_pNode);
		emit signalWriteLogMessage(mtCritical, warning);
		m_cpVSAPI->freeFrame(a_cpFrame);
	}

	if(!ticket.discard)
	{
		if(ticket.isComplete())
		{
			emit signalDistributeFrame(ticket.frameNumber, ticket.outputIndex,
				ticket.cpOutputFrame, ticket.cpPreviewFrame);
		}
		else
		{
			emit signalFrameRequestDiscarded(ticket.frameNumber,
				ticket.outputIndex, QString());
		}
	}

	freeFrameTicket(ticket);
}

// END OF void VapourSynthScriptProcessor::receiveFrame(
//		const VSFrame * a_cpFrame, int a_frameNumber,
//		VSNode * a_pNode, const QString & a_errorMessage)
//==============================================================================

void VapourSynthScriptProcessor::processFrameTicketsQueue()
{
	Q_ASSERT(m_cpVSAPI);

	size_t oldInQueue = m_frameTicketsQueue.size();
	size_t oldInProcess = m_frameTicketsInProcess.size();

	while(((int)m_frameTicketsInProcess.size() < m_cpCoreInfo.numThreads) &&
		(!m_frameTicketsQueue.empty()))
	{
		FrameTicket ticket = std::move(m_frameTicketsQueue.front());
		m_frameTicketsQueue.pop_front();

		// In case preview node was hot-swapped.
		NodePair & nodePair =
			getNodePair(ticket.outputIndex, ticket.needPreview);

		bool validPair = (nodePair.pOutputNode != nullptr);
		if(ticket.needPreview)
			validPair = validPair && (nodePair.pPreviewNode != nullptr);
		if(!validPair)
		{
			QString reason = tr("No nodes to produce the frame "
				"%1 at output #%2.").arg(ticket.frameNumber)
				.arg(ticket.outputIndex);
			emit signalFrameRequestDiscarded(ticket.frameNumber,
				ticket.outputIndex, reason);
			continue;
		}

		ticket.pOutputNode = m_cpVSAPI->addNodeRef(nodePair.pOutputNode);
		if(ticket.needPreview)
			ticket.pPreviewNode =
				m_cpVSAPI->addNodeRef(nodePair.pPreviewNode);

		m_cpVSAPI->getFrameAsync(ticket.frameNumber, ticket.pOutputNode,
			frameReady, this);

		m_frameTicketsInProcess.push_back(ticket);
	}

	size_t inQueue = m_frameTicketsQueue.size();
	size_t inProcess = m_frameTicketsInProcess.size();
	if((inQueue != oldInQueue) || (oldInProcess != inProcess))
		sendFrameQueueChangeSignal();

	if(m_finalizing)
		finalize();
}

// END OF void VapourSynthScriptProcessor::processFrameTicketsQueue()
//==============================================================================

void VapourSynthScriptProcessor::sendFrameQueueChangeSignal()
{
	size_t inQueue = m_frameTicketsQueue.size();
	size_t inProcess = m_frameTicketsInProcess.size();

	m_cpVSAPI->getCoreInfo(m_pCore, &m_cpCoreInfo);

	size_t maxThreads = m_cpCoreInfo.numThreads;
	double usedCacheRatio = (double)m_cpCoreInfo.usedFramebufferSize
		/ (double)m_cpCoreInfo.maxFramebufferSize;
	emit signalFrameQueueStateChanged(inQueue, inProcess, maxThreads,
		usedCacheRatio);
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

	int outputMediaType = m_cpVSAPI->getNodeType(a_nodePair.pOutputNode);
	if(outputMediaType == mtAudio)
	{
#ifdef Q_OS_WIN // AUDIO
		return recreateAudioPreviewNode(a_nodePair);
#else
		m_error = tr("Audio playback is supported on Windows only.");
		emit signalWriteLogMessage(mtCritical, m_error);
		return false;
#endif
	}

	const VSVideoInfo * cpVideoInfo =
		m_cpVSAPI->getVideoInfo(a_nodePair.pOutputNode);
	if(!cpVideoInfo)
		return false;
	const VSVideoFormat * cpFormat = &cpVideoInfo->format;

	bool to_10_bit = (QColormap::instance().depth() == 30);

	VSMap * pResultMap = nullptr;

	if(vsh::isSameVideoPresetFormat(pfRGB24, cpFormat, m_pCore, m_cpVSAPI))
	{
		to_10_bit = false;
		pResultMap = m_cpVSAPI->createMap();
		m_cpVSAPI->mapSetNode(pResultMap, "clip", a_nodePair.pOutputNode,
			maReplace);
	}
	else if(to_10_bit &&
		vsh::isSameVideoPresetFormat(pfRGB30, cpFormat, m_pCore, m_cpVSAPI))
	{
		pResultMap = m_cpVSAPI->createMap();
		m_cpVSAPI->mapSetNode(pResultMap, "clip", a_nodePair.pOutputNode,
			maReplace);
	}
	else
	{
		bool isVF = isVariableFormat(cpVideoInfo);
		bool isYUV = cpFormat->colorFamily == cfYUV;

		VSPlugin * pResizePlugin = m_cpVSAPI->getPluginByID(
			"com.vapoursynth.resize", m_pCore);
		const char * resizeName = "Point";

		VSMap * pArgumentMap = m_cpVSAPI->createMap();

		VSCoreInfo coreInfo;
		m_cpVSAPI->getCoreInfo(m_pCore, &coreInfo);
		if(coreInfo.core < 58)
			m_cpVSAPI->mapSetInt(pArgumentMap, "prefer_props", 1, maReplace);

		// Set matrix and chromaloc

		int64_t matrixIn;
		switch(m_yuvMatrix)
		{
		case YuvMatrixCoefficients::m709:
			matrixIn = VSC_MATRIX_BT709;
			break;
		case YuvMatrixCoefficients::m470BG:
			matrixIn = VSC_MATRIX_BT470_BG;
			break;
		case YuvMatrixCoefficients::m170M:
			matrixIn = VSC_MATRIX_ST170_M;
			break;
		case YuvMatrixCoefficients::m2020_NCL:
			matrixIn = VSC_MATRIX_BT2020_NCL;
			break;
		default:
			Q_ASSERT(false);
		}

		m_cpVSAPI->mapSetInt(pArgumentMap, "matrix_in", matrixIn, maReplace);

		int64_t chromaLoc;
		switch(m_chromaPlacement)
		{
		case ChromaPlacement::LEFT:
			chromaLoc = VSC_CHROMA_LEFT;
			break;
		case ChromaPlacement::CENTER:
			chromaLoc = VSC_CHROMA_CENTER;
			break;
		case ChromaPlacement::TOP_LEFT:
			chromaLoc = VSC_CHROMA_TOP_LEFT;
			break;
		default:
			Q_ASSERT(false);
		}
		m_cpVSAPI->mapSetInt(pArgumentMap, "chromaloc_in", chromaLoc, maReplace);

		QString ditherType;
		switch (m_ditherType)
		{
		case DitherType::NONE:
			ditherType = "none";
			break;
		case DitherType::ORDERED:
			ditherType = "ordered";
			break;
		case DitherType::RANDOM:
			ditherType = "random";
			break;
		case DitherType::ERROR_DIFFUSION:
			ditherType = "error_diffusion";
			break;
		default:
			Q_ASSERT(false);
		}
		m_cpVSAPI->mapSetData(pArgumentMap, "dither_type",
			ditherType.toStdString().c_str(), ditherType.size(),
			dtUtf8, maReplace);

		VSMap *pTempMap = m_cpVSAPI->createMap();
		m_cpVSAPI->copyMap(pArgumentMap, pTempMap);
		m_cpVSAPI->mapSetNode(pTempMap, "clip",
			a_nodePair.pOutputNode, maReplace);
		setMatrixFilter(pTempMap, pArgumentMap, m_pCore, m_cpVSAPI);
		m_cpVSAPI->clearMap(pTempMap);

		m_cpVSAPI->mapSetInt(pArgumentMap, "format", (to_10_bit ?
			pfRGB30 : pfRGB24), maReplace);

		if(isYUV || isVF)
		{
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
				m_cpVSAPI->mapSetFloat(pArgumentMap, "filter_param_a_uv",
					m_resamplingFilterParameterA, maReplace);
				m_cpVSAPI->mapSetFloat(pArgumentMap, "filter_param_b_uv",
					m_resamplingFilterParameterB, maReplace);
				break;
			case ResamplingFilter::Lanczos:
				resizeName = "Lanczos";
				m_cpVSAPI->mapSetFloat(pArgumentMap, "filter_param_a_uv",
					m_resamplingFilterParameterA, maReplace);
				break;
			case ResamplingFilter::Spline16:
				resizeName = "Spline16";
				break;
			case ResamplingFilter::Spline36:
				resizeName = "Spline36";
				break;
			case ResamplingFilter::Spline64:
				resizeName = "Spline64";
			default:
				Q_ASSERT(false);
			}
		}

		pResultMap = m_cpVSAPI->invoke(pResizePlugin, resizeName, pArgumentMap);

		m_cpVSAPI->freeMap(pArgumentMap);

	}

	const char * cpResultError = m_cpVSAPI->mapGetError(pResultMap);

	if(cpResultError)
	{
		m_error = tr("Failed to convert to RGB:\n");
		m_error += cpResultError;
		emit signalWriteLogMessage(mtCritical, m_error);
		m_cpVSAPI->freeMap(pResultMap);
		return false;
	}

	VSNode * pRGBNode = m_cpVSAPI->mapGetNode(pResultMap, "clip", 0, nullptr);
	m_cpVSAPI->freeMap(pResultMap);

	VSNode * pPreviewNode = packRGBFilter(pRGBNode, a_nodePair.pOutputNode,
		to_10_bit, m_pCore, m_cpVSAPI);

	Q_ASSERT(pPreviewNode);
	a_nodePair.pPreviewNode = pPreviewNode;

	return true;
}

// END OF bool VapourSynthScriptProcessor::recreatePreviewNode(
//		NodePair & a_nodePair)
//==============================================================================

bool VapourSynthScriptProcessor::recreateAudioPreviewNode(NodePair &a_nodePair)
{
	Q_ASSERT(m_pCore);

	const VSAudioInfo * cpAudioInfo =
		m_cpVSAPI->getAudioInfo(a_nodePair.pOutputNode);

	VSMap * map = m_cpVSAPI->createMap();
	VSPlugin *stdPlugin = m_cpVSAPI->getPluginByID(VSH_STD_PLUGIN_ID, m_pCore);
	m_cpVSAPI->mapSetInt(map, "length", cpAudioInfo->numFrames, maReplace);
	VSMap * blankMap = m_cpVSAPI->invoke(stdPlugin, "BlankClip", map);
	m_cpVSAPI->clearMap(map);

	QString props = vsedit::nodeInfoString(
		nodeInfo(a_nodePair.outputIndex), m_cpVSAPI);
	QString text = QString("Audio node #%1\n\n").arg(a_nodePair.outputIndex)
		+ props.replace("| ", "\n");
    m_cpVSAPI->mapSetData(blankMap, "text", text.toStdString().c_str(),
		text.size(), dtUtf8, maReplace);
	VSPlugin * textPlugin = m_cpVSAPI->getPluginByID(VSH_TEXT_PLUGIN_ID, m_pCore);
    VSMap * textMap = m_cpVSAPI->invoke(textPlugin, "Text", blankMap);
	m_cpVSAPI->clearMap(blankMap);

	VSNode * textClip = m_cpVSAPI->mapGetNode(textMap, "clip", 0, nullptr);
	m_cpVSAPI->clearMap(textMap);

	a_nodePair.pPreviewNode = packRGBFilter(textClip, a_nodePair.pOutputNode,
		false, m_pCore, m_cpVSAPI);
    return true;
}

void VapourSynthScriptProcessor::freeFrameTicket(FrameTicket & a_ticket)
{
	Q_ASSERT(m_cpVSAPI);

	a_ticket.discard = true;

	if(a_ticket.cpOutputFrame)
	{
		m_cpVSAPI->freeFrame(a_ticket.cpOutputFrame);
		a_ticket.cpOutputFrame = nullptr;
	}

	if(a_ticket.cpPreviewFrame)
	{
		m_cpVSAPI->freeFrame(a_ticket.cpPreviewFrame);
		a_ticket.cpPreviewFrame = nullptr;
	}

	if(a_ticket.pOutputNode)
	{
		m_cpVSAPI->freeNode(a_ticket.pOutputNode);
		a_ticket.pOutputNode = nullptr;
	}

	if(a_ticket.pPreviewNode)
	{

		m_cpVSAPI->freeNode(a_ticket.pPreviewNode);
		a_ticket.pPreviewNode = nullptr;
	}
}

// END OF void VapourSynthScriptProcessor::freeFrameTicket(
//		FrameTicket & a_ticket)
//==============================================================================

NodePair & VapourSynthScriptProcessor::getNodePair(int a_outputIndex,
	bool a_needPreview)
{
	NodePair & nodePair = m_nodePairForOutputIndex[a_outputIndex];

	nodePair.outputIndex = a_outputIndex;

	if(!nodePair.pOutputNode)
	{
		Q_ASSERT(!nodePair.pPreviewNode);

		nodePair.pOutputNode =
			m_pVSScriptLibrary->getOutput(m_pVSScript, a_outputIndex);
		if(!nodePair.pOutputNode)
		{
			m_error = tr("Couldn't resolve output node #%1.")
				.arg(a_outputIndex);
			emit signalWriteLogMessage(
				a_outputIndex == 0 ? mtCritical : mtWarning, m_error);
			return nodePair;
		}
	}

	if(a_needPreview && (!nodePair.pPreviewNode))
	{
		bool previewNodeCreated = recreatePreviewNode(nodePair);
		if(!previewNodeCreated)
		{
			m_error = tr("Couldn't create preview node for output "
				"#%1.").arg(a_outputIndex);
			emit signalWriteLogMessage(mtCritical, m_error);
			return nodePair;
		}
	}

	return nodePair;
}

// END OF NodePair VapourSynthScriptProcessor::getNodePair(int a_outputIndex,
//		bool a_needPreview)
//==============================================================================

QString VapourSynthScriptProcessor::framePropsString(
	const VSFrame * a_cpFrame) const
{
	if(!a_cpFrame)
		return tr("Null frame.");

	Q_ASSERT(m_cpVSAPI);

	QString propsString;
	QStringList propsStringList;

	static std::map<VSPropertyType, QString> propTypeToString =
	{
		{ptUnset, "<unset>"},
		{ptInt, "int"},
		{ptFloat, "float"},
		{ptData, "data"},
		{ptFunction, "function"},
		{ptVideoNode, "vnode"},
		{ptVideoFrame, "vframe"},
		{ptAudioNode, "anode"},
		{ptAudioFrame, "aframe"}
	};

	static std::map<int64_t, QString> _ChromaLocationToString = 
	{
		{VSC_CHROMA_LEFT, "left"},
		{VSC_CHROMA_CENTER, "center"},
		{VSC_CHROMA_TOP_LEFT, "top left"},
		{VSC_CHROMA_BOTTOM_LEFT, "bottom left"},
		{VSC_CHROMA_BOTTOM, "bottom"},
	};

	static std::map<int64_t, QString> _ColorRangeToString = 
	{
		{VSC_RANGE_FULL, "full"},
		{VSC_RANGE_LIMITED, "limited"},
	};

	static std::map<int64_t, QString> _PrimariesToString = 
	{
		{VSC_PRIMARIES_BT709, "709"},
		{VSC_PRIMARIES_UNSPECIFIED, "unspec"},
		{VSC_PRIMARIES_BT470_M, "470m"},
		{VSC_PRIMARIES_BT470_BG, "470bg"},
		{VSC_PRIMARIES_ST170_M, "170m"},
		{VSC_PRIMARIES_ST240_M, "240m"},
		{VSC_PRIMARIES_FILM, "film"},
		{VSC_PRIMARIES_BT2020, "2020"},
		{VSC_PRIMARIES_ST428, "st428"},
		{VSC_PRIMARIES_ST431_2, "st431-2"},
		{VSC_PRIMARIES_ST432_1, "st432-1"},
		{VSC_PRIMARIES_EBU3213_E, "jedec-p22"},
	};

	static std::map<int64_t, QString> _MatrixToString =
	{
		{VSC_MATRIX_RGB, "rgb"},
		{VSC_MATRIX_BT709, "709"},
		{VSC_MATRIX_UNSPECIFIED, "unspec"},
		{VSC_MATRIX_FCC, "fcc"},
		{VSC_MATRIX_BT470_BG, "470bg"},
		{VSC_MATRIX_ST170_M, "170m"},
		{VSC_MATRIX_ST240_M, "240m"},
		{VSC_MATRIX_YCGCO, "ycgco"},
		{VSC_MATRIX_BT2020_NCL, "2020ncl"},
		{VSC_MATRIX_BT2020_CL, "2020cl"},
		{VSC_MATRIX_CHROMATICITY_DERIVED_NCL, "chromancl"},
		{VSC_MATRIX_CHROMATICITY_DERIVED_CL, "chromacl"},
		{VSC_MATRIX_ICTCP, "ictcp"},
	};

	static std::map<int64_t, QString> _TransferToString =
	{
		{VSC_TRANSFER_BT709, "709"},
		{VSC_TRANSFER_UNSPECIFIED, "unspec"},
		{VSC_TRANSFER_BT470_M, "470m"},
		{VSC_TRANSFER_BT470_BG, "470bg"},
		{VSC_TRANSFER_BT601, "601"},
		{VSC_TRANSFER_ST240_M, "240m"},
		{VSC_TRANSFER_LINEAR, "linear"},
		{VSC_TRANSFER_LOG_100, "log100"},
		{VSC_TRANSFER_LOG_316, "log316"},
		{VSC_TRANSFER_IEC_61966_2_4, "xvycc"},
		{VSC_TRANSFER_IEC_61966_2_1, "srgb"},
		{VSC_TRANSFER_BT2020_10, "2020_10"},
		{VSC_TRANSFER_BT2020_12, "2020_12"},
		{VSC_TRANSFER_ST2084, "st2084"},
		//{VSC_TRANSFER_ST428, "st428"},
		{17, "st428"},
		{VSC_TRANSFER_ARIB_B67, "std-b67"},
	};

	static std::map<int64_t, QString> _FieldBasedToString =
	{
		{VSC_FIELD_PROGRESSIVE, "progressive"},
		{VSC_FIELD_BOTTOM, "bottom field first"},
		{VSC_FIELD_TOP, "top field first"},
	};

	static std::map<int64_t, QString> _FieldToString =
	{
		{0, "from bottom field"},
		{1, "from top field"},
	};

	static std::map<QString, std::map<int64_t, QString>> reservedPropToMap =
	{
		{"_ChromaLocation", _ChromaLocationToString},
		{"_ColorRange", _ColorRangeToString},
		{"_Primaries", _PrimariesToString},
		{"_Matrix", _MatrixToString},
		{"_Transfer", _TransferToString},
		{"_FieldBased", _FieldBasedToString},
		{"_Field", _FieldToString},
	};

	const VSMap * cpProps = m_cpVSAPI->getFramePropertiesRO(a_cpFrame);

	int propsNumber = m_cpVSAPI->mapNumKeys(cpProps);
	for(int i = 0; i < propsNumber; ++i)
	{
		const char * propKey = m_cpVSAPI->mapGetKey(cpProps, i);
		if(!propKey)
			continue;
		QString currentPropString = QString("%1 : ").arg(propKey);
		auto propType = (VSPropertyType)m_cpVSAPI->mapGetType(cpProps, propKey);
		currentPropString += propTypeToString[propType];
		int elementsNumber = m_cpVSAPI->mapNumElements(cpProps, propKey);
		if(elementsNumber > 1)
			currentPropString += "[]";
		switch(propType)
		{
		case ptVideoFrame:
		case ptVideoNode:
		case ptAudioFrame:
		case ptAudioNode:
		case ptFunction:
			break;
		case ptUnset:
			currentPropString += ": <unset>";
			break;
		case ptInt:
		{
			currentPropString += " : ";
			QStringList elementStringList;
			for(int j = 0; j < elementsNumber; ++j)
			{
				QString elementString;
				int error;
				int64_t element = m_cpVSAPI->mapGetInt(cpProps,
					propKey, j, &error);
				if(error)
					elementString = "<error>";
				else
				{
					auto it = reservedPropToMap.find(QString(propKey));
					if(it == reservedPropToMap.end())
						elementString = QString::number(element);
					else
					{
						auto it2 = it->second.find(element);
						if(it2 == it->second.end())
							elementString = QString::number(element);
						else
							elementString = QString("%1 (%2)")
								.arg(element).arg(it2->second);
					}
				}

				elementStringList += elementString;
			}
			currentPropString += elementStringList.join(", ");
			break;
		}
		case ptFloat:
		{
			currentPropString += " : ";
			QStringList elementStringList;
			for(int j = 0; j < elementsNumber; ++j)
			{
				QString elementString;
				int error;
				double element = m_cpVSAPI->mapGetFloat(cpProps,
					propKey, j, &error);
				if(error)
					elementString = "<error>";
				else
					elementString = QString::number(element);

				elementStringList += elementString;
			}
			currentPropString += elementStringList.join(", ");
			break;
		}
		case ptData:
		{
			currentPropString += " : ";
			QStringList elementStringList;
			for(int j = 0; j < elementsNumber; ++j)
			{
				QString elementString;
				int error;
				int hint = m_cpVSAPI->mapGetDataTypeHint(cpProps,
					propKey, j, &error);
				if(error)
					elementString = "<error>";
				else if(hint == dtUtf8)
				{
					const char * element = m_cpVSAPI->mapGetData(cpProps,
						propKey, j, &error);
					if(error)
						elementString = "<error>";
					else
						elementString = QString::fromUtf8(element);
				}
				else if(hint == dtBinary)
				{
					int len = m_cpVSAPI->mapGetDataSize(cpProps,
						propKey, j, &error);
					if(error)
						elementString = "<error>";
					else
						elementString = QString("<binary with %1 bytes>")
							.arg(len);
				}
				else
				{
					const char * element = m_cpVSAPI->mapGetData(cpProps,
						propKey, j, &error);
					if(error)
						elementString = "<error>";
					else
					{
						elementString = QString::fromUtf8(element);
						QByteArray elementAsUtf8 = elementString.toUtf8();
						int len = m_cpVSAPI->mapGetDataSize(cpProps,
							propKey, j, &error);
						if(elementAsUtf8.length() != len ||
							elementAsUtf8.toStdString() != element)
							elementString = "<unknown type>";
					}
				}

				elementStringList += elementString;
			}
			currentPropString += elementStringList.join(", ");
			break;
		}
		default:
			Q_ASSERT(false);
		}

		propsStringList += currentPropString;
	}

	propsString = propsStringList.join(" \n");

	return propsString;
}

// END OF QString VapourSynthScriptProcessor::framePropsString(
//		const VSFrame * a_cpFrame) const
//==============================================================================

bool VapourSynthScriptProcessor::clearCoreCaches()
{
	return m_pVSScriptLibrary->clearCoreCaches(m_pCore);
}
