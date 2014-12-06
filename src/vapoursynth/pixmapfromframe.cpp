#include "vapoursynthscriptprocessor.h"

#include "../common/helpers.h"
#include "../image/yuvtorgb.h"
#include "../image/resample.h"

#include <vapoursynth/VSHelper.h>

//==============================================================================

QPixmap VapourSynthScriptProcessor::pixmapFromGray1B(
	const VSFrameRef * a_cpFrameRef)
{
	int width = m_cpVSAPI->getFrameWidth(a_cpFrameRef, 0);
	int height = m_cpVSAPI->getFrameHeight(a_cpFrameRef, 0);
	const uint8_t * cpRead = m_cpVSAPI->getReadPtr(a_cpFrameRef, 0);
	int stride = m_cpVSAPI->getStride(a_cpFrameRef, 0);

	std::vector<vsedit::RGB32> image(width * height);
	size_t i = 0;
	const uint8_t * cpLine = cpRead;
	for(size_t h = 0; h < (size_t)height; ++h)
	{
		for(size_t w = 0; w < (size_t)width; ++w)
		{
			image[i].parts.r = cpLine[w];
			image[i].parts.g = cpLine[w];
			image[i].parts.b = cpLine[w];
			image[i].parts.x = 255;
			i++;
		}
		cpLine += stride;
	}

	QImage frameImage((const uchar *)image.data(), width, height,
		QImage::Format_RGB32);
	QPixmap framePixmap = QPixmap::fromImage(frameImage).copy();

	return framePixmap;
}

// END OF QPixmap VapourSynthScriptProcessor::pixmapFromGray1B(
//		const VSFrameRef * a_cpFrameRef)
//==============================================================================

QPixmap VapourSynthScriptProcessor::pixmapFromGray2B(
	const VSFrameRef * a_cpFrameRef)
{
	int width = m_cpVSAPI->getFrameWidth(a_cpFrameRef, 0);
	int height = m_cpVSAPI->getFrameHeight(a_cpFrameRef, 0);
	const uint8_t * cpRead = m_cpVSAPI->getReadPtr(a_cpFrameRef, 0);
	int stride = m_cpVSAPI->getStride(a_cpFrameRef, 0);

	std::vector<vsedit::RGB32> image(width * height);
	size_t i = 0;
	const uint16_t * cpLine;
	uint8_t gray;
	for(size_t h = 0; h < (size_t)height; ++h)
	{
		cpLine = (const uint16_t *)cpRead;
		for(size_t w = 0; w < (size_t)width; ++w)
		{
			// Shift with binary rounding
			if(cpLine[w] >= 0xFF80)
				gray = cpLine[w] >> 8;
			else
				gray = (cpLine[w] + 0x0080) >> 8;
			image[i].parts.r = gray;
			image[i].parts.g = gray;
			image[i].parts.b = gray;
			image[i].parts.x = 255;
			i++;
		}
		cpRead += stride;
	}

	QImage frameImage((const uchar *)image.data(), width, height,
		QImage::Format_RGB32);
	QPixmap framePixmap = QPixmap::fromImage(frameImage).copy();

	return framePixmap;
}

// END OF QPixmap VapourSynthScriptProcessor::pixmapFromGray2B(
//		const VSFrameRef * a_cpFrameRef)
//==============================================================================

QPixmap VapourSynthScriptProcessor::pixmapFromGrayH(
	const VSFrameRef * a_cpFrameRef)
{
	int width = m_cpVSAPI->getFrameWidth(a_cpFrameRef, 0);
	int height = m_cpVSAPI->getFrameHeight(a_cpFrameRef, 0);
	const uint8_t * cpRead = m_cpVSAPI->getReadPtr(a_cpFrameRef, 0);
	int stride = m_cpVSAPI->getStride(a_cpFrameRef, 0);

	std::vector<vsedit::RGB32> image(width * height);
	size_t i = 0;
	const uint16_t * cpLine;
	float gray;
	for(size_t h = 0; h < (size_t)height; ++h)
	{
		cpLine = (const uint16_t *)cpRead;
		for(size_t w = 0; w < (size_t)width; ++w)
		{
			vsedit::FP16 half;
			half.u = cpLine[w];
			gray = halfToSingle(half).f * 255.0f;
			vsedit::clamp(gray, 0.0f, 255.0f);
			image[i].parts.r = gray;
			image[i].parts.g = gray;
			image[i].parts.b = gray;
			image[i].parts.x = 255;
			i++;
		}
		cpRead += stride;
	}

	QImage frameImage((const uchar *)image.data(), width, height,
		QImage::Format_RGB32);
	QPixmap framePixmap = QPixmap::fromImage(frameImage).copy();

	return framePixmap;
}

// END OF QPixmap VapourSynthScriptProcessor::pixmapFromGrayH(
//		const VSFrameRef * a_cpFrameRef)
//==============================================================================

