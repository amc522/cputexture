#include "cputex/texture_operations.h"
#include "cputex/config.h"

#include <gpufmt/storage.h>
#include <gpufmt/traits.h>

namespace cputex {
    template<gpufmt::Format FormatV>
    class Clear {
    public:
        void operator()(cputex::TextureSurfaceSpan surface, [[maybe_unused]] const glm::dvec4 &clearColor) noexcept {
            using Traits = gpufmt::FormatTraits<FormatV>;
            using Storage = gpufmt::FormatStorage<FormatV>;

            if constexpr(Traits::info.compression == gpufmt::CompressionType::None &&
                         !Traits::info.depth &&
                         !Traits::info.stencil &&
                         FormatV != gpufmt::Format::UNDEFINED)
            {
                span<cputex::byte> surfaceSpan = surface.accessData();
                span<typename Traits::BlockType> surfaceBlockSpan(reinterpret_cast<typename Traits::BlockType *>(surfaceSpan.data()), surfaceSpan.size_bytes() / Traits::BlockByteSize);

                typename Traits::WideSampleType wideClearColor(clearColor);
                typename Traits::BlockType nativeClearColor = Storage::storeBlock(span<typename Traits::WideSampleType, Traits::BlockTexelCount>(&wideClearColor, 1u));

                for(typename Traits::BlockType &block : surfaceBlockSpan) {
                    block = nativeClearColor;
                }
            }
        }
    };

    void clear(cputex::TextureSurfaceSpan surface, const glm::dvec4 &clearColor) noexcept {
        gpufmt::visitFormat<Clear>(surface.format(), surface, clearColor);
    }

    void clear(cputex::TextureSpan texture, const glm::dvec4 &clearColor) noexcept {
        for(CountType arraySlice = 0; arraySlice < texture.arraySize(); ++arraySlice) {
            for(CountType face = 0; face < texture.faces(); ++face) {
                for(CountType mip = 0; mip < texture.mips(); ++mip) {
                    clear(texture.accessMipSurface(arraySlice, face, mip), clearColor);
                }
            }
        }
    }


    template<gpufmt::Format FormatV>
    class HorizontalFlip {
    public:
        bool operator()(span<const cputex::byte> sourceSurface, span<cputex::byte> destSurface, const Extent &surfaceExtent) noexcept
        {
            using Traits = gpufmt::FormatTraits<FormatV>;

            if constexpr(Traits::info.compression != gpufmt::CompressionType::None ||
                         Traits::info.blockExtent.x > 1 || Traits::info.blockExtent.y > 1 || Traits::info.blockExtent.z > 1 ||
                         std::is_void_v<Traits::BlockType>)
            {
                return false;
            }
            else
            {
                span<const Traits::BlockType> sourceTexelSpan{ reinterpret_cast<const Traits::BlockType *>(sourceSurface.data()), sourceSurface.size_bytes() / sizeof(Traits::BlockType) };
                span<Traits::BlockType> destTexelSpan{ reinterpret_cast<Traits::BlockType *>(destSurface.data()), destSurface.size_bytes() / sizeof(Traits::BlockType) };

                using ExtentValueType = decltype(surfaceExtent.x);

                ExtentValueType topRow{ 0 };
                ExtentValueType bottomRow = surfaceExtent.y - ExtentValueType{ 1 };

                while(topRow < bottomRow) {
                    span<const Traits::BlockType> sourceTopRowSpan = sourceTexelSpan.subspan(static_cast<size_t>(topRow) * static_cast<size_t>(surfaceExtent.x), surfaceExtent.x);
                    span<const Traits::BlockType> sourceBottomRowSpan = sourceTexelSpan.subspan(static_cast<size_t>(bottomRow) * static_cast<size_t>(surfaceExtent.x), surfaceExtent.x);

                    span<Traits::BlockType> destTopRowSpan = destTexelSpan.subspan(static_cast<size_t>(topRow) * static_cast<size_t>(surfaceExtent.x), surfaceExtent.x);
                    span<Traits::BlockType> destBottomRowSpan = destTexelSpan.subspan(static_cast<size_t>(bottomRow) * static_cast<size_t>(surfaceExtent.x), surfaceExtent.x);

                    for(ExtentValueType column{ 0u }; column < surfaceExtent.x; ++column) {
                        const Traits::BlockType sourceTopValue = sourceTopRowSpan[column];
                        const Traits::BlockType sourceBottomValue = sourceBottomRowSpan[column];

                        destTopRowSpan[column] = sourceBottomValue;
                        destBottomRowSpan[column] = sourceTopValue;
                    }

                    ++topRow;
                    --bottomRow;
                }

                return true;
            }
        }
    };

