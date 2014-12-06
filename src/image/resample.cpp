#include "resample.h"

#include "../common/helpers.h"

#include <vapoursynth/VSHelper.h>

#include <cassert>
#include <vector>

#define ERROR_STRING_SIZE 2048

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
	, m_inFloatBuffer()
	, m_outFloatBuffer()
	, m_resizeTempBuffer()
	, m_error()
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

	m_inFloatBuffer.clear();
	m_outFloatBuffer.clear();
	m_resizeTempBuffer.clear();

	m_error.clear();
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

		if(!m_pResizeContext)
		{
			std::vector<char> zimgErrorString(ERROR_STRING_SIZE);
			zimg_get_last_error(zimgErrorString.data(), ERROR_STRING_SIZE);
			m_error = QString("Could not create zimg resize context. %1")
				.arg(zimgErrorString.data());
			return false;
		}
	}

	//--------------------------------------------------------------------------
	// Check for matching temporary buffer size

	size_t resizeTempBufferSize =
		zimg_resize_tmp_size(m_pResizeContext, ZIMG_PIXEL_FLOAT);
	m_resizeTempBuffer.resize(resizeTempBufferSize);
	if(m_resizeTempBuffer.size() != resizeTempBufferSize)
	{
		m_error = QString("Could not allocate temporary buffer.");
		return false;
	}

	//--------------------------------------------------------------------------

	if(a_pixelType == ZIMG_PIXEL_FLOAT)
	{
		int error = zimg_resize_process(m_pResizeContext, a_pSource,
			a_pDestination, m_resizeTempBuffer.data(),
			a_sourceWidth, a_sourceHeight,
			a_destinationWidth, a_destinationHeight, a_sourceStride,
			a_destinationStride, ZIMG_PIXEL_FLOAT);

		if(error)
		{
			std::vector<char> zimgErrorString(ERROR_STRING_SIZE);
			zimg_get_last_error(zimgErrorString.data(), ERROR_STRING_SIZE);
			m_error = QString("Could not resample the plane. %1")
				.arg(zimgErrorString.data());
			return false;
		}
	}
	else
	{
		size_t bufferSize;

		//----------------------------------------------------------------------
		// Check for matching input float buffer size

		int inFloatRowLength = a_sourceWidth * sizeof(float);
		int inFloatStride = (inFloatRowLength /
			VSE_MEMORY_ALIGNMENT) * VSE_MEMORY_ALIGNMENT;
		if(inFloatRowLength % VSE_MEMORY_ALIGNMENT)
			inFloatStride += VSE_MEMORY_ALIGNMENT;

		bufferSize = inFloatStride * a_sourceHeight;
		m_inFloatBuffer.resize(bufferSize);
		if(m_inFloatBuffer.size() != bufferSize)
		{
			m_error = QString("Could not allocate temporary buffer.");
			return false;
		}

		//----------------------------------------------------------------------
		// Check for matching output float buffer size

		int outFloatRowLength = a_destinationWidth * sizeof(float);
		int outFloatStride = (outFloatRowLength /
			VSE_MEMORY_ALIGNMENT) * VSE_MEMORY_ALIGNMENT;
		if(outFloatRowLength % VSE_MEMORY_ALIGNMENT)
			outFloatStride += VSE_MEMORY_ALIGNMENT;

		bufferSize = outFloatStride * a_destinationHeight;
		m_outFloatBuffer.resize(bufferSize);
		if(m_outFloatBuffer.size() != bufferSize)
		{
			m_error = QString("Could not allocate temporary buffer.");
			return false;
		}

		//----------------------------------------------------------------------
		// Copy source to input float buffer

		const uint8_t * cpSourceLine = (const uint8_t *)a_pSource;
		uint8_t * pDestinationLine = m_inFloatBuffer.data();
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
				pDestinationLine += inFloatStride;
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
				pDestinationLine += inFloatStride;
			}
		}

		//----------------------------------------------------------------------
		// Resample

		int error = zimg_resize_process(m_pResizeContext,
			m_inFloatBuffer.data(), m_outFloatBuffer.data(),
			m_resizeTempBuffer.data(), a_sourceWidth, a_sourceHeight,
			a_destinationWidth, a_destinationHeight, inFloatStride,
			outFloatStride, ZIMG_PIXEL_FLOAT);

		if(error)
		{
			std::vector<char> zimgErrorString(ERROR_STRING_SIZE);
			zimg_get_last_error(zimgErrorString.data(), ERROR_STRING_SIZE);
			m_error = QString("Could not resample the plane. %1")
				.arg(zimgErrorString.data());
			return false;
		}

		//----------------------------------------------------------------------
		// Copy output float buffer to destination with clamping

		uint8_t * pSourceLine = m_outFloatBuffer.data();
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

				pSourceLine += outFloatStride;
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

				pSourceLine += outFloatStride;
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

	m_error.clear();
	return true;
}

// END OF bool vsedit::Resampler::resample(void * a_pSource, int a_sourceWidth,
//		int a_sourceHeight, int a_sourceStride, void * a_pDestination,
//		int a_destinationWidth, int a_destinationHeight,
//		int a_destinationStride, int a_pixelType, double a_shiftW,
//		double a_shiftH, int a_filterType, double a_filterParamA,
//		double a_filterParamB, float a_clampMin, float a_clampMax)
//==============================================================================

const QString & vsedit::Resampler::getError() const
{
	return m_error;
}

// END OF const QString & vsedit::Resampler::getError() const
//==============================================================================
