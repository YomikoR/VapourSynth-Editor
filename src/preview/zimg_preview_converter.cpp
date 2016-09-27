#include "zimg_preview_converter.h"

#include "../settings/settingsmanager.h"
#include "../common/helpers.h"
#include "zimg_helpers.h"

#include <QImage>
#include <QPainter>
#include <map>
#include <cassert>

#if ZIMG_API_VERSION < ZIMG_MAKE_API_VERSION(2, 2)
#error zAPI v2 or greater required
#endif

//==============================================================================

ZimgPreviewConverter::ZimgPreviewConverter(SettingsManager * a_pSettingsManager,
	QObject * a_pParent, const VSAPI * a_cpVSAPI):
	  QObject(a_pParent)
	, m_pSettingsManager(a_pSettingsManager)
	, m_cpVSAPI(a_cpVSAPI)
	, m_fpUnpackCallback(nullptr)
	, m_pUnpackData(nullptr)
	, m_pFilterGraph(nullptr)
{
	assert(a_pSettingsManager);

	zimg_image_format_default(&m_inFormat, ZIMG_API_VERSION);

	zimg_image_format_default(&m_outFormat, ZIMG_API_VERSION);
	m_outFormat.color_family = ZIMG_COLOR_RGB;
	m_outFormat.matrix_coefficients = ZIMG_MATRIX_RGB;
	m_outFormat.pixel_type = ZIMG_PIXEL_BYTE;
	m_outFormat.pixel_range = ZIMG_RANGE_FULL;
	m_outFormat.depth = 8;

	m_inZimgBuffer.plane[0].mask = ZIMG_BUFFER_MAX;
	m_inZimgBuffer.plane[1].mask = ZIMG_BUFFER_MAX;
	m_inZimgBuffer.plane[2].mask = ZIMG_BUFFER_MAX;
	m_outZimgBuffer.plane[0].mask = ZIMG_BUFFER_MAX;
	m_outZimgBuffer.plane[1].mask = ZIMG_BUFFER_MAX;
	m_outZimgBuffer.plane[2].mask = ZIMG_BUFFER_MAX;

	m_yuy2UnpackData.pZimgBuffer = &m_inZimgBuffer;
	m_bgr32PackData.cpZimgBuffer =
		reinterpret_cast<zimg_image_buffer_const *>(&m_outZimgBuffer);

	zimg_graph_builder_params_default(&m_graphBuilderParams, ZIMG_API_VERSION);

	slotResetSettings();
}

// END OF ZimgPreviewConverter::ZimgPreviewConverter(
//		SettingsManager * a_pSettingsManager, QObject * a_pParent,
//		const VSAPI * a_cpVSAPI)
//==============================================================================

ZimgPreviewConverter::~ZimgPreviewConverter()
{
	if(m_pFilterGraph)
	{
		zimg_filter_graph_free(m_pFilterGraph);
		m_pFilterGraph = nullptr;
	}
}

// END OF ZimgPreviewConverter::~ZimgPreviewConverter()
//==============================================================================

void ZimgPreviewConverter::setVSAPI(const VSAPI * a_cpVSAPI)
{
	m_cpVSAPI = a_cpVSAPI;
}

// END OF void ZimgPreviewConverter::setVSAPI(const VSAPI * a_cpVSAPI)
//==============================================================================