    bool flipHorizontal(cputex::SurfaceSpan surface) noexcept {
        const cputex::Extent extent = surface.extent();
        for(CountType volumeSlice = 0; volumeSlice < extent.z; ++volumeSlice) {
            auto surfaceSlice = surface.accessVolumeSlice(volumeSlice);
            bool result = gpufmt::visitFormat<HorizontalFlip>(surface.format(),
                                                              surfaceSlice.getData(),
                                                              surfaceSlice.accessData(),
                                                              surfaceSlice.extent());
            
            if(!result) {
                return false;
            }
        }

        return true;
    }

    bool flipHorizontal(cputex::TextureSpan texture) noexcept {
        for(CountType arraySlice = 0u; arraySlice < texture.arraySize(); ++arraySlice) {
            for(CountType face = 0; face < texture.faces(); ++face) {
                for(CountType mip = 0u; mip < texture.mips(); ++mip) {
                    const bool result = flipHorizontal((SurfaceSpan)texture.accessMipSurface(arraySlice, face, mip));

                    if(!result) {
                        return false;
                    }
                }
            }
        }

        return true;
    }

    bool flipHorizontalTo(cputex::SurfaceView sourceSurface, cputex::SurfaceSpan destSurface) noexcept
    {
        if(!sourceSurface.equivalentLayout(destSurface)) {
            return false;
        }

        const cputex::Extent extent = sourceSurface.extent();

        for(CountType volumeSlice = 0; volumeSlice < extent.z; ++volumeSlice) {

            bool result = gpufmt::visitFormat<HorizontalFlip>(sourceSurface.format(),
                                                              sourceSurface.getVolumeSlice(volumeSlice).getData(),
                                                              destSurface.accessVolumeSlice(volumeSlice).accessData(),
                                                              cputex::Extent(extent.x, extent.y, 1));

            if(!result) {
                return false;
            }
        }

        return true;
    }

    bool flipHorizontalTo(cputex::TextureView sourceTexture, cputex::TextureSpan destTexture) noexcept {
        if(!sourceTexture.equivalentLayout(destTexture)) {
            return false;
        }

        for(CountType arraySlice = 0u; arraySlice < sourceTexture.arraySize(); ++arraySlice) {
            for(CountType face = 0; face < sourceTexture.faces(); ++face) {
                for(CountType mip = 0u; mip < sourceTexture.mips(); ++mip) {
                    const bool result = flipHorizontalTo((SurfaceView)sourceTexture.getMipSurface(arraySlice, face, mip), (SurfaceSpan)destTexture.accessMipSurface(arraySlice, face, mip));

                    if(!result) {
                        return false;
                    }
                }
            }
        }

        return true;
    }

