#include "zimg_helpers.h"

//==============================================================================

template <>
int propGetScalar<int>(const VSMap * a_cpMap, const char * a_cpKey,
	const VSAPI * a_cpVSAPI)
{
	int64_t x = a_cpVSAPI->propGetInt(a_cpMap, a_cpKey, 0, nullptr);
	return range_check_integer<int>(x, a_cpKey);
}

//------------------------------------------------------------------------------

template <>
unsigned propGetScalar<unsigned>(const VSMap * a_cpMap, const char * a_cpKey,
	const VSAPI * a_cpVSAPI)
{
	int64_t x = a_cpVSAPI->propGetInt(a_cpMap, a_cpKey, 0, nullptr);
	return range_check_integer<unsigned>(x, a_cpKey);
}

//------------------------------------------------------------------------------

template <>
double propGetScalar<double>(const VSMap * a_cpMap, const char * a_cpKey,
	const VSAPI * a_cpVSAPI)
{
	return a_cpVSAPI->propGetFloat(a_cpMap, a_cpKey, 0, nullptr);
}

//------------------------------------------------------------------------------

template <>
const char * propGetScalar<const char *>(const VSMap * a_cpMap,
	const char * a_cpKey, const VSAPI * a_cpVSAPI)
{
	return a_cpVSAPI->propGetData(a_cpMap, a_cpKey, 0, nullptr);
}

//==============================================================================

bool operator==(const zimg_image_format & a, const zimg_image_format & b)
{
	bool ret = true;

	ret = ret && (a.width == b.width);
	ret = ret && (a.height == b.height);
	ret = ret && (a.pixel_type == b.pixel_type);
	ret = ret && (a.subsample_w == b.subsample_w);
	ret = ret && (a.subsample_h == b.subsample_h);
	ret = ret && (a.color_family == b.color_family);

	if(a.color_family != ZIMG_COLOR_GREY)
	{
		ret = ret && (a.matrix_coefficients == b.matrix_coefficients);
		ret = ret && (a.transfer_characteristics == b.transfer_characteristics);
		ret = ret && (a.color_primaries == b.color_primaries);
	}

	ret = ret && (a.depth == b.depth);
	ret = ret && (a.pixel_range == b.pixel_range);
	ret = ret && (a.field_parity == b.field_parity);

	if((a.color_family == ZIMG_COLOR_YUV) && (a.subsample_w || a.subsample_h))
		ret = ret && (a.chroma_location == b.chroma_location);

	return ret;
}

//------------------------------------------------------------------------------

bool operator!=(const zimg_image_format & a, const zimg_image_format & b)
{
	return !(a == b);
}

//==============================================================================

int unpackYUY2(void * a_pUser, unsigned a_i, unsigned a_left, unsigned a_right)
{
	const YUY2UnpackData * pUnpackData = (const YUY2UnpackData *)a_pUser;

	const uint8_t * yuy2 =
		pUnpackData->pYUY2Buffer + a_i * pUnpackData->yuy2BufferStride;

	uint8_t * planar_y = (uint8_t *)pUnpackData->pZimgBuffer->plane[0].data +
		a_i * pUnpackData->pZimgBuffer->plane[0].stride;
	uint8_t * planar_u = (uint8_t *)pUnpackData->pZimgBuffer->plane[1].data +
		a_i * pUnpackData->pZimgBuffer->plane[1].stride;
	uint8_t * planar_v = (uint8_t *)pUnpackData->pZimgBuffer->plane[2].data +
		a_i * pUnpackData->pZimgBuffer->plane[2].stride;
	unsigned j;

	unsigned left = a_left % 2 ? a_left - 1 : a_left;
	unsigned right = a_right % 2 ? a_right + 1 : a_right;

	for(j = left; j < right; j += 2)
	{
		uint8_t y0, y1, u, v;

		y0 = yuy2[j * 2 + 0];
		u = yuy2[j * 2 + 1];
		y1 = yuy2[j * 2 + 2];
		v = yuy2[j * 2 + 3];

		planar_y[j + 0] = y0;
		planar_y[j + 1] = y1;
		planar_u[j / 2] = u;
		planar_v[j / 2] = v;
	}

	return 0;
}

//------------------------------------------------------------------------------

int packBGR32(void * a_pUser, unsigned a_i, unsigned a_left, unsigned a_right)
{
	BGR32PackData * pPackData = (BGR32PackData *)a_pUser;

	const uint8_t * planar_r = (const uint8_t *)
		pPackData->cpZimgBuffer->plane[0].data +
		a_i * pPackData->cpZimgBuffer->plane[0].stride;
	const uint8_t * planar_g = (const uint8_t *)
		pPackData->cpZimgBuffer->plane[1].data +
		a_i * pPackData->cpZimgBuffer->plane[1].stride;
	const uint8_t * planar_b = (const uint8_t *)
		pPackData->cpZimgBuffer->plane[2].data +
		a_i * pPackData->cpZimgBuffer->plane[2].stride;

	uint8_t * bgr32 =
		pPackData->pBGR32Buffer + a_i * pPackData->bgr32BufferStride;
	unsigned j;

	for(j = a_left; j < a_right; ++j)
	{
		uint8_t r, g, b;

		r = planar_r[j];
		g = planar_g[j];
		b = planar_b[j];

		bgr32[j * 4 + 0] = b;
		bgr32[j * 4 + 1] = g;
		bgr32[j * 4 + 2] = r;
	}

	return 0;
}

//==============================================================================