QPixmap VapourSynthScriptProcessor::pixmapFromGrayS(
	const VSFrameRef * a_cpFrameRef)
{
	int width = m_cpVSAPI->getFrameWidth(a_cpFrameRef, 0);
	int height = m_cpVSAPI->getFrameHeight(a_cpFrameRef, 0);
	const uint8_t * cpRead = m_cpVSAPI->getReadPtr(a_cpFrameRef, 0);
	int stride = m_cpVSAPI->getStride(a_cpFrameRef, 0);

	std::vector<vsedit::RGB32> image(width * height);
	size_t i = 0;
	const float * cpLine;
	float gray;
	for(size_t h = 0; h < (size_t)height; ++h)
	{
		cpLine = (const float *)cpRead;
		for(size_t w = 0; w < (size_t)width; ++w)
		{
			gray = cpLine[w] * 255.0f;
			vsedit::clamp(gray, 0.0f, 255.0f);
			image[i].parts.r = gray;
			image[i].parts.g = gray;
			image[i].parts.b = gray;
			image[i].parts.x = 255;
			i++;
		}
		cpRead += stride;
	}

	QImage frameImage((const uchar *)image.data(), width, height,
		QImage::Format_RGB32);
	QPixmap framePixmap = QPixmap::fromImage(frameImage).copy();

	return framePixmap;
}

// END OF QPixmap VapourSynthScriptProcessor::pixmapFromGrayS(
//		const VSFrameRef * a_cpFrameRef)
//==============================================================================

QPixmap VapourSynthScriptProcessor::pixmapFromYUV1B(
	const VSFrameRef * a_cpFrameRef)
{
	int widthY = m_cpVSAPI->getFrameWidth(a_cpFrameRef, 0);
	int heightY = m_cpVSAPI->getFrameHeight(a_cpFrameRef, 0);
	int widthU = m_cpVSAPI->getFrameWidth(a_cpFrameRef, 1);
	int heightU = m_cpVSAPI->getFrameHeight(a_cpFrameRef, 1);
	int widthV = m_cpVSAPI->getFrameWidth(a_cpFrameRef, 2);
	int heightV = m_cpVSAPI->getFrameHeight(a_cpFrameRef, 2);
	const uint8_t * cpReadY = m_cpVSAPI->getReadPtr(a_cpFrameRef, 0);
	const uint8_t * cpReadU = m_cpVSAPI->getReadPtr(a_cpFrameRef, 1);
	const uint8_t * cpReadV = m_cpVSAPI->getReadPtr(a_cpFrameRef, 2);
	int strideY = m_cpVSAPI->getStride(a_cpFrameRef, 0);
	int strideU = m_cpVSAPI->getStride(a_cpFrameRef, 1);
	int strideV = m_cpVSAPI->getStride(a_cpFrameRef, 2);

	uint8_t * pUpsampledU = nullptr;
	uint8_t * pUpsampledV = nullptr;
	uint8_t clampLow = 0;
	uint8_t clampHigh = 255;

	if((widthU != widthY) || (heightU != heightY))
	{
		pUpsampledU = vs_aligned_malloc<uint8_t>(heightY * strideY, 32);
		if(!pUpsampledU)
		{
			emit signalWriteLogMessage(mtCritical, QString(
			"Error while creating pixmap for frame.\n"
			"Could not allocate temporary buffer."));
			return QPixmap();
		}

		bool success = m_pResampler->resample(cpReadU, widthU, heightU, strideU,
			pUpsampledU, widthY, heightY, strideY, ZIMG_PIXEL_BYTE,
			0.0, 0.0, ZIMG_RESIZE_SPLINE36, 0.0, 0.0,
			(float)clampLow, (float)clampHigh);
		if(!success)
		{
			emit signalWriteLogMessage(mtCritical, QString(
			"Error while creating pixmap for frame.\n"
			"%1").arg(m_pResampler->getError()));
			vs_aligned_free(pUpsampledU);
			return QPixmap();
		}

		cpReadU = pUpsampledU;
		strideU = strideY;
	}

	if((widthV != widthY) || (heightV != heightY))
	{
		pUpsampledV = vs_aligned_malloc<uint8_t>(heightY * strideY, 32);
		if(!pUpsampledV)
		{
			emit signalWriteLogMessage(mtCritical, QString(
			"Error while creating pixmap for frame.\n"
			"Could not allocate temporary buffer."));
			if(pUpsampledU)
				vs_aligned_free(pUpsampledU);
			return QPixmap();
		}

		bool success = m_pResampler->resample(cpReadV, widthV, heightV, strideV,
			pUpsampledV, widthY, heightY, strideY, ZIMG_PIXEL_BYTE,
			0.0, 0.0, ZIMG_RESIZE_SPLINE36, 0.0, 0.0,
			(float)clampLow, (float)clampHigh);
		if(!success)
		{
			emit signalWriteLogMessage(mtCritical, QString(
			"Error while creating pixmap for frame.\n"
			"%1").arg(m_pResampler->getError()));
			if(pUpsampledU)
				vs_aligned_free(pUpsampledU);
			vs_aligned_free(pUpsampledV);
			return QPixmap();
		}

		cpReadV = pUpsampledV;
		strideV = strideY;
	}

	std::vector<vsedit::RGB32> image(widthY * heightY);
	size_t i = 0;
	for(size_t h = 0; h < (size_t)heightY; ++h)
	{
		for(size_t w = 0; w < (size_t)widthY; ++w)
		{
			image[i] = m_pYuvToRgbConverter->yuvToRgb32(cpReadY[w], cpReadU[w],
				cpReadV[w]);
			i++;
		}
		cpReadY += strideY;
		cpReadU += strideU;
		cpReadV += strideV;
	}

	if(pUpsampledU)
		vs_aligned_free(pUpsampledU);
	if(pUpsampledV)
		vs_aligned_free(pUpsampledV);

	QImage frameImage((const uchar *)image.data(), widthY, heightY,
		QImage::Format_RGB32);
	QPixmap framePixmap = QPixmap::fromImage(frameImage).copy();

	return framePixmap;
}

