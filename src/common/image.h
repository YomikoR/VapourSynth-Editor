#ifndef IMAGE_H_INCLUDED
#define IMAGE_H_INCLUDED

#include <cstdint>
#include <cmath>

namespace vsedit
{
	//--------------------------------------------------------------------------
	// Half to single precision float conversion
	// by Fabian "ryg" Giesen.

	union FP32
	{
		uint32_t u;
		float f;
		struct
		{
			unsigned int Mantissa : 23;
			unsigned int Exponent : 8;
			unsigned int Sign : 1;
		} parts;
	};

	union FP16
	{
		uint16_t u;
		struct
		{
			unsigned int Mantissa : 10;
			unsigned int Exponent : 5;
			unsigned int Sign : 1;
		} parts;
	};

	FP32 halfToSingle(FP16 a_half);

	//--------------------------------------------------------------------------

	union RGB32
	{
		uint32_t u;
		struct
		{
			uint8_t b;
			uint8_t g;
			uint8_t r;
			uint8_t x;
		} parts;
	};

	extern const float ky1;
	extern const float ku1;
	extern const float kv1;
	extern const float ky2;
	extern const float ku2;
	extern const float kv2;
	extern const float ky3;
	extern const float ku3;
	extern const float kv3;

	RGB32 yuvToRgb32(uint8_t a_y, uint8_t a_u, uint8_t a_v);
	RGB32 yuvToRgb32(uint16_t a_y, uint16_t a_u, uint16_t a_v,
		int a_bitsPerSample);
	RGB32 yuvToRgb32(float a_y, float a_u, float a_v);
	RGB32 yuvToRgb32(FP16 a_y, FP16 a_u, FP16 a_v);

	//--------------------------------------------------------------------------

	union YUY2
	{
		uint32_t u;
		struct
		{
			uint8_t y0;
			uint8_t u;
			uint8_t y1;
			uint8_t v;
		} parts;
	};

	//--------------------------------------------------------------------------

	template<typename T>
	class DoSample
	{
		public:

			DoSample(const T * a_cpSource, size_t a_sourceWidth,
				size_t a_sourceHeight, ptrdiff_t a_sourceStride):
				m_cpSource(a_cpSource)
				, m_sourceWidth(a_sourceWidth)
				, m_sourceHeight(a_sourceHeight)
				, m_sourceStride(a_sourceStride)
			{};

			T operator()(ptrdiff_t a_x, ptrdiff_t a_y)
			{
				ptrdiff_t l_x = a_x;
				clamp(l_x, 0, (ptrdiff_t)m_sourceWidth - 1);
				ptrdiff_t l_y = a_y;
				clamp(l_y, 0, (ptrdiff_t)m_sourceHeight - 1);

				const uint8_t * cpSourceBase = (const uint8_t *)m_cpSource;
				cpSourceBase += m_sourceStride * l_y;
				const T * cpLine = (const T *)cpSourceBase;
				return cpLine[l_x];
			};

			void row(T * a_pDestination, ptrdiff_t a_xFirst, size_t a_length,
				ptrdiff_t a_y)
			{
				ptrdiff_t l_y = a_y;
				clamp(l_y, 0, (ptrdiff_t)m_sourceHeight - 1);

				const uint8_t * cpSourceBase = (const uint8_t *)m_cpSource;
				cpSourceBase += m_sourceStride * l_y;
				const T * cpLine = (const T *)cpSourceBase;

				ptrdiff_t destinationFirst = std::max(0, -a_xFirst);
				ptrdiff_t copyLength = std::min(a_length - destinationFirst,
					m_sourceWidth - a_xFirst);
				ptrdiff_t sourceFirst = std::max(0, a_xFirst);

				memcpy(a_pDestination + destinationFirst, cpLine + sourceFirst,
					copyLength * sizeof(T));

				for(ptrdiff_t i = 0; i < destinationFirst; ++i)
					a_pDestination[i] = cpLine[0];

				for(ptrdiff_t i = destinationFirst + copyLength;
					i < a_length; ++i)
					a_pDestination[i] = cpLine[m_sourceWidth - 1];
			};

