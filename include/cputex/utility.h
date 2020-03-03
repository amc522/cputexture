#pragma once

#include <cputex/config.h>
#include <cputex/definitions.h>

namespace cputex {
    [[nodiscard]]
    inline uint32_t maxMips(const cputex::Extent &extent) noexcept {
        const uint32_t maxExtent(std::max(extent.x, std::max(extent.y, extent.z)));

        return static_cast<uint32_t>(std::log2(maxExtent)) + 1u;
    }

    template<class T>
    [[nodiscard]]
    inline uint32_t maxMips(T extent) noexcept {
        return static_cast<uint32_t>(std::log2(extent)) + T(1);
    }
}