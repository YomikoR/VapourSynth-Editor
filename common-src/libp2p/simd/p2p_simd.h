#pragma once

#ifndef P2P_SIMD_H_
#define P2P_SIMD_H_

#ifdef P2P_SIMD

#include "../p2p.h"

namespace P2P_NAMESPACE {
namespace simd {

#define UNPACK(format, cpu) void unpack_##format##_##cpu(const void *, void * const *, unsigned, unsigned);
#define PACK(format, cpu) \
  void pack_##format##_0_##cpu(const void * const *, void *, unsigned, unsigned); \
  void pack_##format##_1_##cpu(const void * const *, void *, unsigned, unsigned);

#if defined(__x86_64__) || defined(__i386__) || defined(_M_IX86) || defined(_M_X64)
UNPACK(argb32_be, sse41)
UNPACK(argb32_le, sse41)
UNPACK(rgba32_be, sse41)
UNPACK(rgba32_le, sse41)

PACK(argb32_be, sse41)
PACK(argb32_le, sse41)
PACK(rgba32_be, sse41)
PACK(rgba32_le, sse41)
#endif // x86

#undef PACK
#undef UNPACK

} // namespace simd
} // namespace p2p

#endif // P2P_SIMD

#endif // P2P_SIMD_H_
