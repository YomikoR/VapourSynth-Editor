#include "vapoursynthscriptprocessor.h"

#include <cassert>
#include <vector>
#include <cmath>

#include <QImage>

//==============================================================================

void VS_CC vsMessageHandler(int a_msgType, const char * a_message,
	void * a_pUserData)
{
	VapourSynthScriptProcessor * scriptProcessor =
		static_cast<VapourSynthScriptProcessor *>(a_pUserData);
	scriptProcessor->handleVSMessage(a_msgType, a_message);
}

// END OF void VS_CC vsMessageHandler(int a_msgType, const char * a_message,
//	void * a_pUserData)
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
	, m_pOutputNode(nullptr)
	, m_pPreviewNode(nullptr)
	, m_cpVideoInfo(nullptr)
	, m_currentFrame(0)
	, m_cpCurrentFrameRef(nullptr)
	, m_chromaResamplingFilter()
	, m_chromaPlacement()
	, m_resamplingFilterParameterA(NAN)
	, m_resamplingFilterParameterB(NAN)
	, m_yuvMatrix()
{
	slotSettingsChanged();
}

// END OF VapourSynthScriptProcessor::VapourSynthScriptProcessor(
//		QObject * a_pParent)
//==============================================================================

VapourSynthScriptProcessor::~VapourSynthScriptProcessor()
{
	if(m_initialized)
		finalize();
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

	int opresult = vsscript_init();
	if(!opresult)
	{
		m_error = trUtf8("Failed to initialize VapourSynth");
		emit signalWriteLogMessage(mtCritical, m_error);
		return false;
	}
	m_vsScriptInitialized = true;

	m_cpVSAPI = vsscript_getVSApi();
	if(!m_cpVSAPI)
	{
		m_error = trUtf8("Failed to get VapourSynth API!");
		emit signalWriteLogMessage(mtCritical, m_error);
		finalize();
		return false;
	}

	m_cpVSAPI->setMessageHandler(::vsMessageHandler,
		static_cast<void *>(this));

	opresult = vsscript_evaluateScript(&m_pVSScript,
		a_script.toUtf8().constData(), a_scriptName.toUtf8().constData(),
		efSetWorkingDir);

	if(opresult)
	{
		m_error = trUtf8("Failed to evaluate the script");
		const char * vsError = vsscript_getError(m_pVSScript);
		if(vsError)
			m_error += QString(":\n") + vsError;
		else
			m_error += '.';

		emit signalWriteLogMessage(mtCritical, m_error);
		finalize();
		return false;
	}

	if(m_cpVSAPI->getCoreInfo(vsscript_getCore(m_pVSScript))->core < 29)
	{
		m_error = trUtf8("VapourSynth R29+ required for preview.");
		emit signalWriteLogMessage(mtCritical, m_error);
		finalize();
		return false;
	}

	m_pOutputNode = vsscript_getOutput(m_pVSScript, 0);
	if(!m_pOutputNode)
	{
		m_error = trUtf8("Failed to get the script output node.");
		emit signalWriteLogMessage(mtCritical, m_error);
		finalize();
		return false;
	}

	m_cpVideoInfo = m_cpVSAPI->getVideoInfo(m_pOutputNode);
	const VSFormat * cpFormat = m_cpVideoInfo->format;

	if(!cpFormat || cpFormat->id != pfCompatBGR32)
	{
		VSCore * pCore = vsscript_getCore(m_pVSScript);
		VSPlugin * pResizePlugin = m_cpVSAPI->getPluginById(
			"com.vapoursynth.resize", pCore);
		const char * resizeName = nullptr;

		VSMap * pArgumentMap = m_cpVSAPI->createMap();
		m_cpVSAPI->propSetNode(pArgumentMap, "clip", m_pOutputNode, paReplace);
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

		if(cpFormat->colorFamily == cmYUV)
		{
			const char * matrixInS = nullptr;
			switch(m_yuvMatrix)
			{
			case YuvToRgbConversionMatrix::Bt601:
				matrixInS = "601";
				break;
			case YuvToRgbConversionMatrix::Bt709:
				matrixInS = "709";
				break;
			default:
				assert(false);
			}
			int matrixStringLength = (int)strlen(matrixInS);
			m_cpVSAPI->propSetData(pArgumentMap, "matrix_in_s",
				matrixInS, matrixStringLength, paReplace);

			int64_t chromaLoc = 0;
			switch(m_chromaPlacement)
			{
			case ChromaPlacement::MPEG1:
				chromaLoc = 1;
				break;
			case ChromaPlacement::MPEG2:
				chromaLoc = 0;
				break;
			default:
				assert(false);
			}
			m_cpVSAPI->propSetInt(pArgumentMap, "chromaloc",
				chromaLoc, paReplace);
		}

		VSMap * pResultMap = m_cpVSAPI->invoke(pResizePlugin, resizeName,
			pArgumentMap);

		m_cpVSAPI->freeMap(pArgumentMap);

		if (const char * pResultError = m_cpVSAPI->getError(pResultMap))
		{
			m_error = trUtf8("Failed to convert to RGB:\n");
			m_error += pResultError;
			emit signalWriteLogMessage(mtCritical, m_error);
			finalize();
			return false;
		}

		VSNodeRef * pPreviewNode = m_cpVSAPI->propGetNode(pResultMap, "clip", 0,
			nullptr);
		assert(pPreviewNode);
		m_pPreviewNode = pPreviewNode;
	}
	else
	{
		m_pPreviewNode = m_cpVSAPI->cloneNodeRef(m_pOutputNode);
	}

	m_currentFrame = 0;
	m_error.clear();
	m_initialized = true;
	return true;
}