// END OF QPixmap VapourSynthScriptProcessor::pixmapFromYUV1B(
//		const VSFrameRef * a_cpFrameRef)
//==============================================================================

QPixmap VapourSynthScriptProcessor::pixmapFromYUV2B(
	const VSFrameRef * a_cpFrameRef)
{
	int widthY = m_cpVSAPI->getFrameWidth(a_cpFrameRef, 0);
	int heightY = m_cpVSAPI->getFrameHeight(a_cpFrameRef, 0);
	int widthU = m_cpVSAPI->getFrameWidth(a_cpFrameRef, 1);
	int heightU = m_cpVSAPI->getFrameHeight(a_cpFrameRef, 1);
	int widthV = m_cpVSAPI->getFrameWidth(a_cpFrameRef, 2);
	int heightV = m_cpVSAPI->getFrameHeight(a_cpFrameRef, 2);
	const uint8_t * cpReadY = m_cpVSAPI->getReadPtr(a_cpFrameRef, 0);
	const uint8_t * cpReadU = m_cpVSAPI->getReadPtr(a_cpFrameRef, 1);
	const uint8_t * cpReadV = m_cpVSAPI->getReadPtr(a_cpFrameRef, 2);
	int strideY = m_cpVSAPI->getStride(a_cpFrameRef, 0);
	int strideU = m_cpVSAPI->getStride(a_cpFrameRef, 1);
	int strideV = m_cpVSAPI->getStride(a_cpFrameRef, 2);
	int bitsPerSample = m_cpVideoInfo->format->bitsPerSample;

	uint8_t * pUpsampledU = nullptr;
	uint8_t * pUpsampledV = nullptr;
	uint16_t clampLow = 0;
	uint16_t clampHigh = (1ul << bitsPerSample) - 1ul;

	if((widthU != widthY) || (heightU != heightY))
	{
		pUpsampledU = vs_aligned_malloc<uint8_t>(heightY * strideY, 32);
		if(!pUpsampledU)
		{
			emit signalWriteLogMessage(mtCritical, QString(
			"Error while creating pixmap for frame.\n"
			"Could not allocate temporary buffer."));
			return QPixmap();
		}

		bool success = m_pResampler->resample(cpReadU, widthU, heightU, strideU,
			pUpsampledU, widthY, heightY, strideY, ZIMG_PIXEL_WORD,
			0.0, 0.0, ZIMG_RESIZE_SPLINE36, 0.0, 0.0,
			(float)clampLow, (float)clampHigh);
		if(!success)
		{
			emit signalWriteLogMessage(mtCritical, QString(
			"Error while creating pixmap for frame.\n"
			"%1").arg(m_pResampler->getError()));
			vs_aligned_free(pUpsampledU);
			return QPixmap();
		}

		cpReadU = pUpsampledU;
		strideU = strideY;
	}

	if((widthV != widthY) || (heightV != heightY))
	{
		pUpsampledV = vs_aligned_malloc<uint8_t>(heightY * strideY, 32);
		if(!pUpsampledV)
		{
			emit signalWriteLogMessage(mtCritical, QString(
			"Error while creating pixmap for frame.\n"
			"Could not allocate temporary buffer."));
			if(pUpsampledU)
				vs_aligned_free(pUpsampledU);
			return QPixmap();
		}

		bool success = m_pResampler->resample(cpReadV, widthV, heightV, strideV,
			pUpsampledV, widthY, heightY, strideY, ZIMG_PIXEL_WORD,
			0.0, 0.0, ZIMG_RESIZE_SPLINE36, 0.0, 0.0,
			(float)clampLow, (float)clampHigh);
		if(!success)
		{
			emit signalWriteLogMessage(mtCritical, QString(
			"Error while creating pixmap for frame.\n"
			"%1").arg(m_pResampler->getError()));
			if(pUpsampledU)
				vs_aligned_free(pUpsampledU);
			vs_aligned_free(pUpsampledV);
			return QPixmap();
		}

		cpReadV = pUpsampledV;
		strideV = strideY;
	}

	std::vector<vsedit::RGB32> image(widthY * heightY);
	const uint16_t * cpLineY;
	const uint16_t * cpLineU;
	const uint16_t * cpLineV;
	size_t i = 0;
	for(size_t h = 0; h < (size_t)heightY; ++h)
	{
		cpLineY = (const uint16_t *)cpReadY;
		cpLineU = (const uint16_t *)cpReadU;
		cpLineV = (const uint16_t *)cpReadV;
		for(size_t w = 0; w < (size_t)widthY; ++w)
		{
			image[i] = m_pYuvToRgbConverter->yuvToRgb32(cpLineY[w], cpLineU[w],
				cpLineV[w], bitsPerSample);
			i++;
		}
		cpReadY += strideY;
		cpReadU += strideU;
		cpReadV += strideV;
	}

	if(pUpsampledU)
		vs_aligned_free(pUpsampledU);
	if(pUpsampledV)
		vs_aligned_free(pUpsampledV);

	QImage frameImage((const uchar *)image.data(), widthY, heightY,
		QImage::Format_RGB32);
	QPixmap framePixmap = QPixmap::fromImage(frameImage).copy();

	return framePixmap;
}

