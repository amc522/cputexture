#include "cputex/sampler.h"

#include <cmath>

namespace cputex {
    Sampler::Sampler(TextureView texture) noexcept
        : mTexture(texture)
        , mSampler(texture.format())
    {}

    const cputex::Extent &Sampler::blockExtent() const noexcept {
        return mSampler.blockExtent();
    }

    cputex::CountType Sampler::blockTexelCount() const noexcept {
        return static_cast<cputex::CountType>(mSampler.blockTexelCount());
    }
    
    gpufmt::SampleVariant Sampler::sample(glm::vec3 uvCoords, cputex::CountType arraySlice, cputex::CountType face, cputex::CountType mip) const noexcept {
        std::array<gpufmt::SampleVariant, 144> blockSamples;
        return sample(uvCoords, blockSamples, arraySlice, face, mip);
    }

    gpufmt::SampleVariant Sampler::sample(glm::vec3 uvCoords, cputex::span<gpufmt::SampleVariant> blockSamples, cputex::CountType arraySlice, cputex::CountType face, cputex::CountType mip) const noexcept {
        if(mTexture.empty()) {
            return {};
        }

        uvCoords.x = (uvCoords.x <= 1.0f) ? uvCoords.x : uvCoords.x - std::trunc(uvCoords.x);
        uvCoords.y = (uvCoords.y <= 1.0f) ? uvCoords.y : uvCoords.y - std::trunc(uvCoords.y);
        uvCoords.z = (uvCoords.z <= 1.0f) ? uvCoords.z : uvCoords.z - std::trunc(uvCoords.z);

        const Extent &extent = mTexture.extent(mip);

        gpufmt::Extent texel;
        texel.x = static_cast<int>(std::floor(((extent.x - 1.0f) * uvCoords.x) + 0.5f));
        texel.y = static_cast<int>(std::floor(((extent.y - 1.0f) * uvCoords.y) + 0.5f));
        texel.z = static_cast<int>(std::floor(((extent.z - 1.0f) * uvCoords.z) + 0.5f));

        return load(texel, blockSamples, arraySlice, face, mip);
    }

    gpufmt::SampleVariant Sampler::load(cputex::Extent texel, cputex::CountType arraySlice, cputex::CountType face, cputex::CountType mip) const noexcept {
        std::array<gpufmt::SampleVariant, 144> blockSamples;
        return load(texel, blockSamples, arraySlice, face, mip);
    }

    gpufmt::SampleVariant Sampler::load(cputex::Extent texel, cputex::span<gpufmt::SampleVariant> blockSamples, cputex::CountType arraySlice, cputex::CountType face, cputex::CountType mip) const noexcept {
        if(mTexture.empty()) {
            return {};
        }

        if(blockSamples.size() < blockTexelCount()) {
            return {};
        }

        auto surface = mTexture.get2DSurfaceData(arraySlice, face, mip, texel.z);

        const auto &formatInfo = gpufmt::formatInfo(mTexture.format());
        cputex::Extent bloxel = texel / formatInfo.blockExtent;

        const auto &surfaceExtent = mTexture.extent(mip);
        cputex::Extent surfaceBlockExtent = (surfaceExtent + (formatInfo.blockExtent - cputex::Extent{cputex::ExtentComponent(1), cputex::ExtentComponent(1), cputex::ExtentComponent(1)})) / formatInfo.blockExtent;

        gpufmt::Surface<const cputex::byte> blockSurface;
        blockSurface.blockData = surface;
        blockSurface.extentInBlocks = surfaceBlockExtent;

        gpufmt::BlockSampleError error = mSampler.variantSampleTo(blockSurface, bloxel, blockSamples);

        if(error != gpufmt::BlockSampleError::None) {
            return {};
        }

        cputex::Extent texelInBlock;
        texelInBlock.x = texel.x % formatInfo.blockExtent.x;
        texelInBlock.y = texel.y % formatInfo.blockExtent.y;
        texelInBlock.z = texel.z % formatInfo.blockExtent.z;

        cputex::ExtentComponent texelIndex = texelInBlock.z * (formatInfo.blockExtent.y * formatInfo.blockExtent.x) + texelInBlock.y * formatInfo.blockExtent.x + texelInBlock.x;
        return blockSamples[texelIndex];
    }


}