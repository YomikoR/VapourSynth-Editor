#include <cassert>
#include <vector>

#include "vapoursynthscriptprocessor.h"

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

VapourSynthScriptProcessor::VapourSynthScriptProcessor(QObject * a_pParent):
	QObject(a_pParent)
	, m_script()
	, m_scriptName()
	, m_error()
	, m_vsScriptInitialized(false)
	, m_initialized(false)
	, m_cpVSAPI(nullptr)
	, m_pVSScript(nullptr)
	, m_pOutputNode(nullptr)
	, m_cpVideoInfo(nullptr)
	, m_currentFrame(0)
	, m_cpCurrentFrameRef(nullptr)
	, m_pResampleLinearFilter(nullptr)
{
	double a = 1.0 / 3.0;
	m_pResampleLinearFilter = new vsedit::CubicResampleFilter(a, a);
}

// END OF VapourSynthScriptProcessor::VapourSynthScriptProcessor(
//		QObject * a_pParent)
//==============================================================================

VapourSynthScriptProcessor::~VapourSynthScriptProcessor()
{
	if(m_initialized)
		finalize();

	if(m_pResampleLinearFilter)
		delete(m_pResampleLinearFilter);
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

	m_pOutputNode = vsscript_getOutput(m_pVSScript, 0);
	if(!m_pOutputNode)
	{
		m_error = trUtf8("Failed to get the script output node.");
		emit signalWriteLogMessage(mtCritical, m_error);
		finalize();
    	return false;
	}

	m_cpVideoInfo = m_cpVSAPI->getVideoInfo(m_pOutputNode);

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

	assert(m_pOutputNode);
	assert(m_cpVSAPI);

	char getFrameErrorMessage[1024] = {0};

	const VSFrameRef * cpNewFrameRef = m_cpVSAPI->getFrame(a_frameNumber,
		m_pOutputNode, getFrameErrorMessage, sizeof(getFrameErrorMessage) - 1);

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
		m_pOutputNode, getFrameErrorMessage, sizeof(getFrameErrorMessage) - 1);

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
	assert(m_cpVideoInfo);

	const VSFormat * cpFormat = m_cpVideoInfo->format;

	if(cpFormat->id == pfCompatBGR32)
		return pixmapFromCompatBGR32(a_cpFrameRef);
	else if(cpFormat->id == pfCompatYUY2)
		return pixmapFromCompatYUY2(a_cpFrameRef);
	else if(cpFormat->id == pfGray8)
		return pixmapFromGray1B(a_cpFrameRef);
	else if(cpFormat->id == pfGray16)
		return pixmapFromGray2B(a_cpFrameRef);
	else if(cpFormat->id == pfGrayH)
		return pixmapFromGrayH(a_cpFrameRef);
	else if(cpFormat->id == pfGrayS)
		return pixmapFromGrayS(a_cpFrameRef);
	else if((cpFormat->colorFamily == cmYUV) &&
		(cpFormat->sampleType == stInteger) &&
		(cpFormat->bytesPerSample == 1))
		return pixmapFromYUV1B(a_cpFrameRef);
	else if((cpFormat->colorFamily == cmYUV) &&
		(cpFormat->sampleType == stInteger) &&
		(cpFormat->bytesPerSample == 2))
		return pixmapFromYUV2B(a_cpFrameRef);
	else if((cpFormat->colorFamily == cmYUV) &&
		(cpFormat->sampleType == stFloat) &&
		(cpFormat->bytesPerSample == 2))
		return pixmapFromYUVH(a_cpFrameRef);
	else if((cpFormat->colorFamily == cmYUV) &&
		(cpFormat->sampleType == stFloat) &&
		(cpFormat->bytesPerSample == 4))
		return pixmapFromYUVS(a_cpFrameRef);
	else if((cpFormat->colorFamily == cmRGB) &&
		(cpFormat->sampleType == stInteger) &&
		(cpFormat->bytesPerSample == 1))
		return pixmapFromRGB1B(a_cpFrameRef);
	else if((cpFormat->colorFamily == cmRGB) &&
		(cpFormat->sampleType == stInteger) &&
		(cpFormat->bytesPerSample == 2))
		return pixmapFromRGB2B(a_cpFrameRef);
	else if((cpFormat->colorFamily == cmRGB) &&
		(cpFormat->sampleType == stFloat) &&
		(cpFormat->bytesPerSample == 2))
		return pixmapFromRGBH(a_cpFrameRef);
	else if((cpFormat->colorFamily == cmRGB) &&
		(cpFormat->sampleType == stFloat) &&
		(cpFormat->bytesPerSample == 4))
		return pixmapFromRGBS(a_cpFrameRef);
	else
		return QPixmap();
}

// END OF QPixmap VapourSynthScriptProcessor::pixmapFromFrame(
//		const VSFrameRef * a_cpFrameRef)
//==============================================================================