// END OF QPixmap VapourSynthScriptProcessor::pixmapFromYUV2B(
//		const VSFrameRef * a_cpFrameRef)
//==============================================================================

QPixmap VapourSynthScriptProcessor::pixmapFromYUVH(
	const VSFrameRef * a_cpFrameRef)
{
	int widthY = m_cpVSAPI->getFrameWidth(a_cpFrameRef, 0);
	int heightY = m_cpVSAPI->getFrameHeight(a_cpFrameRef, 0);
	int widthU = m_cpVSAPI->getFrameWidth(a_cpFrameRef, 1);
	int heightU = m_cpVSAPI->getFrameHeight(a_cpFrameRef, 1);
	int widthV = m_cpVSAPI->getFrameWidth(a_cpFrameRef, 2);
	int heightV = m_cpVSAPI->getFrameHeight(a_cpFrameRef, 2);
	const uint8_t * cpReadY = m_cpVSAPI->getReadPtr(a_cpFrameRef, 0);
	const uint8_t * cpReadU = m_cpVSAPI->getReadPtr(a_cpFrameRef, 1);
	const uint8_t * cpReadV = m_cpVSAPI->getReadPtr(a_cpFrameRef, 2);
	int strideY = m_cpVSAPI->getStride(a_cpFrameRef, 0);
	int strideU = m_cpVSAPI->getStride(a_cpFrameRef, 1);
	int strideV = m_cpVSAPI->getStride(a_cpFrameRef, 2);

	int upsampledFloatRowLength = widthY * sizeof(float);
	int upsampledFloatStride = (upsampledFloatRowLength / 32) * 32;
	if(upsampledFloatRowLength % 32)
		upsampledFloatStride += 32;
	size_t upsampledFloatBufferSize = upsampledFloatStride * heightY;

	//--------------------------------------------------------------------------
	// Convert Y-plane to float

	uint8_t * pFloatY =
		vs_aligned_malloc<uint8_t>(upsampledFloatBufferSize, 32);
	if(!pFloatY)
	{
		emit signalWriteLogMessage(mtCritical, QString(
		"Error while creating pixmap for frame.\n"
		"Could not allocate temporary buffer."));
		return QPixmap();
	}

	uint8_t * pByteLine = pFloatY;
	const uint16_t * cpHalfLine;
	float * pFloatLine;
	vsedit::FP16 halfValue;

	for(int h = 0; h < heightY; ++h)
	{
		cpHalfLine = (const uint16_t *)cpReadY;
		pFloatLine = (float *)pByteLine;

		for(int w = 0; w < widthY; ++w)
		{
			halfValue.u = cpHalfLine[w];
			pFloatLine[w] = vsedit::halfToSingle(halfValue).f;
		}

		cpReadY += strideY;
		pByteLine += upsampledFloatStride;
	}

	float clampLow = -0.5f;
	float clampHigh = 0.5f;

	//--------------------------------------------------------------------------
	// Convert U-plane to float

	int floatRowLengthU = widthU * sizeof(float);
	int floatStrideU = (floatRowLengthU / 32) * 32;
	if(floatRowLengthU % 32)
		floatStrideU += 32;

	uint8_t * pFloatU = vs_aligned_malloc<uint8_t>(floatStrideU * heightU, 32);
	if(!pFloatU)
	{
		emit signalWriteLogMessage(mtCritical, QString(
		"Error while creating pixmap for frame.\n"
		"Could not allocate temporary buffer."));
		vs_aligned_free(pFloatY);
		return QPixmap();
	}

	pByteLine = pFloatU;

	for(int h = 0; h < heightU; ++h)
	{
		cpHalfLine = (const uint16_t *)cpReadU;
		pFloatLine = (float *)pByteLine;

		for(int w = 0; w < widthU; ++w)
		{
			halfValue.u = cpHalfLine[w];
			pFloatLine[w] = vsedit::halfToSingle(halfValue).f;
		}

		cpReadU += strideU;
		pByteLine += floatStrideU;
	}

	if((widthU != widthY) || (heightU != heightY))
	{
		uint8_t * pUpsampledU =
			vs_aligned_malloc<uint8_t>(upsampledFloatBufferSize, 32);
		if(!pUpsampledU)
		{
			emit signalWriteLogMessage(mtCritical, QString(
			"Error while creating pixmap for frame.\n"
			"Could not allocate temporary buffer."));
			vs_aligned_free(pFloatY);
			vs_aligned_free(pFloatU);
			return QPixmap();
		}

		bool success = m_pResampler->resample(pFloatU, widthU, heightU,
			floatStrideU, pUpsampledU, widthY, heightY, upsampledFloatStride,
			ZIMG_PIXEL_FLOAT, 0.0, 0.0, ZIMG_RESIZE_SPLINE36, 0.0, 0.0,
			(float)clampLow, (float)clampHigh);
		if(!success)
		{
			emit signalWriteLogMessage(mtCritical, QString(
			"Error while creating pixmap for frame.\n"
			"%1").arg(m_pResampler->getError()));
			vs_aligned_free(pFloatY);
			vs_aligned_free(pFloatU);
			vs_aligned_free(pUpsampledU);
			return QPixmap();
		}

		vs_aligned_free(pFloatU);
		pFloatU = pUpsampledU;
	}

	//--------------------------------------------------------------------------
	// Convert V-plane to float

	int floatRowLengthV = widthV * sizeof(float);
	int floatStrideV = (floatRowLengthV / 32) * 32;
	if(floatRowLengthV % 32)
		floatStrideV += 32;

	uint8_t * pFloatV = vs_aligned_malloc<uint8_t>(floatStrideV * heightV, 32);
	if(!pFloatV)
	{
		emit signalWriteLogMessage(mtCritical, QString(
		"Error while creating pixmap for frame.\n"
		"Could not allocate temporary buffer."));
		vs_aligned_free(pFloatY);
		vs_aligned_free(pFloatU);
		return QPixmap();
	}

	pByteLine = pFloatV;

	for(int h = 0; h < heightV; ++h)
	{
		cpHalfLine = (const uint16_t *)cpReadV;
		pFloatLine = (float *)pByteLine;

		for(int w = 0; w < widthV; ++w)
		{
			halfValue.u = cpHalfLine[w];
			pFloatLine[w] = vsedit::halfToSingle(halfValue).f;
		}

		cpReadV += strideV;
		pByteLine += floatStrideV;
	}

	if((widthV != widthY) || (heightV != heightY))
	{
		uint8_t * pUpsampledV =
			vs_aligned_malloc<uint8_t>(upsampledFloatBufferSize, 32);
		if(!pUpsampledV)
		{
			emit signalWriteLogMessage(mtCritical, QString(
			"Error while creating pixmap for frame.\n"
			"Could not allocate temporary buffer."));
			vs_aligned_free(pFloatY);
			vs_aligned_free(pFloatU);
			vs_aligned_free(pFloatV);
			return QPixmap();
		}

		bool success = m_pResampler->resample(pFloatV, widthV, heightV,
			floatStrideV, pUpsampledV, widthY, heightY, upsampledFloatStride,
			ZIMG_PIXEL_FLOAT, 0.0, 0.0, ZIMG_RESIZE_SPLINE36, 0.0, 0.0,
			(float)clampLow, (float)clampHigh);
		if(!success)
		{
			emit signalWriteLogMessage(mtCritical, QString(
			"Error while creating pixmap for frame.\n"
			"%1").arg(m_pResampler->getError()));
			vs_aligned_free(pFloatY);
			vs_aligned_free(pFloatU);
			vs_aligned_free(pFloatV);
			vs_aligned_free(pUpsampledV);
			return QPixmap();
		}

		vs_aligned_free(pFloatV);
		pFloatV = pUpsampledV;
	}

	//--------------------------------------------------------------------------

	std::vector<vsedit::RGB32> image(widthY * heightY);
	size_t i = 0;
	const uint8_t * cpInLineY = (const uint8_t *)pFloatY;
	const uint8_t * cpInLineU = (const uint8_t *)pFloatU;
	const uint8_t * cpInLineV = (const uint8_t *)pFloatV;

	const float * cpFloatLineY;
	const float * cpFloatLineU;
	const float * cpFloatLineV;

	for(size_t h = 0; h < (size_t)heightY; ++h)
	{
		cpFloatLineY = (const float *)cpInLineY;
		cpFloatLineU = (const float *)cpInLineU;
		cpFloatLineV = (const float *)cpInLineV;

		for(size_t w = 0; w < (size_t)widthY; ++w)
		{
			image[i] = m_pYuvToRgbConverter->yuvToRgb32(cpFloatLineY[w],
				cpFloatLineU[w], cpFloatLineV[w]);
			i++;
		}

		cpInLineY += upsampledFloatStride;
		cpInLineU += upsampledFloatStride;
		cpInLineV += upsampledFloatStride;
	}

	QImage frameImage((const uchar *)image.data(), widthY, heightY,
		QImage::Format_RGB32);
	QPixmap framePixmap = QPixmap::fromImage(frameImage).copy();

	vs_aligned_free(pFloatY);
	vs_aligned_free(pFloatU);
	vs_aligned_free(pFloatV);
	return framePixmap;
}

