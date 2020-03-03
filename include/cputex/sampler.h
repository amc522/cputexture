#pragma once

#include <cputex/texture_view.h>

#include <gpufmt/sample.h>

namespace cputex {
    class Sampler {
    public:
        Sampler() noexcept = default;
        Sampler(TextureView texture) noexcept;

        [[nodiscard]]
        const cputex::Extent &blockExtent() const noexcept;

        [[nodiscard]]
        size_t blockTexelCount() const noexcept;

        [[nodiscard]]
        gpufmt::SampleVariant sample(glm::vec3 uvCoords, uint32_t arraySlice = 0, uint32_t face = 0, uint32_t mip = 0) const noexcept;

        [[nodiscard]]
        gpufmt::SampleVariant sample(glm::vec3 uvCoords, cputex::span<gpufmt::SampleVariant> blockSamples, uint32_t arraySlice = 0, uint32_t face = 0, uint32_t mip = 0) const noexcept;

        [[nodiscard]]
        gpufmt::SampleVariant load(glm::ivec3 texel, uint32_t arraySlice = 0, uint32_t face = 0, uint32_t mip = 0) const noexcept;

        [[nodiscard]]
        gpufmt::SampleVariant load(glm::ivec3 texel, cputex::span<gpufmt::SampleVariant> blockSamples, uint32_t arraySlice = 0, uint32_t face = 0, uint32_t mip = 0) const noexcept;

    private:
        TextureView mTexture;
        gpufmt::BlockSampler mSampler;
    };
}