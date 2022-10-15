#pragma once

#include <gpufmt/config.h>

namespace cputex {
    template<class T, size_t Extent = gpufmt::span_dynamic_extent>
    using span = gpufmt::span<T, Extent>;

    using byte = gpufmt::byte;
    using SizeType = std::ptrdiff_t;
    using IndexType = gpufmt::ExtentComponent;
    using CountType = IndexType;

    constexpr CountType kDefaultSurfaceByteAlignment = 4;
}