// END OF QPixmap VapourSynthScriptProcessor::pixmapFromYUVH(
//		const VSFrameRef * a_cpFrameRef)
//==============================================================================

QPixmap VapourSynthScriptProcessor::pixmapFromYUVS(
	const VSFrameRef * a_cpFrameRef)
{
	int widthY = m_cpVSAPI->getFrameWidth(a_cpFrameRef, 0);
	int heightY = m_cpVSAPI->getFrameHeight(a_cpFrameRef, 0);
	int widthU = m_cpVSAPI->getFrameWidth(a_cpFrameRef, 1);
	int heightU = m_cpVSAPI->getFrameHeight(a_cpFrameRef, 1);
	int widthV = m_cpVSAPI->getFrameWidth(a_cpFrameRef, 2);
	int heightV = m_cpVSAPI->getFrameHeight(a_cpFrameRef, 2);
	const uint8_t * cpReadY = m_cpVSAPI->getReadPtr(a_cpFrameRef, 0);
	const uint8_t * cpReadU = m_cpVSAPI->getReadPtr(a_cpFrameRef, 1);
	const uint8_t * cpReadV = m_cpVSAPI->getReadPtr(a_cpFrameRef, 2);
	int strideY = m_cpVSAPI->getStride(a_cpFrameRef, 0);
	int strideU = m_cpVSAPI->getStride(a_cpFrameRef, 1);
	int strideV = m_cpVSAPI->getStride(a_cpFrameRef, 2);

	uint8_t * pUpsampledU = nullptr;
	uint8_t * pUpsampledV = nullptr;
	float clampLow = -0.5f;
	float clampHigh = 0.5f;

	if((widthU != widthY) || (heightU != heightY))
	{
		pUpsampledU = vs_aligned_malloc<uint8_t>(heightY * strideY, 32);
		if(!pUpsampledU)
		{
			emit signalWriteLogMessage(mtCritical, QString(
			"Error while creating pixmap for frame.\n"
			"Could not allocate temporary buffer."));
			return QPixmap();
		}

		bool success = m_pResampler->resample(cpReadU, widthU, heightU, strideU,
			pUpsampledU, widthY, heightY, strideY, ZIMG_PIXEL_FLOAT,
			0.0, 0.0, ZIMG_RESIZE_SPLINE36, 0.0, 0.0,
			clampLow, clampHigh);
		if(!success)
		{
			emit signalWriteLogMessage(mtCritical, QString(
			"Error while creating pixmap for frame.\n"
			"%1").arg(m_pResampler->getError()));
			vs_aligned_free(pUpsampledU);
			return QPixmap();
		}

		cpReadU = pUpsampledU;
		strideU = strideY;
	}

	if((widthV != widthY) || (heightV != heightY))
	{
		pUpsampledV = vs_aligned_malloc<uint8_t>(heightY * strideY, 32);
		if(!pUpsampledV)
		{
			emit signalWriteLogMessage(mtCritical, QString(
			"Error while creating pixmap for frame.\n"
			"Could not allocate temporary buffer."));
			if(pUpsampledU)
				vs_aligned_free(pUpsampledU);
			return QPixmap();
		}

		bool success = m_pResampler->resample(cpReadV, widthV, heightV, strideV,
			pUpsampledV, widthY, heightY, strideY, ZIMG_PIXEL_FLOAT,
			0.0, 0.0, ZIMG_RESIZE_SPLINE36, 0.0, 0.0,
			clampLow, clampHigh);
		if(!success)
		{
			emit signalWriteLogMessage(mtCritical, QString(
			"Error while creating pixmap for frame.\n"
			"%1").arg(m_pResampler->getError()));
			if(pUpsampledU)
				vs_aligned_free(pUpsampledU);
			vs_aligned_free(pUpsampledV);
			return QPixmap();
		}

		cpReadV = pUpsampledV;
		strideV = strideY;
	}

	std::vector<vsedit::RGB32> image(widthY * heightY);
	const float * cpLineY;
	const float * cpLineU;
	const float * cpLineV;
	size_t i = 0;
	for(size_t h = 0; h < (size_t)heightY; ++h)
	{
		cpLineY = (const float *)cpReadY;
		cpLineU = (const float *)cpReadU;
		cpLineV = (const float *)cpReadV;
		for(size_t w = 0; w < (size_t)widthY; ++w)
		{
			image[i] = m_pYuvToRgbConverter->yuvToRgb32(cpLineY[w], cpLineU[w],
				cpLineV[w]);
			i++;
		}
		cpReadY += strideY;
		cpReadU += strideU;
		cpReadV += strideV;
	}

	if(pUpsampledU)
		vs_aligned_free(pUpsampledU);
	if(pUpsampledV)
		vs_aligned_free(pUpsampledV);

	QImage frameImage((const uchar *)image.data(), widthY, heightY,
		QImage::Format_RGB32);
	QPixmap framePixmap = QPixmap::fromImage(frameImage).copy();

	return framePixmap;
}

