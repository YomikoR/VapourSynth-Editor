#ifndef PADIMAGE_H_INCLUDED
#define PADIMAGE_H_INCLUDED

namespace vsedit
{
	template<typename T>
	T * padImageRepeatBorders(const T * a_cpSource, size_t a_sourceWidth,
		size_t a_sourceHeight, ptrdiff_t a_sourceStride, size_t a_padding)
	{
		size_t paddedWidth = a_sourceWidth + 2 * a_padding;
		size_t paddedHeight = a_sourceHeight + 2 * a_padding;
		size_t sourceLineSize = a_sourceWidth * sizeof(T);

		T * pPaddedImage = (T *)malloc(paddedWidth * paddedHeight * sizeof(T));

		// Blit source image into padded image and fill sides padding
		const uint8_t * cpSourceLine = (const uint8_t *)a_cpSource;
		const T * cpTSourceLine;
		T * pPaddedImageLine = pPaddedImage + a_padding * paddedWidth;
		T * pRightBorder = pPaddedImageLine + a_padding + a_sourceWidth;
		size_t rightIndex = a_sourceWidth - 1;

		for(size_t h = 0; h < a_sourceHeight; ++h)
		{
			cpTSourceLine = (const T *)cpSourceLine;
			memcpy(pPaddedImageLine + a_padding, cpSourceLine, sourceLineSize);
			for(size_t i = 0; i < a_padding; ++i)
			{
				pPaddedImageLine[i] = cpTSourceLine[0];
				pRightBorder[i] = cpTSourceLine[rightIndex];
			}
			cpSourceLine += a_sourceStride;
			pPaddedImageLine += paddedWidth;
			pRightBorder += paddedWidth;
		}

		// Fill top and bottom padding
        T * pTopCopyLine = pPaddedImage + a_padding * paddedWidth;
        T * pBottomCopyLine = pTopCopyLine + (a_sourceHeight - 1) * paddedWidth;
        T * pTopLine = pPaddedImage;
        T * pBottomLine = pBottomCopyLine + paddedWidth;
        size_t lineSize = paddedWidth * sizeof(T);
        for(size_t i = 0; i < a_padding; ++i)
		{
			memcpy(pTopLine, pTopCopyLine, lineSize);
			memcpy(pBottomLine, pBottomCopyLine, lineSize);
			pTopLine += paddedWidth;
			pBottomLine += paddedWidth;
		}

		return pPaddedImage;
	}
}

#endif // PADIMAGE_H_INCLUDED
