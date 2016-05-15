#include "vapoursynthscriptprocessor.h"

#include "../common/helpers.h"

#include <cassert>
#include <vector>
#include <cmath>

#include <QImage>
#include <QSettings>
#include <QProcessEnvironment>

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
	, m_vsScriptLibrary(this)
{
	initLibrary();
	slotSettingsChanged();
}

// END OF VapourSynthScriptProcessor::VapourSynthScriptProcessor(
//		QObject * a_pParent)
//==============================================================================

VapourSynthScriptProcessor::~VapourSynthScriptProcessor()
{
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

	if(m_cpVSAPI->getCoreInfo(vssGetCore(m_pVSScript))->core < 29)
	{
		m_error = trUtf8("VapourSynth R29+ required for preview.");
		emit signalWriteLogMessage(mtCritical, m_error);
		finalize();
		return false;
	}

	m_pOutputNode = vssGetOutput(m_pVSScript, 0);
	if(!m_pOutputNode)
	{
		m_error = trUtf8("Failed to get the script output node.");
		emit signalWriteLogMessage(mtCritical, m_error);
		finalize();
		return false;
	}

	m_cpVideoInfo = m_cpVSAPI->getVideoInfo(m_pOutputNode);

	initPreviewNode();
	if(!m_pPreviewNode)
	{
		finalize();
		return false;
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

	if(m_pPreviewNode)
	{
		m_cpVSAPI->freeNode(m_pPreviewNode);
		m_pPreviewNode = nullptr;
	}

	if(m_pOutputNode)
	{
		m_cpVSAPI->freeNode(m_pOutputNode);
		m_pOutputNode = nullptr;
	}

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

	if(!cpNewFrameRef)
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

	// Pixmap will likely be used for preview, so current frame, referenced
	// from the output node, must be of the same number with the frame
	// from the preview node. That ensures the work of color picker and such.
	requestFrame(a_frameNumber);

	assert(m_pPreviewNode);
	assert(m_cpVSAPI);

	char getFrameErrorMessage[1024] = {0};

	const VSFrameRef * cpFrameRef = m_cpVSAPI->getFrame(a_frameNumber,
		m_pPreviewNode, getFrameErrorMessage, sizeof(getFrameErrorMessage) - 1);

	if(!cpFrameRef)
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

void VapourSynthScriptProcessor::colorAtPoint(size_t a_x, size_t a_y,
	double & a_rValue1, double & a_rValue2, double & a_rValue3)
{
	if(!m_cpCurrentFrameRef)
		requestFrame(m_currentFrame);

	if(!m_cpCurrentFrameRef)
		return;

	assert(m_cpVSAPI);

	int width = m_cpVSAPI->getFrameWidth(m_cpCurrentFrameRef, 0);
	int height = m_cpVSAPI->getFrameHeight(m_cpCurrentFrameRef, 0);
	const VSFormat * cpFormat = m_cpVSAPI->getFrameFormat(m_cpCurrentFrameRef);

	if((a_x >= (size_t)width) || (a_y >= (size_t)height))
		return;

	if(cpFormat->id == pfCompatBGR32)
	{
		const uint8_t * cpData = m_cpVSAPI->getReadPtr(m_cpCurrentFrameRef, 0);
		int stride = m_cpVSAPI->getStride(m_cpCurrentFrameRef, 0);
		const uint32_t * cpLine = (const uint32_t *)(cpData + a_y * stride);
		uint32_t packedValue = cpLine[a_x];
		a_rValue3 = (double)(packedValue & 0xFF);
		a_rValue2 = (double)((packedValue >> 8) & 0xFF);
		a_rValue1 = (double)((packedValue >> 16) & 0xFF);
		return;
	}
	else if(cpFormat->id == pfCompatYUY2)
	{
		size_t x = a_x >> 1;
		size_t rem = a_x & 0x1;
		const uint8_t * cpData = m_cpVSAPI->getReadPtr(m_cpCurrentFrameRef, 0);
		int stride = m_cpVSAPI->getStride(m_cpCurrentFrameRef, 0);
		const uint32_t * cpLine = (const uint32_t *)(cpData + a_y * stride);
		uint32_t packedValue = cpLine[x];

		if(rem == 0)
			a_rValue1 = (double)(packedValue & 0xFF);
		else
			a_rValue1 = (double)((packedValue >> 16) & 0xFF);
		a_rValue2 = (double)((packedValue >> 8) & 0xFF);
		a_rValue3 = (double)((packedValue >> 24) & 0xFF);
		return;
	}

	a_rValue1 = valueAtPoint(a_x, a_y, 0);
	if(cpFormat->numPlanes > 1)
		a_rValue2 = valueAtPoint(a_x, a_y, 1);
	if(cpFormat->numPlanes > 2)
		a_rValue3 = valueAtPoint(a_x, a_y, 2);
}

// END OF void VapourSynthScriptProcessor::colorAtPoint(size_t a_x, size_t a_y,
//		 double & a_rValue1, double & a_rValue2, double & a_rValue3)
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

	initPreviewNode();
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
	int stride = m_cpVSAPI->getStride(a_cpFrameRef, 0);

	QImage frameImage(static_cast<const uchar *>(pData), width, height,
		stride, QImage::Format_RGB32);

	QImage flippedImage = frameImage.mirrored();
	QPixmap framePixmap = QPixmap::fromImage(std::move(flippedImage));

	return framePixmap;
}

// END OF QPixmap VapourSynthScriptProcessor::pixmapFromFrame(
//		const VSFrameRef * a_cpFrameRef)
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

double VapourSynthScriptProcessor::valueAtPoint(size_t a_x, size_t a_y,
	int a_plane)
{
	assert(m_cpCurrentFrameRef);
	assert(m_cpVSAPI);

	const VSFormat * cpFormat = m_cpVSAPI->getFrameFormat(m_cpCurrentFrameRef);

	assert((a_plane >= 0) && (a_plane < cpFormat->numPlanes));

    const uint8_t * cpPlane =
		m_cpVSAPI->getReadPtr(m_cpCurrentFrameRef, a_plane);

	size_t x = a_x;
	size_t y = a_y;

	if(a_plane != 0)
	{
		x = (a_x >> cpFormat->subSamplingW);
		y = (a_y >> cpFormat->subSamplingH);
	}
	int stride = m_cpVSAPI->getStride(m_cpCurrentFrameRef, a_plane);
	const uint8_t * cpLine = cpPlane + y * stride;

	double value = 0.0;

	if(cpFormat->sampleType == stInteger)
	{
		if(cpFormat->bytesPerSample == 1)
			value = (double)cpLine[x];
		else if(cpFormat->bytesPerSample == 2)
			value = (double)((uint16_t *)cpLine)[x];
		else if(cpFormat->bytesPerSample == 4)
			value = (double)((uint32_t *)cpLine)[x];
	}
	else if(cpFormat->sampleType == stFloat)
	{
		if(cpFormat->bytesPerSample == 2)
		{
			vsedit::FP16 half;
			half.u = ((uint16_t *)cpLine)[x];
			vsedit::FP32 single = vsedit::halfToSingle(half);
			value = (double)single.f;
		}
		else if(cpFormat->bytesPerSample == 4)
			value = (double)((float *)cpLine)[x];
	}

	return value;
}

// END OF double VapourSynthScriptProcessor::valueAtPoint(size_t a_x,
//		size_t a_y, int a_plane)
//==============================================================================

void VapourSynthScriptProcessor::initPreviewNode()
{
	if(!m_cpVSAPI)
	{
		// Function was called while VapourSynth was not initialized.
		return;
	}

	if(m_pPreviewNode)
	{
		m_cpVSAPI->freeNode(m_pPreviewNode);
		m_pPreviewNode = nullptr;
	}

	if(!m_pOutputNode)
	{
		m_error = trUtf8("Failed to create preview node: "
			"there is no output node.\n");
		emit signalWriteLogMessage(mtCritical, m_error);
		return;
	}

	assert(m_cpVideoInfo);
	const VSFormat * cpFormat = m_cpVideoInfo->format;

	if(cpFormat->id == pfCompatBGR32)
	{
		m_pPreviewNode = m_cpVSAPI->cloneNodeRef(m_pOutputNode);
		return;
	}

	VSCore * pCore = vssGetCore(m_pVSScript);
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
			matrixInS = "470bg";
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

	const char * cpResultError = m_cpVSAPI->getError(pResultMap);

	if(cpResultError)
	{
		m_error = trUtf8("Failed to convert to RGB:\n");
		m_error += cpResultError;
		emit signalWriteLogMessage(mtCritical, m_error);
		m_cpVSAPI->freeMap(pResultMap);
		return;
	}

	VSNodeRef * pPreviewNode = m_cpVSAPI->propGetNode(pResultMap, "clip", 0,
		nullptr);
	assert(pPreviewNode);
	m_pPreviewNode = pPreviewNode;

	m_cpVSAPI->freeMap(pResultMap);
}

// END OF void VapourSynthScriptProcessor::initPreviewNode()
//==============================================================================
