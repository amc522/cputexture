#pragma once

#include <cputex/config.h>
#include <glm/vec3.hpp>
#include <gpufmt/format.h>
#include <gpufmt/definitions.h>

namespace cputex {
    using Extent = gpufmt::Extent;
    using ExtentComponent = gpufmt::ExtentComponent;
    using Format = gpufmt::Format;

    enum class TextureDimension {
        Texture1D,
        Texture2D,
        Texture3D,
        TextureCube
    };

    struct TextureParams {
        gpufmt::Format format = gpufmt::Format::UNDEFINED;
        TextureDimension dimension = TextureDimension::Texture2D;
        Extent extent;
        cputex::CountType arraySize = 0u;
        cputex::CountType faces = 0u;
        cputex::CountType mips = 0u;
        cputex::CountType surfaceByteAlignment = cputex::kDefaultSurfaceByteAlignment;
    };

    constexpr cputex::ExtentComponent maxExtentComponent = 16384;
    constexpr cputex::Extent maxExtent = { maxExtentComponent, maxExtentComponent, maxExtentComponent };
    constexpr cputex::CountType maxMipCount = 15;
}