QPixmap ZimgPreviewConverter::pixmap(const VSFrameRef * a_cpFrameRef)
{
	if((!m_cpVSAPI) || (!a_cpFrameRef))
		return QPixmap();

	unsigned width = (unsigned)m_cpVSAPI->getFrameWidth(a_cpFrameRef, 0);
	unsigned height = (unsigned)m_cpVSAPI->getFrameHeight(a_cpFrameRef, 0);

	const VSFormat * cpFormat = m_cpVSAPI->getFrameFormat(a_cpFrameRef);

	if(cpFormat->id == pfCompatBGR32)
	{
		const void * pData = m_cpVSAPI->getReadPtr(a_cpFrameRef, 0);
		int stride = m_cpVSAPI->getStride(a_cpFrameRef, 0);
		QImage frameImage((const uchar *)pData, width, height,
			stride, QImage::Format_RGB32);
		QImage flippedImage = frameImage.mirrored();
		QPixmap framePixmap = QPixmap::fromImage(flippedImage);
		return framePixmap;
	}

	QPixmap pixmap(width, height);

	zimg_image_format newInFormat;
	zimg_image_format_default(&newInFormat, ZIMG_API_VERSION);
	newInFormat.width = width;
	newInFormat.height = height;

	bool isInterlaced = false;

	bool result = translateFormat(cpFormat, &newInFormat);

	if(result)
	{
		const VSMap * cpProps = m_cpVSAPI->getFramePropsRO(a_cpFrameRef);
		result = importFrameProps(cpProps, &newInFormat, &isInterlaced);
	}

	if(!result)
	{
		inpaintError(pixmap);
		return pixmap;
	}

	if((newInFormat.color_family == ZIMG_COLOR_YUV) &&
		(newInFormat.matrix_coefficients == ZIMG_MATRIX_UNSPECIFIED))
	{
		newInFormat.matrix_coefficients = ZIMG_MATRIX_709;
	}

	if(cpFormat->id == pfCompatYUY2)
	{
		m_fpUnpackCallback = unpackYUY2;
		ptrdiff_t strideY = vsedit::roundUp(width, VSE_MEMORY_ALIGNMENT);
		size_t dataSizeY = strideY * height;
		ptrdiff_t strideUV = vsedit::roundUp(width / 2, VSE_MEMORY_ALIGNMENT);
		size_t dataSizeUV = strideUV * height;

		m_inBuffer.resize(dataSizeY + dataSizeUV + dataSizeUV);
		m_inZimgBuffer.plane[0].data = m_inBuffer.data();
		m_inZimgBuffer.plane[0].stride = strideY;
		m_inZimgBuffer.plane[1].data = m_inBuffer.data() + dataSizeY;
		m_inZimgBuffer.plane[1].stride = strideUV;
		m_inZimgBuffer.plane[2].data = m_inBuffer.data() +
			dataSizeY + dataSizeUV;
		m_inZimgBuffer.plane[2].stride = strideUV;

		m_yuy2UnpackData.pYUY2Buffer =
			m_cpVSAPI->getReadPtr(a_cpFrameRef, 0);
		m_yuy2UnpackData.yuy2BufferStride =
			m_cpVSAPI->getStride(a_cpFrameRef, 0);

		m_pUnpackData = &m_yuy2UnpackData;
	}
	else
	{
		m_fpUnpackCallback = nullptr;
		m_pUnpackData = nullptr;
		m_inBuffer.clear();

		m_inZimgBuffer.plane[0].data =
			(void *)m_cpVSAPI->getReadPtr(a_cpFrameRef, 0);
		m_inZimgBuffer.plane[0].stride = m_cpVSAPI->getStride(a_cpFrameRef, 0);
		m_inZimgBuffer.plane[1].data =
			(void *)m_cpVSAPI->getReadPtr(a_cpFrameRef, 1);
		m_inZimgBuffer.plane[1].stride = m_cpVSAPI->getStride(a_cpFrameRef, 1);
		m_inZimgBuffer.plane[2].data =
			(void *)m_cpVSAPI->getReadPtr(a_cpFrameRef, 2);
		m_inZimgBuffer.plane[2].stride = m_cpVSAPI->getStride(a_cpFrameRef, 2);
	}

	ptrdiff_t stride = vsedit::roundUp(width, VSE_MEMORY_ALIGNMENT);
	size_t planeSize = stride * height;
	m_outBuffer.resize(planeSize * 3);
	m_outZimgBuffer.plane[0].data = m_outBuffer.data();
	m_outZimgBuffer.plane[0].stride = stride;
	m_outZimgBuffer.plane[1].data = m_outBuffer.data() + planeSize;
	m_outZimgBuffer.plane[1].stride = stride;
	m_outZimgBuffer.plane[2].data = m_outBuffer.data() +
		planeSize + planeSize;
	m_outZimgBuffer.plane[2].stride = stride;

	ptrdiff_t packStride = width * 4;
	m_bgr32Buffer.resize(packStride * height, 255);
	m_bgr32PackData.pBGR32Buffer = m_bgr32Buffer.data();
	m_bgr32PackData.bgr32BufferStride = packStride;

	if((m_inFormat != newInFormat) || (m_outFormat.width != width) ||
		(m_outFormat.height != height) || (!m_pFilterGraph))
	{
		m_inFormat = newInFormat;

		m_outFormat.width = width;
		m_outFormat.height = height;

		if(m_pFilterGraph)
			zimg_filter_graph_free(m_pFilterGraph);

		m_pFilterGraph = zimg_filter_graph_build(&m_inFormat, &m_outFormat,
			&m_graphBuilderParams);
		if(!m_pFilterGraph)
		{
			checkZimgError();
			inpaintError(pixmap);
			return pixmap;
		}
	}

	size_t tempBufferSize;
	zimg_error_code_e errorCode = zimg_filter_graph_get_tmp_size(
		m_pFilterGraph, &tempBufferSize);
	if(errorCode != ZIMG_ERROR_SUCCESS)
	{
		checkZimgError();
		inpaintError(pixmap);
		return pixmap;
	}
	m_tempBuffer.resize(tempBufferSize);

	errorCode = zimg_filter_graph_process(m_pFilterGraph,
		reinterpret_cast<zimg_image_buffer_const *>(&m_inZimgBuffer),
		&m_outZimgBuffer, m_tempBuffer.data(), m_fpUnpackCallback,
		m_pUnpackData, packBGR32, &m_bgr32PackData);
	if(errorCode != ZIMG_ERROR_SUCCESS)
	{
		checkZimgError();
		inpaintError(pixmap);
		return pixmap;
	}

	QImage frameImage(m_bgr32Buffer.data(), width, height,
		width * 4, QImage::Format_RGB32);
	pixmap = QPixmap::fromImage(frameImage);

	return pixmap;
}

