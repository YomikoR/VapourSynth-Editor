#ifndef ZIMG_HELPERS_H_INCLUDED
#define ZIMG_HELPERS_H_INCLUDED

#include <zimg.h>
#include <vapoursynth/VapourSynth.h>

#include <limits>
#include <cassert>

//==============================================================================

template <class T, class U>
T range_check_integer(U a_x, const char * a_cpKey)
{
	(void)a_cpKey;

	T min = std::numeric_limits<T>::min();
	T max = std::numeric_limits<T>::max();

	if(a_x < min)
	{
		assert(false);
		return min;
	}

	if(a_x > max)
	{
		assert(false);
		return max;
	}

	return static_cast<T>(a_x);
}

//------------------------------------------------------------------------------

template <class T>
T propGetScalar(const VSMap * a_cpMap, const char * a_cpKey,
	const VSAPI * a_cpVSAPI);

//------------------------------------------------------------------------------

template <class T>
T propGetScalarDef(const VSMap * a_cpMap, const char * a_cpKey, T a_default,
	const VSAPI * a_cpVSAPI)
{
	if(a_cpVSAPI->propNumElements(a_cpMap, a_cpKey) > 0)
		return propGetScalar<T>(a_cpMap, a_cpKey, a_cpVSAPI);
	else
		return a_default;
}

//------------------------------------------------------------------------------

template <class T, class U, class Pred>
void propGetIfValid(const VSMap * a_cpMap, const char * a_cpKey, U * a_pOut,
	Pred a_predicate, const VSAPI * a_cpVSAPI)
{
	if(a_cpVSAPI->propNumElements(a_cpMap, a_cpKey) > 0)
	{
		T x = propGetScalar<T>(a_cpMap, a_cpKey, a_cpVSAPI);
		if(a_predicate(x))
			*a_pOut = static_cast<U>(x);
	}
}

//==============================================================================

bool operator==(const zimg_image_format & a, const zimg_image_format & b);
bool operator!=(const zimg_image_format & a, const zimg_image_format & b);

//==============================================================================

struct YUY2UnpackData
{
	const uint8_t * pYUY2Buffer;
	ptrdiff_t yuy2BufferStride;
	zimg_image_buffer * pZimgBuffer;
};

int unpackYUY2(void * a_pUser, unsigned a_i, unsigned a_left,
	unsigned a_right);

struct BGR32PackData
{
	zimg_image_buffer_const * cpZimgBuffer;
	uint8_t * pBGR32Buffer;
	ptrdiff_t bgr32BufferStride;
};

int packBGR32(void * a_pUser, unsigned a_i, unsigned a_left,
	unsigned a_right);

//==============================================================================

#endif // ZIMG_HELPERS_H_INCLUDED
