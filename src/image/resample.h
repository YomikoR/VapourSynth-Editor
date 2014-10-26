#ifndef RESAMPLE_H_INCLUDED
#define RESAMPLE_H_INCLUDED

#include <vector>
#include <cmath>

#include "padimage.h"
#include "resamplefilters.h"

namespace vsedit
{
	template<typename T>
	void resampleImageLinear(const AbstractResampleLinearFilter * a_cpFilter,
		const T * a_cpSource, size_t a_sourceWidth,
		size_t a_sourceHeight, ptrdiff_t a_sourceStride,
		T * a_pDestination, size_t a_destinationWidth,
		size_t a_destinationHeight, ptrdiff_t a_destinationStride,
		T a_clampLow, T a_clampHigh)
	{
		const float shiftX = -0.25f;
		const float shiftY = -0.25f;
		size_t taps = a_cpFilter->taps();
		size_t windowOverlap = (size_t)std::ceil(
			(float)taps / 2.0f - 1.0f);
		size_t padding = (size_t)std::ceil((float)windowOverlap +
			std::max(std::fabs(shiftX), std::fabs(shiftY)));

		uint8_t * pDestinationBase = (uint8_t *)a_pDestination;
		float kx = (float)a_sourceWidth / (float)a_destinationWidth;
		float ky = (float)a_sourceHeight / (float)a_destinationHeight;

		T * pPaddedImage = padImageRepeatBorders(a_cpSource, a_sourceWidth,
			a_sourceHeight, a_sourceStride, padding);

		size_t paddedWidth = a_sourceWidth + 2 * padding;

		float sx;
		float sy;
		std::vector<double> wx(taps);
		std::vector<double> wy(taps);
		double rowSumm;
		double result;
		const T * pSourceLine;
		std::vector<const T *> ppSourceLine(taps);
		T * pDestinationLine;
		const T * pWindowRow;

		for(size_t h = 0; h < a_destinationHeight; ++h)
		{
			sy = (float)h * ky + shiftY;
			for(size_t i = 0; i < taps; ++i)
			{
				wy[i] = a_cpFilter->weight(
					sy - std::floor(sy + i - windowOverlap));
			}

			pSourceLine = pPaddedImage + ((ptrdiff_t)std::floor(sy -
				windowOverlap) + padding) * paddedWidth;
			for(size_t i = 0; i < taps; ++i)
			{
				ppSourceLine[i] = pSourceLine;
				pSourceLine += paddedWidth;
			}

			pDestinationLine = (T *)pDestinationBase;
			for(size_t w = 0; w < a_destinationWidth; ++w)
			{
				sx = (float)w * kx + shiftX;
				for(size_t i = 0; i < taps; ++i)
				{
					wx[i] = a_cpFilter->weight(
						sx - std::floor(sx + i - windowOverlap));
				}

				result = 0.0f;
				for(size_t i = 0; i < taps; ++i)
				{
					rowSumm = 0;
					pWindowRow = ppSourceLine[i] + (ptrdiff_t)std::floor(sx -
						windowOverlap) + padding;
					for(size_t j = 0; j < taps; ++j)
						rowSumm += wx[j] * pWindowRow[j];
					result += wy[i] * rowSumm;
				}

				clamp(result, a_clampLow, a_clampHigh);
				pDestinationLine[w] = (T)result;
			}
			pDestinationBase += a_destinationStride;
		}

		free(pPaddedImage);
	}
}

#endif // RESAMPLE_H_INCLUDED
