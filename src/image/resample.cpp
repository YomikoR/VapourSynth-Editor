#include "resample.h"

#include "../common/helpers.h"

#include <vapoursynth/VSHelper.h>

#include <cassert>

//==============================================================================

vsedit::Resampler::Resampler():
	m_sourceWidth(-1)
	, m_sourceHeight(-1)
	, m_destinationWidth(-1)
	, m_destinationHeight(-1)
	, m_pixelType(-1)
	, m_shiftW(0.0)
	, m_shiftH(0.0)
	, m_filterType(-1)
	, m_filterParamA(0.0)
	, m_filterParamB(0.0)
	, m_pResizeContext(nullptr)
	, m_pInFloatBuffer(nullptr)
	, m_inFloatStride(0)
	, m_pOutFloatBuffer(nullptr)
	, m_outFloatStride(0)
	, m_pResizeTempBuffer(nullptr)
	, m_resizeTempBufferSize(0u)
{
}

// END OF vsedit::Resampler::Resampler()
//==============================================================================

vsedit::Resampler::~Resampler()
{
	clear();
}

// END OF vsedit::Resampler::~Resampler()
//==============================================================================

void vsedit::Resampler::clear()
{
	if(m_pResizeContext)
	{
		zimg_resize_delete(m_pResizeContext);
		m_pResizeContext = nullptr;
	}

	if(m_pInFloatBuffer)
	{
		vs_aligned_free(m_pInFloatBuffer);
		m_pInFloatBuffer = nullptr;
	}

	if(m_pOutFloatBuffer)
	{
		vs_aligned_free(m_pOutFloatBuffer);
		m_pOutFloatBuffer = nullptr;
	}

	if(m_pResizeTempBuffer)
	{
		vs_aligned_free(m_pResizeTempBuffer);
		m_pResizeTempBuffer = nullptr;
	}
}

// END OF void vsedit::Resampler::clear()
//==============================================================================