// END OF QPixmap VapourSynthScriptProcessor::pixmapFromYUVS(
//		const VSFrameRef * a_cpFrameRef)
//==============================================================================

QPixmap VapourSynthScriptProcessor::pixmapFromRGB1B(
	const VSFrameRef * a_cpFrameRef)
{
	int width = m_cpVSAPI->getFrameWidth(a_cpFrameRef, 0);
	int height = m_cpVSAPI->getFrameHeight(a_cpFrameRef, 0);
	const uint8_t * cpReadR = m_cpVSAPI->getReadPtr(a_cpFrameRef, 0);
	const uint8_t * cpReadG = m_cpVSAPI->getReadPtr(a_cpFrameRef, 1);
	const uint8_t * cpReadB = m_cpVSAPI->getReadPtr(a_cpFrameRef, 2);
	int strideR = m_cpVSAPI->getStride(a_cpFrameRef, 0);
	int strideG = m_cpVSAPI->getStride(a_cpFrameRef, 1);
	int strideB = m_cpVSAPI->getStride(a_cpFrameRef, 2);

	std::vector<vsedit::RGB32> image(width * height);
	size_t i = 0;
	for(size_t h = 0; h < (size_t)height; ++h)
	{
		for(size_t w = 0; w < (size_t)width; ++w)
		{
			image[i].parts.r = cpReadR[w];
			image[i].parts.g = cpReadG[w];
			image[i].parts.b = cpReadB[w];
			image[i].parts.x = 255;
			i++;
		}
		cpReadR += strideR;
		cpReadG += strideG;
		cpReadB += strideB;
	}

	QImage frameImage((const uchar *)image.data(), width, height,
		QImage::Format_RGB32);
	QPixmap framePixmap = QPixmap::fromImage(frameImage).copy();

	return framePixmap;
}

