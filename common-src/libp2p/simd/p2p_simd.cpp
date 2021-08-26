#ifdef P2P_SIMD

#include <array>
#include <typeinfo>
#include <tuple>
#include <utility>
#include "../p2p.h"
#include "cpuinfo_x86.h"
#include "p2p_simd.h"

namespace P2P_NAMESPACE {
namespace detail {

namespace {

typedef std::pair<const std::type_info *, detail::unpack_func> unpack_table_entry;
typedef std::tuple<const std::type_info *, detail::pack_func, detail::pack_func> pack_table_entry;

auto populate_unpack_table()
{
	std::array<unpack_table_entry, 100> table;
	size_t idx = 0;

#if defined(__x86_64__) || defined(__i386__) || defined(_M_IX86) || defined(_M_X64)
	simd::X86Capabilities x86 = simd::query_x86_capabilities();

	if (x86.sse41) {
#define ENTRY(format, cpu) table[idx++] = unpack_table_entry{ &typeid(packed_##format), simd::unpack_##format##_##cpu }
		ENTRY(argb32_be, sse41);
		ENTRY(argb32_le, sse41);
		ENTRY(rgba32_be, sse41);
		ENTRY(rgba32_le, sse41);
#undef ENTRY
	}
#endif

	return table;
}

auto populate_pack_table()
{
	std::array<pack_table_entry, 100> table;
	size_t idx = 0;

#if defined(__x86_64__) || defined(__i386__) || defined(_M_IX86) || defined(_M_X64)
	simd::X86Capabilities x86 = simd::query_x86_capabilities();

	if (x86.sse41) {
#define ENTRY(format, cpu) table[idx++] = pack_table_entry{ &typeid(packed_##format), simd::pack_##format##_0_##cpu, simd::pack_##format##_1_##cpu }
		ENTRY(argb32_be, sse41);
		ENTRY(argb32_le, sse41);
		ENTRY(rgba32_be, sse41);
		ENTRY(rgba32_le, sse41);
#undef ENTRY
	}
#endif

	return table;
}

} // namespace


unpack_func search_unpack_func(const std::type_info &ti)
{
	static const auto g_unpack_table = populate_unpack_table();

	for (const auto &entry : g_unpack_table) {
		if (entry.first == &ti)
			return entry.second;
		if (!entry.first)
			break;
	}
	return nullptr;
}

pack_func search_pack_func(const std::type_info &ti, bool alpha_one_fill)
{
	static const auto g_pack_table = populate_pack_table();

	for (const auto &entry : g_pack_table) {
		if (std::get<0>(entry) == &ti)
			return alpha_one_fill ? std::get<2>(entry) : std::get<1>(entry);
		if (!std::get<0>(entry))
			break;
	}

	return nullptr;
}

} // namespace detail
} // namespace p2p

#endif // P2P_SIMD