    template<gpufmt::Format FormatV>
    class VerticalFlip {
    public:
        bool operator()(span<const cputex::byte> sourceSurface, span<cputex::byte> destSurface, const Extent &surfaceExtent) noexcept
        {
            using Traits = gpufmt::FormatTraits<FormatV>;

            if constexpr(Traits::info.compression != gpufmt::CompressionType::None ||
                         Traits::info.blockExtent.x > 1 || Traits::info.blockExtent.y > 1 || Traits::info.blockExtent.z > 1 ||
                         std::is_void_v<Traits::BlockType>)
            {
                return false;
            }
            else
            {
                span<const Traits::BlockType> sourceTexelSpan{ reinterpret_cast<const Traits::BlockType *>(sourceSurface.data()), sourceSurface.size_bytes() / sizeof(Traits::BlockType) };
                span<Traits::BlockType> destTexelSpan{ reinterpret_cast<Traits::BlockType *>(destSurface.data()), destSurface.size_bytes() / sizeof(Traits::BlockType) };

                using ExtentValueType = decltype(surfaceExtent.x);

                for(ExtentValueType row{ 0 }; row < surfaceExtent.y; ++row) {
                    span<const Traits::BlockType> sourceRowSpan = sourceTexelSpan.subspan(static_cast<size_t>(row) * static_cast<size_t>(surfaceExtent.x));
                    span<Traits::BlockType> destRowSpan = destTexelSpan.subspan(static_cast<size_t>(row) * static_cast<size_t>(surfaceExtent.x));

                    ExtentValueType leftColumn{ 0 };
                    ExtentValueType rightColumn{ surfaceExtent.x - ExtentValueType{1} };

                    while(leftColumn < rightColumn) {
                        const Traits::BlockType sourceLeftValue = sourceRowSpan[leftColumn];
                        const Traits::BlockType sourceRightValue = sourceRowSpan[rightColumn];

                        destRowSpan[rightColumn] = sourceLeftValue;
                        destRowSpan[leftColumn] = sourceRightValue;

                        ++leftColumn;
                        --rightColumn;
                    }
                }

                return true;
            }
        }
    };

    bool flipVertical(cputex::SurfaceSpan surface) noexcept
    {
        const cputex::Extent extent = surface.extent();
        for(CountType volumeSlice = 0; volumeSlice < extent.z; ++volumeSlice) {
            bool result = gpufmt::visitFormat<VerticalFlip>(surface.format(), surface.getData(), surface.accessData(), Extent(extent.x, extent.y, 1));

            if(!result) {
                return false;
            }
        }

        return true;
    }

    bool flipVertical(cputex::TextureSpan texture) noexcept {
        for(CountType arraySlice = 0u; arraySlice < texture.arraySize(); ++arraySlice) {
            for(CountType face = 0; face < texture.faces(); ++face) {
                for(CountType mip = 0u; mip < texture.mips(); ++mip) {
                    const bool result = flipVertical((SurfaceSpan)texture.accessMipSurface(arraySlice, face, mip));
                    
                    if(!result) {
                        return false;
                    }
                }
            }
        }

        return true;
    }

    bool flipVerticalTo(cputex::SurfaceView sourceSurface, cputex::SurfaceSpan destSurface) noexcept
    {
        if(!sourceSurface.equivalentLayout(destSurface)) {
            return false;
        }

        const cputex::Extent extent = sourceSurface.extent();
        for(CountType volumeSlice = 0; volumeSlice < extent.z; ++volumeSlice) {
            bool result = gpufmt::visitFormat<VerticalFlip>(sourceSurface.format(), sourceSurface.getData(), destSurface.accessData(), Extent(extent.x, extent.y, 1));

            if(!result) {
                return false;
            }
        }

        return true;
    }

    bool flipVerticalTo(cputex::TextureView sourceTexture, cputex::TextureSpan destTexture) noexcept {
        if(!sourceTexture.equivalentLayout(destTexture)) {
            return false;
        }

        for(CountType arraySlice = 0u; arraySlice < sourceTexture.arraySize(); ++arraySlice) {
            for(CountType face = 0; face < sourceTexture.faces(); ++face) {
                for(CountType mip = 0u; mip < sourceTexture.mips(); ++mip) {
                    const bool result = flipVerticalTo((SurfaceView)sourceTexture.getMipSurface(arraySlice, face, mip), (SurfaceSpan)destTexture.accessMipSurface(arraySlice, face, mip));

                    if(!result) {
                        return false;
                    }
                }
            }
        }

        return true;
    }

