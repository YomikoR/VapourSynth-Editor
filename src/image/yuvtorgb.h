#ifndef YUVTORGB_H_INCLUDED
#define YUVTORGB_H_INCLUDED

#include <cstdint>

#include "../common/helpers.h"

namespace vsedit
{

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

	class AbstractYuvToRgbConverter
	{
		public:

			virtual ~AbstractYuvToRgbConverter();

			RGB32 yuvToRgb32(uint8_t a_y, uint8_t a_u, uint8_t a_v);
			RGB32 yuvToRgb32(uint16_t a_y, uint16_t a_u, uint16_t a_v,
				int a_bitsPerSample);
			RGB32 yuvToRgb32(float a_y, float a_u, float a_v);
			RGB32 yuvToRgb32(FP16 a_y, FP16 a_u, FP16 a_v);

		protected:

			uint16_t m_ay; // Luma lower boundary
			float m_ky1;
			float m_ku1;
			float m_kv1;
			float m_ky2;
			float m_ku2;
			float m_kv2;
			float m_ky3;
			float m_ku3;
			float m_kv3;
	};

	class YuvToRgbConverterBt601 : public AbstractYuvToRgbConverter
	{
		public:
			YuvToRgbConverterBt601();
	};

	class YuvToRgbConverterBt709 : public AbstractYuvToRgbConverter
	{
		public:
			YuvToRgbConverterBt709();
	};

	class YuvToRgbConverterFullRange : public AbstractYuvToRgbConverter
	{
		public:
			YuvToRgbConverterFullRange();
	};

}

#endif // YUVTORGB_H_INCLUDED