// END OF QPixmap ZimgPreviewConverter::pixmap(const VSFrameRef * a_cpFrameRef)
//==============================================================================

void ZimgPreviewConverter::slotResetSettings()
{
	assert(m_pSettingsManager);

	std::map<int, zimg_resample_filter_e> filterMap =
	{
		{(int)ResamplingFilter::Point, ZIMG_RESIZE_POINT},
		{(int)ResamplingFilter::Bilinear, ZIMG_RESIZE_BILINEAR},
		{(int)ResamplingFilter::Bicubic, ZIMG_RESIZE_BICUBIC},
		{(int)ResamplingFilter::Spline16, ZIMG_RESIZE_SPLINE16},
		{(int)ResamplingFilter::Spline36, ZIMG_RESIZE_SPLINE36},
		{(int)ResamplingFilter::Lanczos, ZIMG_RESIZE_LANCZOS},
	};

	int filter = (int)m_pSettingsManager->getChromaResamplingFilter();
	m_graphBuilderParams.resample_filter = filterMap[filter];
	m_graphBuilderParams.resample_filter_uv =
		m_graphBuilderParams.resample_filter;

	if(filter == (int)ResamplingFilter::Bicubic)
	{
		m_graphBuilderParams.filter_param_a =
			m_pSettingsManager->getBicubicFilterParameterB();
		m_graphBuilderParams.filter_param_a_uv =
			m_graphBuilderParams.filter_param_a;
		m_graphBuilderParams.filter_param_b =
			m_pSettingsManager->getBicubicFilterParameterC();
		m_graphBuilderParams.filter_param_b_uv =
			m_graphBuilderParams.filter_param_b;
	}
	else if(filter == (int)ResamplingFilter::Lanczos)
	{
		m_graphBuilderParams.filter_param_a =
			(double)m_pSettingsManager->getLanczosFilterTaps();
		m_graphBuilderParams.filter_param_a_uv =
			m_graphBuilderParams.filter_param_a;
	}

	if(m_pFilterGraph)
	{
		zimg_filter_graph_free(m_pFilterGraph);
		m_pFilterGraph = zimg_filter_graph_build(&m_inFormat, &m_outFormat,
			&m_graphBuilderParams);
	}
}

// END OF void ZimgPreviewConverter::slotResetSettings()
//==============================================================================

bool ZimgPreviewConverter::translateColorFamily(VSColorFamily a_vsColorFamily,
	zimg_color_family_e * a_pZimgColorFamily,
	zimg_matrix_coefficients_e * a_pZimgMatrixCoefficients)
{
	switch(a_vsColorFamily)
	{
	case cmGray:
		*a_pZimgColorFamily = ZIMG_COLOR_GREY;
		*a_pZimgMatrixCoefficients = ZIMG_MATRIX_UNSPECIFIED;
		break;
	case cmRGB:
		*a_pZimgColorFamily = ZIMG_COLOR_RGB;
		*a_pZimgMatrixCoefficients = ZIMG_MATRIX_RGB;
		break;
	case cmYUV:
		*a_pZimgColorFamily = ZIMG_COLOR_YUV;
		*a_pZimgMatrixCoefficients = ZIMG_MATRIX_UNSPECIFIED;
		break;
	case cmYCoCg:
		*a_pZimgColorFamily = ZIMG_COLOR_YUV;
		*a_pZimgMatrixCoefficients = ZIMG_MATRIX_YCGCO;
		break;
	default:
		m_error = trUtf8("Unsupported colour family.");
		emit signalWriteLogMessage(mtCritical, m_error);
		return false;
	}

	return true;
}

