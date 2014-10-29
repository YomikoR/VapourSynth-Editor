#include "yuvtorgb.h"

//==============================================================================

vsedit::AbstractYuvToRgbConverter::~AbstractYuvToRgbConverter()
{
}

// END OF vsedit::AbstractYuvToRgbConverter::~AbstractYuvToRgbConverter()
//==============================================================================

vsedit::RGB32 vsedit::AbstractYuvToRgbConverter::yuvToRgb32(uint8_t a_y,
	uint8_t a_u, uint8_t a_v)
{
	float l_y = a_y - m_ay;
	float l_u = a_u - 128;
	float l_v = a_v - 128;

	float r = l_y * m_ky1 + l_u * m_ku1 + l_v * m_kv1;
	float g = l_y * m_ky2 + l_u * m_ku2 + l_v * m_kv2;
	float b = l_y * m_ky3 + l_u * m_ku3 + l_v * m_kv3;

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

// END OF vsedit::RGB32 vsedit::AbstractYuvToRgbConverter::yuvToRgb32(
//		uint8_t a_y, uint8_t a_u, uint8_t a_v)
//==============================================================================

vsedit::RGB32 vsedit::AbstractYuvToRgbConverter::yuvToRgb32(uint16_t a_y,
	uint16_t a_u, uint16_t a_v, int a_bitsPerSample)
{
	uint16_t maxValue = (((uint32_t)1) << a_bitsPerSample) - 1;
	uint16_t midValue = ((uint32_t)1) << (a_bitsPerSample - 1);
	float delimiter = (float)maxValue / 255.0f;

	float l_y = a_y - (m_ay << (a_bitsPerSample - 8));
	float l_u = a_u - midValue;
	float l_v = a_v - midValue;

	float r = l_y * m_ky1 + l_u * m_ku1 + l_v * m_kv1;
	float g = l_y * m_ky2 + l_u * m_ku2 + l_v * m_kv2;
	float b = l_y * m_ky3 + l_u * m_ku3 + l_v * m_kv3;

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

// END OF vsedit::RGB32 vsedit::AbstractYuvToRgbConverter::yuvToRgb32(
//		uint16_t a_y, uint16_t a_u, uint16_t a_v, int a_bitsPerSample)
//==============================================================================

vsedit::RGB32 vsedit::AbstractYuvToRgbConverter::yuvToRgb32(float a_y,
	float a_u, float a_v)
{
	float r = a_y * m_ky1 + a_u * m_ku1 + a_v * m_kv1;
	float g = a_y * m_ky2 + a_u * m_ku2 + a_v * m_kv2;
	float b = a_y * m_ky3 + a_u * m_ku3 + a_v * m_kv3;

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

// END OF vsedit::RGB32 vsedit::AbstractYuvToRgbConverter::yuvToRgb32(float a_y,
//		float a_u, float a_v)
//==============================================================================

vsedit::RGB32 vsedit::AbstractYuvToRgbConverter::yuvToRgb32(vsedit::FP16 a_y,
	vsedit::FP16 a_u, vsedit::FP16 a_v)
{
	vsedit::FP32 l_y = halfToSingle(a_y);
	vsedit::FP32 l_u = halfToSingle(a_u);
	vsedit::FP32 l_v = halfToSingle(a_v);

	return vsedit::AbstractYuvToRgbConverter::yuvToRgb32(l_y.f, l_u.f, l_v.f);
}

// END OF vsedit::RGB32 vsedit::AbstractYuvToRgbConverter::yuvToRgb32(
//		vsedit::FP16 a_y, vsedit::FP16 a_u, vsedit::FP16 a_v)
//==============================================================================

vsedit::YuvToRgbConverterBt601::YuvToRgbConverterBt601():
	AbstractYuvToRgbConverter()
	{
		m_ay = 16u,
		m_ky1 = 1.164f; m_ku1 =  0.000f; m_kv1 =  1.596f;
		m_ky2 = 1.164f; m_ku2 = -0.392f; m_kv2 = -0.813f;
		m_ky3 = 1.164f; m_ku3 =  2.017f; m_kv3 =  0.000f;
	}

vsedit::YuvToRgbConverterBt709::YuvToRgbConverterBt709():
	AbstractYuvToRgbConverter()
	{
		m_ay = 16u;
		m_ky1 = 1.164f; m_ku1 =  0.000f; m_kv1 =  1.793f;
		m_ky2 = 1.164f; m_ku2 = -0.213f; m_kv2 = -0.533f;
		m_ky3 = 1.164f; m_ku3 =  2.112f; m_kv3 =  0.000f;
	}

vsedit::YuvToRgbConverterFullRange::YuvToRgbConverterFullRange():
	AbstractYuvToRgbConverter()
	{
		m_ay = 0u;
		m_ky1 = 1.000f; m_ku1 =  0.000f; m_kv1 =  1.400f;
		m_ky2 = 1.000f; m_ku2 = -0.343f; m_kv2 = -0.711f;
		m_ky3 = 1.000f; m_ku3 =  1.765f; m_kv3 =  0.000f;
	}

//==============================================================================