    template<gpufmt::Format FormatV>
    class RegionCopy {
    public:
        [[nodiscard]]
        bool operator()(cputex::SurfaceView sourceSurface, cputex::Extent sourceOffset, cputex::SurfaceSpan destSurface, cputex::Extent destOffset, cputex::Extent copyExtent) const noexcept {
            using Traits = gpufmt::FormatTraits<FormatV>;

            if constexpr(!std::is_void_v<Traits::BlockType>)
            {
                if((sourceOffset.x + copyExtent.x) >= sourceSurface.extent().x ||
                   (sourceOffset.y + copyExtent.y) >= sourceSurface.extent().y ||
                   (sourceOffset.z + copyExtent.z) >= sourceSurface.extent().z)
                {
                    return false;
                }

                if((destOffset.x + copyExtent.x) >= destSurface.extent().x ||
                   (destOffset.y + copyExtent.y) >= destSurface.extent().y ||
                   (destOffset.z + copyExtent.z) >= destSurface.extent().z)
                {
                    return false;
                }

                if((copyExtent.x % Traits::BlockExtent.x) != 0 ||
                   (copyExtent.y % Traits::BlockExtent.y) != 0 ||
                   (copyExtent.z % Traits::BlockExtent.z) != 0)
                {
                    return false;
                }

                if((sourceOffset.x % Traits::BlockExtent.x) != 0 ||
                   (sourceOffset.y % Traits::BlockExtent.y) != 0 ||
                   (sourceOffset.z % Traits::BlockExtent.z) != 0)
                {
                    return false;
                }

                if((destOffset.x % Traits::BlockExtent.x) != 0 ||
                   (destOffset.y % Traits::BlockExtent.y) != 0 ||
                   (destOffset.z % Traits::BlockExtent.z) != 0)
                {
                    return false;
                }

                const auto sourceData = sourceSurface.getDataAs<Traits::BlockType>();
                auto destData = destSurface.accessDataAs<Traits::BlockType>();

                const auto sourceBlockOffset = cputex::Extent{ sourceOffset.x / Traits::BlockExtent.x,
                    sourceOffset.y / Traits::BlockExtent.y,
                    sourceOffset.z / Traits::BlockExtent.z };

                const auto destBlockOffset = cputex::Extent{ destOffset.x / Traits::BlockExtent.x,
                    destOffset.y / Traits::BlockExtent.y,
                    destOffset.z / Traits::BlockExtent.z };

                const auto blockCopyExtent = cputex::Extent{ copyExtent.x / Traits::BlockExtent.x,
                    copyExtent.y / Traits::BlockExtent.y,
                    copyExtent.z / Traits::BlockExtent.z };

                for(CountType arraySlice = 0; arraySlice < blockCopyExtent.z; ++arraySlice) {
                    for(CountType row = 0; row < blockCopyExtent.y; ++row) {
                        const auto sourceRow = sourceData.subspan((arraySlice + sourceBlockOffset.z) * (static_cast<size_t>(blockCopyExtent.y) * static_cast<size_t>(blockCopyExtent.x)) +
                                                                  (row + sourceBlockOffset.y) * blockCopyExtent.x, blockCopyExtent.x);

                        auto destRow = destData.subspan((arraySlice + destBlockOffset.z) * (static_cast<size_t>(blockCopyExtent.y) * static_cast<size_t>(blockCopyExtent.x)) +
                                                        (row + destBlockOffset.y) * blockCopyExtent.x, blockCopyExtent.x);

                        for(CountType column = 0; column < blockCopyExtent.x; ++column) {
                            const auto sourceValue = sourceRow[sourceBlockOffset.x + column];
                            destRow[destBlockOffset.x + column] = sourceValue;
                        }
                    }
                }

                return true;
            }
            else
            {
                return false;
            }
        }
    };

    bool copySurfaceRegionTo(cputex::SurfaceView sourceSurface, cputex::Extent sourceOffset, cputex::SurfaceSpan destSurface, cputex::Extent destOffset, cputex::Extent copyExtent) noexcept {
        if(sourceSurface.format() != destSurface.format()) {
            return false;
        }

        return gpufmt::visitFormat<RegionCopy>(sourceSurface.format(), sourceSurface, sourceOffset, destSurface, destOffset, copyExtent);
    }