// END OF bool ZimgPreviewConverter::translateColorFamily(
//		VSColorFamily a_vsColorFamily, zimg_color_family_e * a_pZimgColorFamily,
//		zimg_matrix_coefficients_e * a_pZimgMatrixCoefficients)
//==============================================================================

bool ZimgPreviewConverter::translatePixelType(const VSFormat * a_cpVSFormat,
	zimg_pixel_type_e * a_pZimgPixelType)
{
	if((a_cpVSFormat->sampleType == stInteger) &&
		(a_cpVSFormat->bytesPerSample == 1))
		*a_pZimgPixelType = ZIMG_PIXEL_BYTE;
	else if((a_cpVSFormat->sampleType == stInteger) &&
		(a_cpVSFormat->bytesPerSample == 2))
		*a_pZimgPixelType = ZIMG_PIXEL_WORD;
	else if((a_cpVSFormat->sampleType == stFloat) &&
		(a_cpVSFormat->bytesPerSample == 2))
		*a_pZimgPixelType = ZIMG_PIXEL_HALF;
	else if((a_cpVSFormat->sampleType == stFloat) &&
		(a_cpVSFormat->bytesPerSample == 4))
		*a_pZimgPixelType = ZIMG_PIXEL_FLOAT;
	else
	{
		m_error = trUtf8("Unsupported pixel type.");
		emit signalWriteLogMessage(mtCritical, m_error);
		return false;
	}

	return true;
}

// END OF bool ZimgPreviewConverter::translatePixelType(
//		const VSFormat * a_cpVSFormat, zimg_pixel_type_e * a_pZimgPixelType)
//==============================================================================

bool ZimgPreviewConverter::translateFormat(const VSFormat * a_cpVSFormat,
	zimg_image_format * a_pZimgFormat)
{
	if(a_cpVSFormat->id == pfCompatBGR32)
	{
		a_pZimgFormat->color_family = ZIMG_COLOR_RGB;
		a_pZimgFormat->matrix_coefficients = ZIMG_MATRIX_RGB;
		a_pZimgFormat->pixel_type = ZIMG_PIXEL_BYTE;
		a_pZimgFormat->depth = 8;
	}
	else if(a_cpVSFormat->id == pfCompatYUY2)
	{
		a_pZimgFormat->color_family = ZIMG_COLOR_YUV;
		a_pZimgFormat->matrix_coefficients = ZIMG_MATRIX_UNSPECIFIED;
		a_pZimgFormat->pixel_type = ZIMG_PIXEL_BYTE;
		a_pZimgFormat->depth = 8;
	}
	else
	{
		bool result = translateColorFamily(
			static_cast<VSColorFamily>(a_cpVSFormat->colorFamily),
			&a_pZimgFormat->color_family, &a_pZimgFormat->matrix_coefficients);
		if(!result)
			return false;
		result = translatePixelType(a_cpVSFormat, &a_pZimgFormat->pixel_type);
		if(!result)
			return false;
		a_pZimgFormat->depth = a_cpVSFormat->bitsPerSample;
	}

	a_pZimgFormat->subsample_w = a_cpVSFormat->subSamplingW;
	a_pZimgFormat->subsample_h = a_cpVSFormat->subSamplingH;
	a_pZimgFormat->pixel_range =
		(a_pZimgFormat->color_family == ZIMG_COLOR_RGB) ?
		ZIMG_RANGE_FULL : ZIMG_RANGE_LIMITED;

	a_pZimgFormat->field_parity = ZIMG_FIELD_PROGRESSIVE;
	a_pZimgFormat->chroma_location =
		(a_pZimgFormat->subsample_w || a_pZimgFormat->subsample_h) ?
		ZIMG_CHROMA_LEFT : ZIMG_CHROMA_CENTER;

	return true;
}

// END OF bool ZimgPreviewConverter::translateFormat(
//		const VSFormat * a_cpVSFormat, zimg_image_format * a_pZimgFormat)
//==============================================================================

