#include "yuvtorgb.h"

//==============================================================================

// Matrix for full range YUV
const float vsedit::ky1 = 1.0f;
const float vsedit::ku1 = 0.0f;
const float vsedit::kv1 = 1.4f;

const float vsedit::ky2 = 1.0f;
const float vsedit::ku2 = -0.343f;
const float vsedit::kv2 = -0.711f;

const float vsedit::ky3 = 1.0f;
const float vsedit::ku3 = 1.765f;
const float vsedit::kv3 = 0.0f;

//==============================================================================

vsedit::RGB32 vsedit::yuvToRgb32(uint8_t a_y, uint8_t a_u, uint8_t a_v)
{
	float r = (float)a_y + vsedit::kv1 * ((float)a_v - 128.0f);
	float g = (float)a_y + vsedit::ku2 * ((float)a_u - 128.0f) +
		vsedit::kv2 * ((float)a_v - 128.0f);
	float b = (float)a_y + vsedit::ku3 * ((float)a_u - 128.0f);

	clamp(r, 0.0f, 255.0f);
	clamp(g, 0.0f, 255.0f);
	clamp(b, 0.0f, 255.0f);

	vsedit::RGB32 rgbOut;
	rgbOut.parts.r = r;
	rgbOut.parts.g = g;
	rgbOut.parts.b = b;
	rgbOut.parts.x = 255;
	return rgbOut;
}

// END OF vsedit::RGB32 vsedit::yuvToRgb32(uint8_t a_y, uint8_t a_u,
//		uint8_t a_v)
//==============================================================================

vsedit::RGB32 vsedit::yuvToRgb32(uint16_t a_y, uint16_t a_u, uint16_t a_v,
	int a_bitsPerSample)
{
	float maxValue = (((uint32_t)1) << a_bitsPerSample) - 1;
	float midValue = ((uint32_t)1) << (a_bitsPerSample - 1);
	float delimiter = ((uint32_t)1) << (a_bitsPerSample - 8);

	float r = (float)a_y + vsedit::kv1 * ((float)a_v - midValue);
	float g = (float)a_y + vsedit::ku2 * ((float)a_u - midValue) +
		vsedit::kv2 * ((float)a_v - midValue);
	float b = (float)a_y + vsedit::ku3 * ((float)a_u - midValue);

	clamp(r, 0.0f, maxValue);
	clamp(g, 0.0f, maxValue);
	clamp(b, 0.0f, maxValue);

	vsedit::RGB32 rgbOut;
	rgbOut.parts.r = r / delimiter;
	rgbOut.parts.g = g / delimiter;
	rgbOut.parts.b = b / delimiter;
	rgbOut.parts.x = 255;
	return rgbOut;
}

// END OF vsedit::RGB32 vsedit::yuvToRgb32(uint16_t a_y, uint16_t a_u,
//		uint16_t a_v, int a_bitsPerSample)
//==============================================================================

vsedit::RGB32 vsedit::yuvToRgb32(float a_y, float a_u, float a_v)
{
	float r = a_y + vsedit::kv1 * a_v;
	float g = a_y + vsedit::ku2 * a_u + vsedit::kv2 * a_v;
	float b = a_y + vsedit::ku3 * a_u;

	clamp(r, 0.0f, 1.0f);
	clamp(g, 0.0f, 1.0f);
	clamp(b, 0.0f, 1.0f);

	vsedit::RGB32 rgbOut;
	rgbOut.parts.r = r * 255.0f;
	rgbOut.parts.g = g * 255.0f;
	rgbOut.parts.b = b * 255.0f;
	rgbOut.parts.x = 255;
	return rgbOut;
}

// END OF vsedit::RGB32 vsedit::yuvToRgb32(float a_y, float a_u, float a_v)
//==============================================================================

vsedit::RGB32 vsedit::yuvToRgb32(vsedit::FP16 a_y, vsedit::FP16 a_u,
	vsedit::FP16 a_v)
{
	vsedit::FP32 l_y = halfToSingle(a_y);
	vsedit::FP32 l_u = halfToSingle(a_u);
	vsedit::FP32 l_v = halfToSingle(a_v);

	return vsedit::yuvToRgb32(l_y.f, l_u.f, l_v.f);
}

// END OF vsedit::RGB32 vsedit::yuvToRgb32(vsedit::FP16 a_y, vsedit::FP16 a_u,
//		vsedit::FP16 a_v)
//==============================================================================
