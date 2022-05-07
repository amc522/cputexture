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
        cputex::CountType blockTexelCount() const noexcept;

        [[nodiscard]]
        gpufmt::SampleVariant sample(glm::vec3 uvCoords, cputex::CountType arraySlice = 0, cputex::CountType face = 0, cputex::CountType mip = 0) const noexcept;

        [[nodiscard]]
        gpufmt::SampleVariant sample(glm::vec3 uvCoords, cputex::span<gpufmt::SampleVariant> blockSamples, cputex::CountType arraySlice = 0, cputex::CountType face = 0, cputex::CountType mip = 0) const noexcept;

        [[nodiscard]]
        gpufmt::SampleVariant load(cputex::Extent texel, cputex::CountType arraySlice = 0, cputex::CountType face = 0, cputex::CountType mip = 0) const noexcept;

        [[nodiscard]]
        gpufmt::SampleVariant load(cputex::Extent texel, cputex::span<gpufmt::SampleVariant> blockSamples, cputex::CountType arraySlice = 0, cputex::CountType face = 0, cputex::CountType mip = 0) const noexcept;

    private:
        TextureView mTexture;
        gpufmt::BlockSampler mSampler;
    };
}