bool ZimgPreviewConverter::importFrameProps(const VSMap * a_cpProps,
	zimg_image_format * a_pZimgFormat, bool * a_pIsInterlaced)
{
	assert(m_cpVSAPI);

	propGetIfValid<int>(a_cpProps, "_ChromaLocation",
		&a_pZimgFormat->chroma_location,
		[](int x){return (x >= 0);}, m_cpVSAPI);

	if(m_cpVSAPI->propNumElements(a_cpProps, "_ColorRange") > 0)
	{
		int64_t x = m_cpVSAPI->propGetInt(a_cpProps, "_ColorRange", 0, nullptr);

		if(x == 0)
			a_pZimgFormat->pixel_range = ZIMG_RANGE_FULL;
		else if(x == 1)
			a_pZimgFormat->pixel_range = ZIMG_RANGE_LIMITED;
		else
		{
			m_error = trUtf8("Invalid \'_ColorRange\' property value: %1")
				.arg(x);
			emit signalWriteLogMessage(mtCritical, m_error);
			return false;
		}
	}

	// Ignore UNSPECIFIED values from properties,
	// since the user can specify them.
	propGetIfValid<int>(a_cpProps, "_Matrix",
		&a_pZimgFormat->matrix_coefficients,
		[](int x){return (x != ZIMG_MATRIX_UNSPECIFIED);}, m_cpVSAPI);
	propGetIfValid<int>(a_cpProps, "_Transfer",
		&a_pZimgFormat->transfer_characteristics,
		[](int x){return (x != ZIMG_TRANSFER_UNSPECIFIED);}, m_cpVSAPI);
	propGetIfValid<int>(a_cpProps, "_Primaries",
		&a_pZimgFormat->color_primaries,
		[](int x){return (x != ZIMG_PRIMARIES_UNSPECIFIED);}, m_cpVSAPI);

	*a_pIsInterlaced = false;
	if(m_cpVSAPI->propNumElements(a_cpProps, "_Field") > 0)
	{
		int64_t x = m_cpVSAPI->propGetInt(a_cpProps, "_Field", 0, nullptr);

		if(x == 0)
			a_pZimgFormat->field_parity = ZIMG_FIELD_BOTTOM;
		else if(x == 1)
			a_pZimgFormat->field_parity = ZIMG_FIELD_TOP;
		else
		{
			m_error = trUtf8("Invalid \'_Field\' property value: %1").arg(x);
			emit signalWriteLogMessage(mtCritical, m_error);
			return false;
		}
	}
	else if(m_cpVSAPI->propNumElements(a_cpProps, "_FieldBased") > 0)
	{
		int64_t x = m_cpVSAPI->propGetInt(a_cpProps, "_FieldBased", 0, nullptr);

		int64_t validValues[] = {0, 1, 2};
		if(!vsedit::contains(validValues, x))
		{
			m_error = trUtf8("Invalid \'_FieldBased\' property value: %1")
				.arg(x);
			emit signalWriteLogMessage(mtCritical, m_error);
			return false;
		}

		*a_pIsInterlaced = ((x == 1) || (x == 2));
	}

	if(*a_pIsInterlaced)
	{
		a_pZimgFormat->active_region.top /= 2;
		a_pZimgFormat->active_region.height /= 2;
	}

	return true;
}

// END OF bool ZimgPreviewConverter::importFrameProps(const VSMap * a_pProps,
//		zimg_image_format * a_pZimgFormat, bool * a_pIsInterlaced)
//==============================================================================

void ZimgPreviewConverter::checkZimgError()
{
	char zimgErrorMessage[1025] = {0};
	int errorCode = zimg_get_last_error(zimgErrorMessage,
		sizeof(zimgErrorMessage));

	if(errorCode == ZIMG_ERROR_SUCCESS)
		return;

	m_error = trUtf8(zimgErrorMessage);
	emit signalWriteLogMessage(mtCritical, m_error);
}

// END OF void ZimgPreviewConverter::checkZimgError()
//==============================================================================

void ZimgPreviewConverter::inpaintError(QPixmap & a_pixmap)
{
	if(m_error.isEmpty())
		return;

	QPainter p(&a_pixmap);
	p.setPen(QColor(Qt::white));
	p.drawText(QRect(0, 0, a_pixmap.width(), a_pixmap.height()),
	Qt::AlignHCenter | Qt::AlignVCenter, m_error);
}

// END OF void ZimgPreviewConverter::inpaintError(QPixmap & a_pixmap)
//==============================================================================
