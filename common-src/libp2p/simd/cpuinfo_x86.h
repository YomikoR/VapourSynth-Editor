#pragma once

#ifndef P2P_CPUINFO_X86_H_
#define P2P_CPUINFO_X86_H_

#ifdef P2P_SIMD
#if defined(__x86_64__) || defined(__i386__) || defined(_M_IX86) || defined(_M_X64)

#include "../p2p.h"

namespace P2P_NAMESPACE {
namespace simd {

/**
 * Bitfield of selected x86 feature flags.
 */
struct X86Capabilities {
	unsigned sse : 1;
	unsigned sse2 : 1;
	unsigned sse3 : 1;
	unsigned ssse3 : 1;
	unsigned fma : 1;
	unsigned sse41 : 1;
	unsigned sse42 : 1;
	unsigned avx : 1;
	unsigned f16c : 1;
	unsigned avx2 : 1;
	unsigned avx512f : 1;
	unsigned avx512dq : 1;
	unsigned avx512ifma : 1;
	unsigned avx512cd : 1;
	unsigned avx512bw : 1;
	unsigned avx512vl : 1;
	unsigned avx512vbmi : 1;
	unsigned avx512vbmi2 : 1;
	unsigned avx512vnni : 1;
	unsigned avx512bitalg : 1;
	unsigned avx512vpopcntdq : 1;
	unsigned avx512vp2intersect : 1;
	unsigned avx512fp16 : 1;
	unsigned avx512bf16 : 1;
	/* AMD architectures needing workarounds. */
	unsigned xop : 1;
	unsigned piledriver : 1;
	unsigned zen1 : 1;
	unsigned zen2 : 1;
	unsigned zen3 : 1;
};

/**
 * Representation of processor cache topology.
 */
struct X86CacheHierarchy {
	unsigned long l1d;
	unsigned long l1d_threads;
	unsigned long l2;
	unsigned long l2_threads;
	unsigned long l3;
	unsigned long l3_threads;
	bool l2_inclusive;
	bool l3_inclusive;
	bool valid;
};

/**
 * Get the x86 feature flags on the current CPU.
 *
 * @return capabilities
 */
X86Capabilities query_x86_capabilities() noexcept;

/**
 * Get the cache topology of the current CPU.
 *
 * On a multi-processor system, the returned topology corresponds to the first
 * processor package on which the function is called. The behaviour is
 * undefined if the platform contains non-identical processors.
 *
 * @return cache hierarchy
 */
X86CacheHierarchy query_x86_cache_hierarchy() noexcept;

unsigned long cpu_cache_size_x86() noexcept;

} // namespace simd
} // namespace p2p

#endif // x86
#endif // P2P_SIMD

#endif // P2P_CPUINFO_X86_H_
