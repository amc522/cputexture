#pragma once

#include <glm/vec3.hpp>
#include <gpufmt/format.h>
#include <gpufmt/definitions.h>

namespace cputex {
    using Extent = gpufmt::Extent;
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
        uint32_t arraySize = 0u;
        uint32_t faces = 0u;
        uint32_t mips = 0u;
        uint32_t surfaceByteAlignment = 4u;
    };
}