#include "cputex/texture_operations.h"
#include "cputex/config.h"

#include <gpufmt/storage.h>
#include <gpufmt/traits.h>

namespace cputex {
    template<gpufmt::Format FormatV>
    class Clear {
    public:
        void operator()(cputex::SurfaceSpan surface, [[maybe_unused]] const glm::dvec4 &clearColor) noexcept {
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

    void clear(cputex::SurfaceSpan surface, const glm::dvec4 &clearColor) noexcept {
        gpufmt::visitFormat<Clear>(surface.format(), surface, clearColor);
    }

    void clear(cputex::TextureSpan texture, const glm::dvec4 &clearColor) noexcept {
        for(uint32_t arraySlice = 0; arraySlice < texture.arraySize(); ++arraySlice) {
            for(uint32_t face = 0; face < texture.faces(); ++face) {
                for(uint32_t mip = 0; mip < texture.mips(); ++mip) {
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

    bool flipHorizontal(cputex::TextureSpan texture) noexcept {
        for(uint32_t arraySlice = 0u; arraySlice < texture.arraySize(); ++arraySlice) {
            for(uint32_t face = 0; face < texture.faces(); ++face) {
                for(uint32_t mip = 0u; mip < texture.mips(); ++mip) {
                    const Extent &mipExtent = texture.extent(mip);

                    for(uint32_t volumeSlice = 0; volumeSlice < mipExtent.z; ++volumeSlice) {

                        auto surface = texture.access2DSurfaceData(arraySlice, face, mip, volumeSlice);
                        bool result = gpufmt::visitFormat<HorizontalFlip>(texture.format(),
                                                                          surface,
                                                                          surface,
                                                                          texture.extent(mip));
                        if(!result) {
                            return false;
                        }
                    }
                }
            }
        }

        return true;
    }

    bool flipHorizontalTo(cputex::TextureView sourceTexture, cputex::TextureSpan destTexture) noexcept {
        if(!sourceTexture.equivalentLayout(destTexture)) {
            return false;
        }

        for(uint32_t arraySlice = 0u; arraySlice < sourceTexture.arraySize(); ++arraySlice) {
            for(uint32_t face = 0; face < sourceTexture.faces(); ++face) {
                for(uint32_t mip = 0u; mip < sourceTexture.mips(); ++mip) {
                    const Extent &mipExtent = sourceTexture.extent(mip);

                    for(uint32_t volumeSlice = 0; volumeSlice < mipExtent.z; ++volumeSlice) {

                        bool result = gpufmt::visitFormat<HorizontalFlip>(sourceTexture.format(),
                                                                          sourceTexture.get2DSurfaceData(arraySlice, face, mip, volumeSlice),
                                                                          destTexture.access2DSurfaceData(arraySlice, face, mip, volumeSlice),
                                                                          sourceTexture.extent(mip));
                        if(!result) {
                            return false;
                        }
                    }
                }
            }
        }

        return true;
    }

    cputex::UniqueTexture flipHorizontalCopy(cputex::TextureView texture) noexcept {
        TextureParams params;
        params.dimension = texture.dimension();
        params.arraySize = texture.arraySize();
        params.faces = texture.faces();
        params.mips = texture.mips();
        params.format = texture.format();
        params.extent = texture.extent();
        params.surfaceByteAlignment = texture.surfaceByteAlignment();

        UniqueTexture destTexture{ params };

        if(!flipHorizontalTo(texture, static_cast<TextureSpan>(destTexture))) {
            return {};
        }

        return destTexture;
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

    bool flipVertical(cputex::TextureSpan texture) noexcept {
        for(uint32_t arraySlice = 0u; arraySlice < texture.arraySize(); ++arraySlice) {
            for(uint32_t face = 0; face < texture.faces(); ++face) {
                for(uint32_t mip = 0u; mip < texture.mips(); ++mip) {
                    const Extent &mipExtent = texture.extent(mip);

                    for(uint32_t volumeSlice = 0u; volumeSlice < mipExtent.z; ++volumeSlice) {
                        auto surface = texture.access2DSurfaceData(arraySlice, face, mip, volumeSlice);
                        bool result = gpufmt::visitFormat<VerticalFlip>(texture.format(),
                                                                        surface,
                                                                        surface,
                                                                        texture.extent(mip));
                        if(!result) {
                            return false;
                        }
                    }
                }
            }
        }

        return true;
    }

    bool flipVerticalTo(cputex::TextureView sourceTexture, cputex::TextureSpan destTexture) noexcept {
        if(!sourceTexture.equivalentLayout(destTexture)) {
            return false;
        }

        for(uint32_t arraySlice = 0u; arraySlice < sourceTexture.arraySize(); ++arraySlice) {
            for(uint32_t face = 0; face < sourceTexture.faces(); ++face) {
                for(uint32_t mip = 0u; mip < sourceTexture.mips(); ++mip) {
                    const Extent &mipExtent = sourceTexture.extent(mip);

                    for(uint32_t volumeSlice = 0u; volumeSlice < mipExtent.z; ++volumeSlice) {
                        bool result = gpufmt::visitFormat<VerticalFlip>(sourceTexture.format(),
                                                                        sourceTexture.get2DSurfaceData(arraySlice, face, mip, volumeSlice),
                                                                        destTexture.access2DSurfaceData(arraySlice, face, mip, volumeSlice),
                                                                        sourceTexture.extent(mip));
                        if(!result) {
                            return false;
                        }
                    }
                }
            }
        }

        return true;
    }

    cputex::UniqueTexture flipVerticalCopy(cputex::TextureView texture) noexcept {
        TextureParams params;
        params.dimension = texture.dimension();
        params.arraySize = texture.arraySize();
        params.faces = texture.faces();
        params.mips = texture.mips();
        params.format = texture.format();
        params.extent = texture.extent();
        UniqueTexture destTexture{ params };

        if(!flipVerticalTo(texture, static_cast<TextureSpan>(destTexture))) {
            return {};
        }

        return destTexture;
    }

    template<gpufmt::Format FormatV>
    class RegionCopy {
    public:
        [[nodiscard]]
        bool operator()(cputex::SurfaceView sourceSurface, glm::uvec3 sourceOffset, cputex::SurfaceSpan destSurface, glm::uvec3 destOffset, cputex::Extent copyExtent) const noexcept {
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

                const auto sourceBlockOffset = glm::uvec3{ sourceOffset.x / Traits::BlockExtent.x,
                    sourceOffset.y / Traits::BlockExtent.y,
                    sourceOffset.z / Traits::BlockExtent.z };

                const auto destBlockOffset = glm::uvec3{ destOffset.x / Traits::BlockExtent.x,
                    destOffset.y / Traits::BlockExtent.y,
                    destOffset.z / Traits::BlockExtent.z };

                const auto blockCopyExtent = glm::uvec3{ copyExtent.x / Traits::BlockExtent.x,
                    copyExtent.y / Traits::BlockExtent.y,
                    copyExtent.z / Traits::BlockExtent.z };

                for(size_t arraySlice = 0; arraySlice < blockCopyExtent.z; ++arraySlice) {
                    for(size_t row = 0; row < blockCopyExtent.y; ++row) {
                        const auto sourceRow = sourceData.subspan((arraySlice + sourceBlockOffset.z) * (static_cast<size_t>(blockCopyExtent.y) * static_cast<size_t>(blockCopyExtent.x)) +
                                                                  (row + sourceBlockOffset.y) * blockCopyExtent.x, blockCopyExtent.x);

                        auto destRow = destData.subspan((arraySlice + destBlockOffset.z) * (static_cast<size_t>(blockCopyExtent.y) * static_cast<size_t>(blockCopyExtent.x)) +
                                                        (row + destBlockOffset.y) * blockCopyExtent.x, blockCopyExtent.x);

                        for(size_t column = 0; column < blockCopyExtent.x; ++column) {
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

    bool copySurfaceRegionTo(cputex::SurfaceView sourceSurface, glm::uvec3 sourceOffset, cputex::SurfaceSpan destSurface, glm::uvec3 destOffset, cputex::Extent copyExtent) noexcept {
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

    cputex::UniqueTexture decompressSurface(cputex::SurfaceView sourceSurface, bool useAltDecompressedFormat) noexcept {
        const gpufmt::FormatInfo &info = gpufmt::formatInfo(sourceSurface.format());

        if(!info.decompressible) {
            return UniqueTexture{};
        }

        cputex::TextureParams params;
        params.arraySize = 1;
        params.dimension = sourceSurface.dimension();
        params.extent = sourceSurface.extent();
        params.faces = 1;
        params.format = (useAltDecompressedFormat && info.decompressedFormatAlt != gpufmt::Format::UNDEFINED) ? info.decompressedFormatAlt : info.decompressedFormat;
        params.mips = 1;
        params.surfaceByteAlignment = sourceSurface.surfaceByteAlignment();

        cputex::UniqueTexture decompressedTexture{ params };
        decompressSurfaceTo(sourceSurface, decompressedTexture.accessMipSurface());

        return decompressedTexture;
    }

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

    cputex::UniqueTexture decompressTexture(cputex::TextureView sourceTexture, bool useAltDecompressedFormat) noexcept {
        const gpufmt::FormatInfo &info = gpufmt::formatInfo(sourceTexture.format());

        if(!info.decompressible) {
            return UniqueTexture{};
        }

        cputex::TextureParams params;
        params.arraySize = sourceTexture.arraySize();
        params.dimension = sourceTexture.dimension();
        params.extent = sourceTexture.extent();
        params.faces = sourceTexture.faces();
        params.format = (useAltDecompressedFormat && info.decompressedFormatAlt != gpufmt::Format::UNDEFINED) ? info.decompressedFormatAlt : info.decompressedFormat;
        params.mips = sourceTexture.mips();
        params.surfaceByteAlignment = sourceTexture.surfaceByteAlignment();

        cputex::UniqueTexture decompressedTexture{ params };

        for(uint32_t arraySlice = 0; arraySlice < sourceTexture.arraySize(); ++arraySlice) {
            for(uint32_t face = 0; face < sourceTexture.faces(); ++face) {
                for(uint32_t mip = 0; mip < sourceTexture.mips(); ++mip) {
                    decompressSurfaceTo(sourceTexture.getMipSurface(arraySlice, face, mip), decompressedTexture.accessMipSurface(arraySlice, face, mip));
                }
            }
        }

        return decompressedTexture;
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

        for(uint32_t arraySlice = 0; arraySlice < sourceTexture.arraySize(); ++arraySlice) {
            for(uint32_t face = 0; face < sourceTexture.faces(); ++face) {
                for(uint32_t mip = 0; mip < sourceTexture.mips(); ++mip) {
                    if(!decompressSurfaceTo(sourceTexture.getMipSurface(arraySlice, face, mip), destTexture.accessMipSurface(arraySlice, face, mip))) {
                        return false;
                    }
                }
            }
        }

        return true;
    }
}