// END OF QPixmap VapourSynthScriptProcessor::pixmapFromRGB1B(
//		const VSFrameRef * a_cpFrameRef)
//==============================================================================

QPixmap VapourSynthScriptProcessor::pixmapFromRGB2B(
	const VSFrameRef * a_cpFrameRef)
{
	int width = m_cpVSAPI->getFrameWidth(a_cpFrameRef, 0);
	int height = m_cpVSAPI->getFrameHeight(a_cpFrameRef, 0);
	const uint8_t * cpReadR = m_cpVSAPI->getReadPtr(a_cpFrameRef, 0);
	const uint8_t * cpReadG = m_cpVSAPI->getReadPtr(a_cpFrameRef, 1);
	const uint8_t * cpReadB = m_cpVSAPI->getReadPtr(a_cpFrameRef, 2);
	int strideR = m_cpVSAPI->getStride(a_cpFrameRef, 0);
	int strideG = m_cpVSAPI->getStride(a_cpFrameRef, 1);
	int strideB = m_cpVSAPI->getStride(a_cpFrameRef, 2);
	int bitsPerSample = m_cpVideoInfo->format->bitsPerSample;


	uint8_t shift = bitsPerSample - 8;
	uint16_t fraction = 1 << (shift - 1);
	uint16_t threshold = (1 << bitsPerSample) - fraction;

	std::vector<vsedit::RGB32> image(width * height);
	size_t i = 0;
	const uint16_t *cpLineR, *cpLineG, *cpLineB;
	for(size_t h = 0; h < (size_t)height; ++h)
	{
		cpLineR = (const uint16_t *)cpReadR;
		cpLineG = (const uint16_t *)cpReadG;
		cpLineB = (const uint16_t *)cpReadB;
		for(size_t w = 0; w < (size_t)width; ++w)
		{
			if(cpLineR[w] >= threshold)
				image[i].parts.r = cpLineR[w] >> shift;
			else
				image[i].parts.r = (cpLineR[w] + fraction) >> shift;

			if(cpLineG[w] >= threshold)
				image[i].parts.g = cpLineG[w] >> shift;
			else
				image[i].parts.g = (cpLineG[w] + fraction) >> shift;

			if(cpLineB[w] >= threshold)
				image[i].parts.b = cpLineB[w] >> shift;
			else
				image[i].parts.b = (cpLineB[w] + fraction) >> shift;

			image[i].parts.x = 255;
			i++;
		}
		cpReadR += strideR;
		cpReadG += strideG;
		cpReadB += strideB;
	}

	QImage frameImage((const uchar *)image.data(), width, height,
		QImage::Format_RGB32);
	QPixmap framePixmap = QPixmap::fromImage(frameImage).copy();

	return framePixmap;
}

// END OF QPixmap VapourSynthScriptProcessor::pixmapFromRGB2B(
//		const VSFrameRef * a_cpFrameRef)
//==============================================================================

QPixmap VapourSynthScriptProcessor::pixmapFromRGBH(
	const VSFrameRef * a_cpFrameRef)
{
	int width = m_cpVSAPI->getFrameWidth(a_cpFrameRef, 0);
	int height = m_cpVSAPI->getFrameHeight(a_cpFrameRef, 0);
	const uint8_t * cpReadR = m_cpVSAPI->getReadPtr(a_cpFrameRef, 0);
	const uint8_t * cpReadG = m_cpVSAPI->getReadPtr(a_cpFrameRef, 1);
	const uint8_t * cpReadB = m_cpVSAPI->getReadPtr(a_cpFrameRef, 2);
	int strideR = m_cpVSAPI->getStride(a_cpFrameRef, 0);
	int strideG = m_cpVSAPI->getStride(a_cpFrameRef, 1);
	int strideB = m_cpVSAPI->getStride(a_cpFrameRef, 2);

	std::vector<vsedit::RGB32> image(width * height);
	size_t i = 0;
	const uint16_t *cpLineR, *cpLineG, *cpLineB;
	vsedit::FP16 half;
	float single;
	for(size_t h = 0; h < (size_t)height; ++h)
	{
		cpLineR = (const uint16_t *)cpReadR;
		cpLineG = (const uint16_t *)cpReadG;
		cpLineB = (const uint16_t *)cpReadB;
		for(size_t w = 0; w < (size_t)width; ++w)
		{
			half.u = cpLineR[w];
			single = halfToSingle(half).f * 255.0f;
			vsedit::clamp(single, 0.0f, 255.0f);
			image[i].parts.r = single;

			half.u = cpLineG[w];
			single = halfToSingle(half).f * 255.0f;
			vsedit::clamp(single, 0.0f, 255.0f);
			image[i].parts.g = single;

			half.u = cpLineB[w];
			single = halfToSingle(half).f * 255.0f;
			vsedit::clamp(single, 0.0f, 255.0f);
			image[i].parts.b = single;

			image[i].parts.x = 255;
			i++;
		}
		cpReadR += strideR;
		cpReadG += strideG;
		cpReadB += strideB;
	}

	QImage frameImage((const uchar *)image.data(), width, height,
		QImage::Format_RGB32);
	QPixmap framePixmap = QPixmap::fromImage(frameImage).copy();

	return framePixmap;
}