bool vsedit::Resampler::resample(const void * a_pSource, int a_sourceWidth,
	int a_sourceHeight, int a_sourceStride, void * a_pDestination,
	int a_destinationWidth, int a_destinationHeight, int a_destinationStride,
	int a_pixelType, double a_shiftW, double a_shiftH, int a_filterType,
	double a_filterParamA, double a_filterParamB, float a_clampMin,
	float a_clampMax)
{
	//--------------------------------------------------------------------------
	// Check for matching context

	if(m_pResizeContext && (
		(m_sourceWidth != a_sourceWidth) ||
		(m_sourceHeight != a_sourceHeight) ||
		(m_destinationWidth != a_destinationWidth) ||
		(m_destinationHeight != a_destinationHeight) ||
		(m_pixelType != a_pixelType) ||
		(m_shiftW != a_shiftW) ||
		(m_shiftH != a_shiftH) ||
		(m_filterType != a_filterType) ||
		(m_filterParamA != a_filterParamA) ||
		(m_filterParamB != a_filterParamB)))
	{
		zimg_resize_delete(m_pResizeContext);
		m_pResizeContext = nullptr;
	}

	if(!m_pResizeContext)
	{
		m_pResizeContext = zimg_resize_create(a_filterType, a_sourceWidth,
		a_sourceHeight, a_destinationWidth, a_destinationHeight, a_shiftW,
		a_shiftH, (double)a_sourceWidth, (double)a_sourceHeight,
		a_filterParamA, a_filterParamB);
	}

	//--------------------------------------------------------------------------
	// Check for matching temporary buffer size

	size_t l_resizeTempBufferSize =
		zimg_resize_tmp_size(m_pResizeContext, ZIMG_PIXEL_FLOAT);

	if(m_pResizeTempBuffer &&
		(m_resizeTempBufferSize != l_resizeTempBufferSize))
	{
		vs_aligned_free(m_pResizeTempBuffer);
		m_pResizeTempBuffer = nullptr;
	}

	if(!m_pResizeTempBuffer)
	{
		m_pResizeTempBuffer =
			vs_aligned_malloc<void>(l_resizeTempBufferSize, 32);
		assert(m_pResizeTempBuffer);

		m_resizeTempBufferSize = l_resizeTempBufferSize;
	}

	//--------------------------------------------------------------------------

	if(a_pixelType == ZIMG_PIXEL_FLOAT)
	{
		zimg_resize_process(m_pResizeContext, a_pSource, a_pDestination,
			m_pResizeTempBuffer, a_sourceWidth, a_sourceHeight,
			a_destinationWidth, a_destinationHeight, a_sourceStride,
			a_destinationStride, ZIMG_PIXEL_FLOAT);
	}
	else
	{
		//----------------------------------------------------------------------
		// Check for matching input float buffer size

		int inFloatRowLength = a_sourceWidth * sizeof(float);
		int l_inFloatStride = (inFloatRowLength / 32) * 32;
		if(inFloatRowLength % 32)
			l_inFloatStride += 32;

		if(m_pInFloatBuffer && ((m_inFloatStride != l_inFloatStride) ||
			(m_sourceHeight != a_sourceHeight)))
		{
			vs_aligned_free(m_pInFloatBuffer);
			m_pInFloatBuffer = nullptr;
		}

		if(!m_pInFloatBuffer)
		{
			m_pInFloatBuffer = vs_aligned_malloc<void>(
				l_inFloatStride * a_sourceHeight, 32);
			assert(m_pInFloatBuffer);
			m_inFloatStride = l_inFloatStride;
		}

		//----------------------------------------------------------------------
		// Check for matching output float buffer size

		int outFloatRowLength = a_destinationWidth * sizeof(float);
		int l_outFloatStride = (outFloatRowLength / 32) * 32;
		if(outFloatRowLength % 32)
			l_outFloatStride += 32;

		if(m_pOutFloatBuffer && ((m_outFloatStride != l_outFloatStride) ||
			(m_destinationHeight != a_destinationHeight)))
		{
			vs_aligned_free(m_pOutFloatBuffer);
			m_pOutFloatBuffer = nullptr;
		}

		if(!m_pOutFloatBuffer)
		{
			m_pOutFloatBuffer = vs_aligned_malloc<void>(
				l_outFloatStride * a_destinationHeight, 32);
			assert(m_pOutFloatBuffer);
			m_outFloatStride = l_outFloatStride;
		}

		//----------------------------------------------------------------------
		// Copy source to input float buffer

		const uint8_t * cpSourceLine = (const uint8_t *)a_pSource;
		uint8_t * pDestinationLine = (uint8_t *)m_pInFloatBuffer;
		float * pFloatLine;

		if(a_pixelType == ZIMG_PIXEL_BYTE)
		{
			const uint8_t * cpByteLine;

			for(int h = 0; h < a_sourceHeight; ++h)
			{
				cpByteLine = (const uint8_t *)cpSourceLine;
				pFloatLine = (float *)pDestinationLine;

				for(int w = 0; w < a_sourceWidth; ++w)
					pFloatLine[w] = (float)cpByteLine[w];

				cpSourceLine += a_sourceStride;
				pDestinationLine += m_inFloatStride;
			}
		}
		else if(a_pixelType == ZIMG_PIXEL_WORD)
		{
			const uint16_t * cpShortLine;

			for(int h = 0; h < a_sourceHeight; ++h)
			{
				cpShortLine = (const uint16_t *)cpSourceLine;
				pFloatLine = (float *)pDestinationLine;

				for(int w = 0; w < a_sourceWidth; ++w)
					pFloatLine[w] = (float)cpShortLine[w];

				cpSourceLine += a_sourceStride;
				pDestinationLine += m_inFloatStride;
			}
		}

		//----------------------------------------------------------------------
		// Resample

		zimg_resize_process(m_pResizeContext, m_pInFloatBuffer,
			m_pOutFloatBuffer, m_pResizeTempBuffer, a_sourceWidth,
			a_sourceHeight, a_destinationWidth, a_destinationHeight,
			m_inFloatStride, m_outFloatStride, ZIMG_PIXEL_FLOAT);

		//----------------------------------------------------------------------
		// Copy output float buffer to destination with clamping

		uint8_t * pSourceLine = (uint8_t *)m_pOutFloatBuffer;
		pDestinationLine = (uint8_t *)a_pDestination;

		if(a_pixelType == ZIMG_PIXEL_BYTE)
		{
			for(int h = 0; h < a_destinationHeight; ++h)
			{
				pFloatLine = (float *)pSourceLine;
				uint8_t * pByteLine = (uint8_t *)pDestinationLine;

				for(int w = 0; w < a_destinationWidth; ++w)
				{
					vsedit::clamp(pFloatLine[w], a_clampMin, a_clampMax);
					pByteLine[w] = (uint8_t)pFloatLine[w];
				}

				pSourceLine += m_outFloatStride;
				pDestinationLine += a_destinationStride;
			}
		}
		else if(a_pixelType == ZIMG_PIXEL_WORD)
		{
			for(int h = 0; h < a_destinationHeight; ++h)
			{
				pFloatLine = (float *)pSourceLine;
				uint16_t * pShortLine = (uint16_t *)pDestinationLine;

				for(int w = 0; w < a_destinationWidth; ++w)
				{
					vsedit::clamp(pFloatLine[w], a_clampMin, a_clampMax);
					pShortLine[w] = (uint16_t)pFloatLine[w];
				}

				pSourceLine += m_outFloatStride;
				pDestinationLine += a_destinationStride;
			}
		}

		//----------------------------------------------------------------------
	}

	m_sourceWidth = a_sourceWidth;
	m_sourceHeight = a_sourceHeight;
	m_destinationWidth = a_destinationWidth;
	m_destinationHeight = a_destinationHeight;
	m_pixelType = a_pixelType;
	m_shiftW = a_shiftW;
	m_shiftH = a_shiftH;
	m_filterType = a_filterType;
	m_filterParamA = a_filterParamA;
	m_filterParamB = a_filterParamB;

	return true;
}

// END OF bool vsedit::Resampler::resample(void * a_pSource, int a_sourceWidth,
//		int a_sourceHeight, int a_sourceStride, void * a_pDestination,
//		int a_destinationWidth, int a_destinationHeight,
//		int a_destinationStride, int a_pixelType, double a_shiftW,
//		double a_shiftH, int a_filterType, double a_filterParamA,
//		double a_filterParamB, float a_clampMin, float a_clampMax)
//==============================================================================