		private:

			const T * m_cpSource;
			size_t m_sourceWidth;
			size_t m_sourceHeight;
			ptrdiff_t m_sourceStride;
	};

	//--------------------------------------------------------------------------

	template<typename T>
	void bilinearResize(const T * a_cpSource, size_t a_sourceWidth,
		size_t a_sourceHeight, ptrdiff_t a_sourceStride,
		T * a_pDestination, size_t a_destinationWidth,
		size_t a_destinationHeight, ptrdiff_t a_destinationStride)
	{
		DoSample<T> sample(a_cpSource, a_sourceWidth, a_sourceHeight,
			a_sourceStride);

		uint8_t * pDestinationBase = (uint8_t *)a_pDestination;
		float kx = (float)a_sourceWidth / (float)a_destinationWidth;
		float ky = (float)a_sourceHeight / (float)a_destinationHeight;

		float sx;
		float sy;
		float wx;
		float wy;

		T * pDestinationLine;
		for(size_t h = 0; h < a_destinationHeight; ++h)
		{
			sy = (float)h * ky - 0.25f;
			wy = sy - (float)(int)sy;
			pDestinationLine = (T *)pDestinationBase;
			for(size_t w = 0; w < a_destinationWidth; ++w)
			{
				sx = (float)w * kx - 0.25f;
				wx = sx - (float)(int)sx;
				pDestinationLine[w] =
					(1.0f - wy) * (1.0f - wx) *
					sample(std::floor(sx), std::floor(sy)) +
					(1.0f - wy) * wx *
					sample(std::ceil(sx), std::floor(sy)) +
					wy * (1.0f - wx) *
					sample(std::floor(sx), std::ceil(sy)) +
					wy * wx *
					sample(std::ceil(sx), std::ceil(sy));
			}
			pDestinationBase += a_destinationStride;
		}
	}

	//--------------------------------------------------------------------------

	float bicubicWeight(float a_distance, float a_alpha);

	template<typename T>
	void bicubicResize(const T * a_cpSource, size_t a_sourceWidth,
		size_t a_sourceHeight, ptrdiff_t a_sourceStride,
		T * a_pDestination, size_t a_destinationWidth,
		size_t a_destinationHeight, ptrdiff_t a_destinationStride,
		T a_clampLow, T a_clampHigh)
	{
		const float a = 0.5f;
		DoSample<T> sample(a_cpSource, a_sourceWidth, a_sourceHeight,
			a_sourceStride);

		uint8_t * pDestinationBase = (uint8_t *)a_pDestination;
		float kx = (float)a_sourceWidth / (float)a_destinationWidth;
		float ky = (float)a_sourceHeight / (float)a_destinationHeight;

		float sx;
		float sy;
		float wx[4];
		float wy[4];
		T x[4];
		float row;
		float result;

		T * pDestinationLine;
		for(size_t h = 0; h < a_destinationHeight; ++h)
		{
			sy = (float)h * ky - 0.25f;
			for(ptrdiff_t i = 0; i < 4; ++i)
				wy[i] = bicubicWeight(sy - std::floor(sy + i - 1), a);

			pDestinationLine = (T *)pDestinationBase;
			for(size_t w = 0; w < a_destinationWidth; ++w)
			{
				sx = (float)w * kx - 0.25f;
				for(ptrdiff_t i = 0; i < 4; ++i)
					wx[i] = bicubicWeight(sx - std::floor(sx + i - 1), a);

				result = 0.0f;
				for(ptrdiff_t i = 0; i < 4; ++i)
				{
					sample.row(x, std::floor(sx - 1), 4,
						std::floor(sy + i - 1));
					row = 0;
					for(ptrdiff_t j = 0; j < 4; ++j)
						row += wx[j] * x[j];
					result += wy[i] * row;
				}

				clamp(result, (float)a_clampLow, (float)a_clampHigh);

				pDestinationLine[w] = (T)result;
			}
			pDestinationBase += a_destinationStride;
		}
	}
}

#endif // IMAGE_H_INCLUDED
