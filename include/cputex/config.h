#pragma once

#if __has_include(<span>)
#include <span>

namespace cputex {
    template<class T, size_t Extent = std::dynamic_extent>
    using span = std::span<T, Extent>;
}
#else
#include <span.hpp>
#include <gpufmt/config.h>

namespace cputex {
    using namespace tcb;

    using byte = gpufmt::byte;
}
#endif