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
}