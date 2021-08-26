#ifdef P2P_SIMD
#if defined(__x86_64__) || defined(__i386__) || defined(_M_IX86) || defined(_M_X64)

#include <cstdint>
#include <smmintrin.h>
#include "../p2p.h"

namespace P2P_NAMESPACE {
namespace simd {

namespace {

template <unsigned Idx>
uint32_t extract_epi32(__m128i x)
{
	return Idx == 0 ? _mm_cvtsi128_si32(x) : _mm_extract_epi32(x, Idx);
}

template <unsigned IdxR, unsigned IdxG, unsigned IdxB, unsigned IdxA>
void unpack_rgb32_sse41(const void *src, void * const * dst, unsigned left, unsigned right)
{
	const __m128i shuffle = _mm_set_epi8(15, 11, 7, 3, 14, 10, 6, 2, 13, 9, 5, 1, 12, 8, 4, 0);

	const uint32_t *src_p = static_cast<const uint32_t *>(src);
	uint8_t *dst_r = static_cast<uint8_t *>(dst[0]);
	uint8_t *dst_g = static_cast<uint8_t *>(dst[1]);
	uint8_t *dst_b = static_cast<uint8_t *>(dst[2]);
	uint8_t *dst_a = static_cast<uint8_t *>(dst[3]);

	if (!dst_a)
		dst_a = dst_r; // Write alpha to some other channel if disabled.

	size_t vec4_left = (left + 3) & ~3U;
	size_t vec16_left = (left + 15) & ~15U;
	size_t vec16_right = right & ~15U;
	size_t vec4_right = right & ~3U;

	// Must always write alpha component first!
	auto scalar_iter = [&](size_t i)
	{
		uint32_t x = src_p[i];
		dst_a[i] = static_cast<uint8_t>((x >> (IdxA * 8)) & 0xFFU);
		dst_r[i] = static_cast<uint8_t>((x >> (IdxR * 8)) & 0xFFU);
		dst_g[i] = static_cast<uint8_t>((x >> (IdxG * 8)) & 0xFFU);
		dst_b[i] = static_cast<uint8_t>((x >> (IdxB * 8)) & 0xFFU);
	};
	auto vec4_iter = [&](size_t i)
	{
		__m128i x = _mm_loadu_si128((const __m128i *)(src_p + i));
		x = _mm_shuffle_epi8(x, shuffle);

		*reinterpret_cast<uint32_t *>(dst_a + i) = extract_epi32<IdxA>(x);
		*reinterpret_cast<uint32_t *>(dst_r + i) = extract_epi32<IdxR>(x);
		*reinterpret_cast<uint32_t *>(dst_g + i) = extract_epi32<IdxG>(x);
		*reinterpret_cast<uint32_t *>(dst_b + i) = extract_epi32<IdxB>(x);
	};
	auto vec16_iter = [&](size_t i)
	{
		__m128i x0 = _mm_loadu_si128((const __m128i *)(src_p + i));
		__m128i x1 = _mm_loadu_si128((const __m128i *)(src_p + i + 4));
		__m128i x2 = _mm_loadu_si128((const __m128i *)(src_p + i + 8));
		__m128i x3 = _mm_loadu_si128((const __m128i *)(src_p + i + 12));

		x0 = _mm_shuffle_epi8(x0, shuffle);
		x1 = _mm_shuffle_epi8(x1, shuffle);
		x2 = _mm_shuffle_epi8(x2, shuffle);
		x3 = _mm_shuffle_epi8(x3, shuffle);

		__m128 x0s = _mm_castsi128_ps(x0), x1s = _mm_castsi128_ps(x1), x2s = _mm_castsi128_ps(x2), x3s = _mm_castsi128_ps(x3);
		_MM_TRANSPOSE4_PS(x0s, x1s, x2s, x3s);
		x0 = _mm_castps_si128(x0s); x1 = _mm_castps_si128(x1s); x2 = _mm_castps_si128(x2s); x3 = _mm_castps_si128(x3s);

		__m128i regs[4] = { x0, x1, x2, x3 };
		_mm_storeu_si128((__m128i *)(dst_a + i), regs[IdxA]);
		_mm_storeu_si128((__m128i *)(dst_r + i), regs[IdxR]);
		_mm_storeu_si128((__m128i *)(dst_g + i), regs[IdxG]);
		_mm_storeu_si128((__m128i *)(dst_b + i), regs[IdxB]);
	};

	for (size_t i = left; i < vec4_left; ++i)
		scalar_iter(i);
	for (size_t i = vec4_left; i < vec16_left; i += 4)
		vec4_iter(i);
	for (size_t i = vec16_left; i < vec16_right; i += 16)
		vec16_iter(i);
	for (size_t i = vec16_right; i < vec4_right; i += 4)
		vec4_iter(i);
	for (size_t i = vec4_right; i < right; ++i)
		scalar_iter(i);
}

template <unsigned IdxR, unsigned IdxG, unsigned IdxB, unsigned IdxA, bool AlphaOneFill>
void pack_rgb32_sse41(const void * const *src, void *dst, unsigned left, unsigned right)
{
#define X (AlphaOneFill ? 0xFF : 0)
	alignas(16) static constexpr uint8_t alpha_fill[16] = { X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X };
#undef X
	const __m128i shuffle = _mm_set_epi8(15, 11, 7, 3, 14, 10, 6, 2, 13, 9, 5, 1, 12, 8, 4, 0);

	const uint8_t *src_r = static_cast<const uint8_t *>(src[0]);
	const uint8_t *src_g = static_cast<const uint8_t *>(src[1]);
	const uint8_t *src_b = static_cast<const uint8_t *>(src[2]);
	const uint8_t *src_a = static_cast<const uint8_t *>(src[3]);
	size_t alpha_addr_mask = ~static_cast<size_t>(0);
	uint32_t *dst_p = static_cast<uint32_t *>(dst);

	size_t vec4_left = (left + 3) & ~3U;
	size_t vec16_left = (left + 15) & ~15U;
	size_t vec16_right = right & ~15U;
	size_t vec4_right = right & ~3U;

	if (!src_a) {
		src_a = alpha_fill;
		alpha_addr_mask = 15;
	}

	auto scalar_iter = [&](size_t i)
	{
		uint8_t r = src_r[i];
		uint8_t g = src_g[i];
		uint8_t b = src_b[i];
		uint8_t a = src_a[i & alpha_addr_mask];

		uint32_t val = (static_cast<uint32_t>(r) << (IdxR * 8)) |
			(static_cast<uint32_t>(g) << (IdxG * 8)) |
			(static_cast<uint32_t>(b) << (IdxB * 8)) |
			(static_cast<uint32_t>(a) << (IdxA * 8));
		dst_p[i] = val;
	};
	auto vec4_iter = [&](size_t i)
	{
		uint32_t r = *reinterpret_cast<const uint32_t *>(src_r + i);
		uint32_t g = *reinterpret_cast<const uint32_t *>(src_g + i);
		uint32_t b = *reinterpret_cast<const uint32_t *>(src_b + i);
		uint32_t a = *reinterpret_cast<const uint32_t *>(src_a + (i & alpha_addr_mask));

		__m128i x = IdxR == 0 ? _mm_cvtsi32_si128(r) :
			IdxG == 0 ? _mm_cvtsi32_si128(g) :
			IdxB == 0 ? _mm_cvtsi32_si128(b) :
			IdxA == 0 ? _mm_cvtsi32_si128(a) :
				throw 1;

		x = IdxR == 0 ? x : _mm_insert_epi32(x, r, IdxR);
		x = IdxG == 0 ? x : _mm_insert_epi32(x, g, IdxG);
		x = IdxB == 0 ? x : _mm_insert_epi32(x, b, IdxB);
		x = IdxA == 0 ? x : _mm_insert_epi32(x, a, IdxA);
		x = _mm_shuffle_epi8(x, shuffle);
		_mm_storeu_si128((__m128i *)(dst_p + i), x);
	};
	auto vec16_iter = [&](size_t i)
	{
		__m128i r = _mm_loadu_si128((const __m128i *)(src_r + i));
		__m128i g = _mm_loadu_si128((const __m128i *)(src_g + i));
		__m128i b = _mm_loadu_si128((const __m128i *)(src_b + i));
		__m128i a = _mm_loadu_si128((const __m128i *)(src_a + (i & alpha_addr_mask)));

		__m128 regs[4];
		regs[IdxR] = _mm_castsi128_ps(r);
		regs[IdxG] = _mm_castsi128_ps(g);
		regs[IdxB] = _mm_castsi128_ps(b);
		regs[IdxA] = _mm_castsi128_ps(a);
		_MM_TRANSPOSE4_PS(regs[0], regs[1], regs[2], regs[3]);

		__m128i x0 = _mm_castps_si128(regs[0]), x1 = _mm_castps_si128(regs[1]), x2 = _mm_castps_si128(regs[2]), x3 = _mm_castps_si128(regs[3]);
		x0 = _mm_shuffle_epi8(x0, shuffle);
		x1 = _mm_shuffle_epi8(x1, shuffle);
		x2 = _mm_shuffle_epi8(x2, shuffle);
		x3 = _mm_shuffle_epi8(x3, shuffle);

		_mm_storeu_si128((__m128i *)(dst_p + i + 0), x0);
		_mm_storeu_si128((__m128i *)(dst_p + i + 4), x1);
		_mm_storeu_si128((__m128i *)(dst_p + i + 8), x2);
		_mm_storeu_si128((__m128i *)(dst_p + i + 12), x3);
	};

	for (size_t i = left; i < vec4_left; ++i)
		scalar_iter(i);
	for (size_t i = vec4_left; i < vec16_left; i += 4)
		vec4_iter(i);
	for (size_t i = vec16_left; i < vec16_right; i += 16)
		vec16_iter(i);
	for (size_t i = vec16_right; i < vec4_right; i += 4)
		vec4_iter(i);
	for (size_t i = vec4_right; i < right; ++i)
		scalar_iter(i);
}

} // namespace


#define RGB32_SSE41(format, a, b, c, d) \
  void unpack_##format##_sse41(const void *src, void * const * dst, unsigned left, unsigned right) \
  { \
    unpack_rgb32_sse41<a, b, c, d>(src, dst, left, right); \
  } \
  void pack_##format##_0_sse41(const void * const *src, void *dst, unsigned left, unsigned right) \
  { \
    pack_rgb32_sse41<a, b, c, d, 0>(src, dst, left, right); \
  } \
  void pack_##format##_1_sse41(const void * const *src, void *dst, unsigned left, unsigned right) \
  { \
    pack_rgb32_sse41<a, b, c, d, 1>(src, dst, left, right); \
  }

RGB32_SSE41(argb32_be, 1, 2, 3, 0)
RGB32_SSE41(argb32_le, 2, 1, 0, 3)
RGB32_SSE41(rgba32_be, 0, 1, 2, 3)
RGB32_SSE41(rgba32_le, 3, 2, 1, 0)

} // namespace simd
} // namespace p2p

#endif // x86
#endif // P2P_SIMD