// END OF bool VapourSynthScriptProcessor::initialize(const QString& a_script,
//		const QString& a_scriptName)
//==============================================================================

void VapourSynthScriptProcessor::finalize()
{
	freeFrame();

	m_cpVideoInfo = nullptr;

	if(m_pOutputNode)
	{
		m_cpVSAPI->freeNode(m_pOutputNode);
		m_pOutputNode = nullptr;
	}

	if(m_pPreviewNode)
	{
		m_cpVSAPI->freeNode(m_pPreviewNode);
		m_pPreviewNode = nullptr;
	}

	if(m_pVSScript)
	{
		vsscript_freeScript(m_pVSScript);
		m_pVSScript = nullptr;
	}

	m_cpVSAPI = nullptr;

	if(m_vsScriptInitialized)
	{
		vsscript_finalize();
		m_vsScriptInitialized = false;
	}

	m_error.clear();
	m_initialized = false;
}

// END OF void VapourSynthScriptProcessor::finalize()
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

const VSVideoInfo * VapourSynthScriptProcessor::videoInfo()
{
	if(!m_initialized)
		return nullptr;

	assert(m_cpVideoInfo);
	return m_cpVideoInfo;
}

// END OF const VSVideoInfo * VapourSynthScriptProcessor::videoInfo()
//==============================================================================

bool VapourSynthScriptProcessor::requestFrame(int a_frameNumber)
{
	if(!m_initialized)
		return false;

	assert(m_pPreviewNode);
	assert(m_cpVSAPI);

	char getFrameErrorMessage[1024] = {0};

	const VSFrameRef * cpNewFrameRef = m_cpVSAPI->getFrame(a_frameNumber,
		m_pPreviewNode, getFrameErrorMessage, sizeof(getFrameErrorMessage) - 1);

	if (!cpNewFrameRef)
	{
		m_error = trUtf8("Error getting the frame number %1:\n%2")
			.arg(a_frameNumber).arg(QString::fromUtf8(getFrameErrorMessage));
		emit signalWriteLogMessage(mtCritical, m_error);
		return false;
	}

	if(m_cpCurrentFrameRef)
		m_cpVSAPI->freeFrame(m_cpCurrentFrameRef);

	m_cpCurrentFrameRef = cpNewFrameRef;
	m_currentFrame = a_frameNumber;
	m_error.clear();
	return true;
}

// END OF bool VapourSynthScriptProcessor::requestFrame(int a_frameNumber)
//==============================================================================

void VapourSynthScriptProcessor::freeFrame()
{
	if(!m_cpCurrentFrameRef)
		return;

	m_cpVSAPI->freeFrame(m_cpCurrentFrameRef);
	m_cpCurrentFrameRef = nullptr;
}

// END OF void VapourSynthScriptProcessor::freeFrame()
//==============================================================================

QPixmap VapourSynthScriptProcessor::pixmap(int a_frameNumber)
{
	if(!m_initialized)
		return QPixmap();

	assert(m_pOutputNode);
	assert(m_cpVSAPI);

	char getFrameErrorMessage[1024] = {0};

	const VSFrameRef * cpFrameRef = m_cpVSAPI->getFrame(a_frameNumber,
		m_pPreviewNode, getFrameErrorMessage, sizeof(getFrameErrorMessage) - 1);

	if (!cpFrameRef)
	{
		m_error = trUtf8("Error getting the frame number %1:\n%2")
			.arg(a_frameNumber).arg(QString::fromUtf8(getFrameErrorMessage));
		emit signalWriteLogMessage(mtCritical, m_error);
		return QPixmap();
	}

	QPixmap framePixmap = pixmapFromFrame(cpFrameRef);

	if(framePixmap.isNull())
	{
		m_error = trUtf8("Can not convert from format \"%1\" for preview!")
			.arg(m_cpVideoInfo->format->name);
		emit signalWriteLogMessage(mtCritical, m_error);
	}

	m_cpVSAPI->freeFrame(cpFrameRef);

	return framePixmap;
}

// END OF QPixmap VapourSynthScriptProcessor::pixmap()
//==============================================================================

void VapourSynthScriptProcessor::slotSettingsChanged()
{
	m_yuvMatrix = m_pSettingsManager->getYuvToRgbConversionMatrix();

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
}

// END OF void VapourSynthScriptProcessor::slotSettingsChanged()
//==============================================================================

void VapourSynthScriptProcessor::handleVSMessage(int a_messageType,
	const QString & a_message)
{
	emit signalWriteLogMessage(a_messageType, a_message);
}

// END OF void VapourSynthScriptProcessor::handleVSMessage(int a_messageType,
//		const QString & a_message)
//==============================================================================

QPixmap VapourSynthScriptProcessor::pixmapFromFrame(
	const VSFrameRef * a_cpFrameRef)
{
	const VSFormat * cpFormat = m_cpVSAPI->getFrameFormat(a_cpFrameRef);
	assert(cpFormat->id == pfCompatBGR32);

	const void * pData = m_cpVSAPI->getReadPtr(a_cpFrameRef, 0);
	int width = m_cpVSAPI->getFrameWidth(a_cpFrameRef, 0);
	int height = m_cpVSAPI->getFrameHeight(a_cpFrameRef, 0);

	QImage frameImage(static_cast<const uchar *>(pData), width, height,
		QImage::Format_RGB32);

	QImage flippedImage = frameImage.mirrored();
	QPixmap framePixmap = QPixmap::fromImage(std::move(flippedImage));

	return framePixmap;
}

// END OF QPixmap VapourSynthScriptProcessor::pixmapFromFrame(
//		const VSFrameRef * a_cpFrameRef)
//==============================================================================
