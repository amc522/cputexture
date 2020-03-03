#pragma once

#include <cputex/shared_texture.h>
#include <cputex/texture_view.h>
#include <cputex/unique_texture.h>

namespace cputex {
    void clear(cputex::SurfaceSpan surface, const glm::dvec4 &clearColor = { 0.0, 0.0, 0.0, 1.0 }) noexcept;
    void clear(cputex::TextureSpan texture, const glm::dvec4 &clearColor = { 0.0, 0.0, 0.0, 1.0 }) noexcept;

    bool flipHorizontal(cputex::TextureSpan texture) noexcept;
    bool flipHorizontalTo(cputex::TextureView sourceTexture, cputex::TextureSpan destTexture) noexcept;

    [[nodiscard]]
    cputex::UniqueTexture flipHorizontalCopy(cputex::TextureView texture) noexcept;

    bool flipVertical(cputex::TextureSpan texture) noexcept;
    bool flipVerticalTo(cputex::TextureView sourceTexture, cputex::TextureSpan destTexture) noexcept;

    [[nodiscard]]
    cputex::UniqueTexture flipVerticalCopy(cputex::TextureView texture) noexcept;

    bool copySurfaceRegionTo(cputex::SurfaceView sourceSurface, glm::uvec3 sourceOffset, cputex::SurfaceSpan destSurface, glm::uvec3 destOffset, cputex::Extent copyExtent) noexcept;

    [[nodiscard]]
    cputex::UniqueTexture decompressSurface(cputex::SurfaceView sourceSurface, bool useAltDecompressedFormat) noexcept;

    bool decompressSurfaceTo(cputex::SurfaceView sourceSurface, cputex::SurfaceSpan destSurface) noexcept;

    [[nodiscard]]
    cputex::UniqueTexture decompressTexture(cputex::TextureView sourceTexture, bool useAltDecompressedFormat) noexcept;

    bool decompressTextureTo(cputex::TextureView sourceTexture, cputex::TextureSpan destTexture) noexcept;
}