// END OF QPixmap VapourSynthScriptProcessor::pixmapFromRGBH(
//		const VSFrameRef * a_cpFrameRef)
//==============================================================================

QPixmap VapourSynthScriptProcessor::pixmapFromRGBS(
	const VSFrameRef * a_cpFrameRef)
{
	int width = m_cpVSAPI->getFrameWidth(a_cpFrameRef, 0);
	int height = m_cpVSAPI->getFrameHeight(a_cpFrameRef, 0);
	const uint8_t * cpReadR = m_cpVSAPI->getReadPtr(a_cpFrameRef, 0);
	const uint8_t * cpReadG = m_cpVSAPI->getReadPtr(a_cpFrameRef, 1);
	const uint8_t * cpReadB = m_cpVSAPI->getReadPtr(a_cpFrameRef, 2);
	int strideR = m_cpVSAPI->getStride(a_cpFrameRef, 0);
	int strideG = m_cpVSAPI->getStride(a_cpFrameRef, 1);
	int strideB = m_cpVSAPI->getStride(a_cpFrameRef, 2);

	std::vector<vsedit::RGB32> image(width * height);
	size_t i = 0;
	const float *cpLineR, *cpLineG, *cpLineB;
	float single;
	for(size_t h = 0; h < (size_t)height; ++h)
	{
		cpLineR = (const float *)cpReadR;
		cpLineG = (const float *)cpReadG;
		cpLineB = (const float *)cpReadB;
		for(size_t w = 0; w < (size_t)width; ++w)
		{
			single = cpLineR[w] * 255.0f;
			vsedit::clamp(single, 0.0f, 255.0f);
			image[i].parts.r = single;

			single = cpLineG[w] * 255.0f;
			vsedit::clamp(single, 0.0f, 255.0f);
			image[i].parts.g = single;

			single = cpLineB[w] * 255.0f;
			vsedit::clamp(single, 0.0f, 255.0f);
			image[i].parts.b = single;

			image[i].parts.x = 255;
			i++;
		}
		cpReadR += strideR;
		cpReadG += strideG;
		cpReadB += strideB;
	}

	QImage frameImage((const uchar *)image.data(), width, height,
		QImage::Format_RGB32);
	QPixmap framePixmap = QPixmap::fromImage(frameImage).copy();

	return framePixmap;
}

// END OF QPixmap VapourSynthScriptProcessor::pixmapFromRGBS(
//		const VSFrameRef * a_cpFrameRef)
//==============================================================================

QPixmap VapourSynthScriptProcessor::pixmapFromCompatBGR32(
	const VSFrameRef * a_cpFrameRef)
{
	int frameWidth = m_cpVSAPI->getFrameWidth(a_cpFrameRef, 0);
	int frameHeight = m_cpVSAPI->getFrameHeight(a_cpFrameRef, 0);
	int stride = m_cpVSAPI->getStride(a_cpFrameRef, 0);
	const uint8_t * pFrameReadPointer =
		m_cpVSAPI->getReadPtr(a_cpFrameRef, 0);

	// Creating QImage from memory doesn't copy or modify the data.
	QImage frameImage(pFrameReadPointer, frameWidth, frameHeight, stride,
		QImage::Format_RGB32);
	// But when creating the pixmap - we use mirrored copy.
	QPixmap framePixmap = QPixmap::fromImage(frameImage.mirrored());

	return framePixmap;
}

// END OF QPixmap VapourSynthScriptProcessor::pixmapFromCompatBGR32(
//		const VSFrameRef * a_cpFrameRef)
//==============================================================================

QPixmap VapourSynthScriptProcessor::pixmapFromCompatYUY2(
	const VSFrameRef * a_cpFrameRef)
{
	int width = m_cpVSAPI->getFrameWidth(a_cpFrameRef, 0);
	int height = m_cpVSAPI->getFrameHeight(a_cpFrameRef, 0);
	const uint8_t * cpRead = m_cpVSAPI->getReadPtr(a_cpFrameRef, 0);
	int stride = m_cpVSAPI->getStride(a_cpFrameRef, 0);

	std::vector<vsedit::RGB32> image(width * height);
	size_t i = 0;
	const vsedit::YUY2 * cpLine;
	for(size_t h = 0; h < (size_t)height; ++h)
	{
		cpLine = (const vsedit::YUY2 *)cpRead;
		for(size_t w = 0; w < (size_t)width; ++w)
		{
			if(w & 1)
			{
				image[i] = m_pYuvToRgbConverter->yuvToRgb32(
					cpLine[w >> 1].parts.y1,
					cpLine[w >> 1].parts.u,
					cpLine[w >> 1].parts.v);
			}
			else
			{
				image[i] = m_pYuvToRgbConverter->yuvToRgb32(
					cpLine[w >> 1].parts.y0,
					cpLine[w >> 1].parts.u,
					cpLine[w >> 1].parts.v);
			}
			i++;
		}
		cpRead += stride;
	}

	QImage frameImage((const uchar *)image.data(), width, height,
		QImage::Format_RGB32);
	QPixmap framePixmap = QPixmap::fromImage(frameImage).copy();

	return framePixmap;
}

// END OF QPixmap VapourSynthScriptProcessor::pixmapFromCompatYUY2(
//		const VSFrameRef * a_cpFrameRef)
//==============================================================================