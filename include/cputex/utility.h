#pragma once

#include <cputex/config.h>
#include <cputex/definitions.h>

namespace cputex {
    template<class T>
    [[nodiscard]]
    constexpr cputex::CountType calcuateMaxMipCount(T extent) noexcept {
        cputex::CountType mipCount = 0;
        while(extent > 0)
        {
            extent >>= 1;
            ++mipCount;
        }

        return mipCount;
    }

    [[nodiscard]]
    constexpr cputex::CountType calcuateMaxMipCount(const cputex::Extent &extent) noexcept {
        cputex::CountType maxComponent(std::max(extent.x, std::max(extent.y, extent.z)));

        return calcuateMaxMipCount(maxComponent);
    }

    [[nodiscard]]
    constexpr cputex::Extent nextSmallestMipExtent(const cputex::Extent& extent, cputex::Extent minExtent = {1, 1, 1}) noexcept {
        return {
            std::max(extent.x >> 1, minExtent.x),
            std::max(extent.y >> 1, minExtent.y),
            std::max(extent.z >> 1, minExtent.z),
        };
    }

    [[nodiscard]]
    constexpr cputex::Extent nextLargestMipExtent(const cputex::Extent& extent) noexcept {
        return {
            std::min(extent.x << 1, cputex::maxExtent.x),
            std::min(extent.y << 1, cputex::maxExtent.y),
            std::min(extent.z << 1, cputex::maxExtent.z)
        };
    }

    [[nodiscard]]
    constexpr cputex::Extent calculateMipExtent(const cputex::Extent& extent, cputex::CountType mip, cputex::Extent minExtent = {1, 1, 1}) noexcept {
        return {                               
            std::max(extent.x >> mip, minExtent.x),
            std::max(extent.y >> mip, minExtent.y),
            std::max(extent.z >> mip, minExtent.z),
        };
    }
}