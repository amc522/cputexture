#pragma once

#include <cputex/config.h>
#include <cputex/definitions.h>

namespace cputex {
    [[nodiscard]]
    inline cputex::CountType maxMips(const cputex::Extent &extent) noexcept {
        const cputex::CountType maxExtent(std::max(extent.x, std::max(extent.y, extent.z)));

        return static_cast<cputex::CountType>(std::log2(maxExtent)) + cputex::CountType(1);
    }

    template<class T>
    [[nodiscard]]
    inline cputex::CountType maxMips(T extent) noexcept {
        return static_cast<cputex::CountType>(std::log2(extent)) + cputex::CountType(1);
    }
}