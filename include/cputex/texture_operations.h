#pragma once

#include <cputex/texture_view.h>

namespace cputex {
    void clear(cputex::TextureSurfaceSpan surface, const glm::dvec4 &clearColor = { 0.0, 0.0, 0.0, 1.0 }) noexcept;
    void clear(cputex::TextureSpan texture, const glm::dvec4 &clearColor = { 0.0, 0.0, 0.0, 1.0 }) noexcept;

    bool flipHorizontal(cputex::SurfaceSpan surface) noexcept;
    bool flipHorizontal(cputex::TextureSpan texture) noexcept;
    bool flipHorizontalTo(cputex::SurfaceView sourceSurface, cputex::SurfaceSpan destSurface) noexcept;
    bool flipHorizontalTo(cputex::TextureView sourceTexture, cputex::TextureSpan destTexture) noexcept;

    bool flipVertical(cputex::SurfaceSpan surface) noexcept;
    bool flipVertical(cputex::TextureSpan texture) noexcept;
    bool flipVerticalTo(cputex::SurfaceView sourceSurface, cputex::SurfaceSpan destSurface) noexcept;
    bool flipVerticalTo(cputex::TextureView sourceTexture, cputex::TextureSpan destTexture) noexcept;

    bool copySurfaceRegionTo(cputex::SurfaceView sourceSurface, cputex::Extent sourceOffset, cputex::SurfaceSpan destSurface, cputex::Extent destOffset, cputex::Extent copyExtent) noexcept;

    bool decompressSurfaceTo(cputex::SurfaceView sourceSurface, cputex::SurfaceSpan destSurface) noexcept;
    bool decompressTextureTo(cputex::TextureView sourceTexture, cputex::TextureSpan destTexture) noexcept;

    template<class Pred>
    void transform(cputex::SurfaceSpan surface, Pred pred)
    {
        const gpufmt::FormatInfo& formatInfo = gpufmt::formatInfo(surface.format());

        const cputex::Extent surfaceExtent = surface.extent();
        const cputex::Extent surfaceBlockExtent = surfaceExtent / formatInfo.blockExtent;

        std::span surfaceData = surface.accessData();

        for(ExtentComponent zBlock = 0; zBlock < surfaceBlockExtent.z; ++zBlock)
        {
            for(ExtentComponent yBlock = 0; yBlock < surfaceBlockExtent.y; ++yBlock)
            {
                for(ExtentComponent xBlock = 0; xBlock < surfaceBlockExtent.x; ++xBlock)
                {
                    const auto offset = (zBlock * surfaceBlockExtent.y * surfaceBlockExtent.x + yBlock * surfaceBlockExtent.x + xBlock) * formatInfo.blockByteSize;
                    pred(surface.format(), surfaceData.subspan(offset, formatInfo.blockByteSize));
                }
            }
        }
    }

    template<class Pred>
    void transform(cputex::TextureSurfaceSpan surface, Pred pred)
    {
        transform((cputex::SurfaceSpan)surface, std::move(pred));
    }

    template<class Pred>
    void transform(cputex::TextureSpan texture, Pred pred)
    {
        for(CountType arraySlice = 0; arraySlice < texture.arraySize(); ++arraySlice)
        {
            for(CountType face = 0; face < texture.faces(); ++face)
            {
                for(CountType mip = 0; mip < texture.mips(); ++mip)
                {
                    transform(texture.accessMipSurface(arraySlice, face, mip, &pred), [arraySlice, face, mip](gpufmt::Format format, std::span<std::byte> block)
                    {
                        pred(format, arraySlice, face, mip, block);
                    });
                }
            }
        }
    }
}