    template<gpufmt::Format FormatV>
    class Decompressor {
    public:
        using CompressedTraits = gpufmt::FormatTraits<FormatV>;
        using DecompressedTraits = gpufmt::FormatTraits<CompressedTraits::info.decompressedFormat>;
        using DecompressedTraitsAlt = gpufmt::FormatTraits<CompressedTraits::info.decompressedFormatAlt>;
        using Storage = gpufmt::FormatStorage<FormatV>;

        gpufmt::DecompressError operator()(cputex::SurfaceView compressedSurface, cputex::SurfaceSpan decompressedSurface) const noexcept {
            if constexpr(!Storage::Decompressible) {
                return gpufmt::DecompressError::FormatNotDecompressible;
            }
            else {
                gpufmt::Surface<const CompressedTraits::BlockType> compressedBlockSurface;
                compressedBlockSurface.blockData = compressedSurface.getDataAs<CompressedTraits::BlockType>();
                compressedBlockSurface.extentInBlocks = (compressedSurface.extent() + (CompressedTraits::BlockExtent - Extent{ 1, 1, 1 })) / CompressedTraits::BlockExtent;

                if(decompressedSurface.format() == CompressedTraits::info.decompressedFormat) {
                    gpufmt::Surface<DecompressedTraits::BlockType> decompressedBlockSurface;
                    decompressedBlockSurface.blockData = decompressedSurface.accessDataAs<DecompressedTraits::BlockType>();
                    decompressedBlockSurface.extentInBlocks = decompressedSurface.extent();

                    return Storage::decompress(compressedBlockSurface, decompressedBlockSurface);
                }
                else if(compressedSurface.format() == CompressedTraits::info.decompressedFormatAlt) {
                    if constexpr(DecompressedTraitsAlt::info.decompressedFormatAlt != gpufmt::Format::UNDEFINED) {
                        gpufmt::Surface<DecompressedTraitsAlt::BlockType> decompressedBlockSurface;
                        decompressedBlockSurface.blockData = decompressedSurface.accessDataAs<DecompressedTraitsAlt::BlockType>();
                        decompressedBlockSurface.extentInBlocks = decompressedSurface.extent();

                        return Storage::decompress(compressedBlockSurface, decompressedBlockSurface);
                    }
                    else {
                        return gpufmt::DecompressError::FormatNotDecompressible;
                    }
                }
                else {
                    return gpufmt::DecompressError::FormatNotDecompressible;
                }
            }
        }
    };

    bool decompressSurfaceTo(cputex::SurfaceView sourceSurface, cputex::SurfaceSpan destSurface) noexcept {
        const gpufmt::FormatInfo &info = gpufmt::formatInfo(sourceSurface.format());

        if(!info.decompressible) {
            return false;
        }

        if(destSurface.format() != info.decompressedFormat &&
           destSurface.format() != info.decompressedFormatAlt)
        {
            return false;
        }

        if(!sourceSurface.equivalentDimensions(destSurface)) {
            return false;
        }

        gpufmt::DecompressError error = gpufmt::visitFormat<Decompressor>(sourceSurface.format(), sourceSurface, destSurface);
        return error == gpufmt::DecompressError::None;
    }

    bool decompressTextureTo(cputex::TextureView sourceTexture, cputex::TextureSpan destTexture) noexcept {
        const gpufmt::FormatInfo &info = gpufmt::formatInfo(sourceTexture.format());

        if(!info.decompressible) {
            return false;
        }

        if(destTexture.format() != info.decompressedFormat &&
           destTexture.format() != info.decompressedFormatAlt)
        {
            return false;
        }

        if(!sourceTexture.equivalentDimensions(destTexture)) {
            return false;
        }

        for(CountType arraySlice = 0; arraySlice < sourceTexture.arraySize(); ++arraySlice) {
            for(CountType face = 0; face < sourceTexture.faces(); ++face) {
                for(CountType mip = 0; mip < sourceTexture.mips(); ++mip) {
                    if(!decompressSurfaceTo((SurfaceView)sourceTexture.getMipSurface(arraySlice, face, mip), (SurfaceSpan)destTexture.accessMipSurface(arraySlice, face, mip))) {
                        return false;
                    }
                }
            }
        }

        return true;
    }
}