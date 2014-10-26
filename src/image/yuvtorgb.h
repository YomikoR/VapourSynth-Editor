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

}

#endif // YUVTORGB_H_INCLUDED
