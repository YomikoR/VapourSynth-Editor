#include <cassert>

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
	, m_pConvertNode(nullptr)
	, m_cpVideoInfo(nullptr)
	, m_currentFrame(0)
	, m_cpCurrentFrameRef(nullptr)
{

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

	m_pOutputNode = vsscript_getOutput(m_pVSScript, 0);
	if(!m_pOutputNode)
	{
		m_error = trUtf8("Failed to get the script output node.");
		emit signalWriteLogMessage(mtCritical, m_error);
		finalize();
    	return false;
	}

	m_cpVideoInfo = m_cpVSAPI->getVideoInfo(m_pOutputNode);

	//--------------------------------------------------------------------------
	// Create another node that converts the output into BGR32 to easily get
	// the preview image. Note that frame will be flipped vertically

	VSCore * pVSCore = vsscript_getCore(m_pVSScript);

	// Conversion is done by standard "resize" plugin.
	VSPlugin * pResizePlugin = m_cpVSAPI->getPluginByNs("resize", pVSCore);

	// Preparing arguments map to call plugin function with.
	VSMap * pResizeArguments = m_cpVSAPI->createMap();
	m_cpVSAPI->propSetNode(pResizeArguments, "clip", m_pOutputNode, paReplace);
	m_cpVSAPI->propSetInt(pResizeArguments, "format", pfCompatBGR32, paReplace);

	// Calling the plugin function "Bicubic". The result is a map containing
	// new node.
	VSMap * pResizeRet = m_cpVSAPI->invoke(pResizePlugin, "Bicubic",
		pResizeArguments);

	m_cpVSAPI->freeMap(pResizeArguments);

	const char * resizeError = m_cpVSAPI->getError(pResizeRet);
	if(resizeError)
	{
		m_error = trUtf8("Failed to convert the script's output to "
			"COMPATBGR32:\n");
		m_error += QString::fromUtf8(resizeError);
		emit signalWriteLogMessage(mtCritical, m_error);

		m_cpVSAPI->freeMap(pResizeRet);
		finalize();
		return false;
	}

	m_pConvertNode = m_cpVSAPI->propGetNode(pResizeRet, "clip", 0, nullptr);
	if(!m_pConvertNode)
	{
		m_error = trUtf8("Failed to convert the script's output to "
			"COMPATBGR32:\nresize.Bicubic() did not return a valid clip.");
		emit signalWriteLogMessage(mtCritical, m_error);

		m_cpVSAPI->freeMap(pResizeRet);
		finalize();
		return false;
	}

	m_cpVSAPI->freeMap(pResizeRet);
	//--------------------------------------------------------------------------

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

	if(m_pConvertNode)
	{
		m_cpVSAPI->freeNode(m_pConvertNode);
		m_pConvertNode = nullptr;
	}

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

	assert(m_pConvertNode);
	assert(m_cpVSAPI);

	char getFrameErrorMessage[1024] = {0};

	const VSFrameRef * cpFrameRef = m_cpVSAPI->getFrame(a_frameNumber,
		m_pConvertNode, getFrameErrorMessage, sizeof(getFrameErrorMessage) - 1);

	if (!cpFrameRef)
	{
		m_error = trUtf8("Error getting the frame number %1:\n%2")
			.arg(a_frameNumber).arg(QString::fromUtf8(getFrameErrorMessage));
		emit signalWriteLogMessage(mtCritical, m_error);
		return QPixmap();
	}

	int frameWidth = m_cpVSAPI->getFrameWidth(cpFrameRef, 0);
	int frameHeight = m_cpVSAPI->getFrameHeight(cpFrameRef, 0);
	int stride = m_cpVSAPI->getStride(cpFrameRef, 0);
	const uint8_t * pFrameReadPointer =
		m_cpVSAPI->getReadPtr(cpFrameRef, 0);

	// Creating QImage from memory doesn't copy or modify the data.
	QImage frameImage(pFrameReadPointer, frameWidth, frameHeight, stride,
		QImage::Format_RGB32);
	// But when creating the pixmap - we use mirrored copy.
	QPixmap framePixmap = QPixmap::fromImage(frameImage.mirrored());

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
