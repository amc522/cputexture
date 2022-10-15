#include <cputex/converter.h>
#include <cputex/texture_operations.h>

namespace cputex {
    Converter::Converter() {}

    Converter::Converter(Converter &&other) noexcept
        : mBlockSampler(std::move(other.mBlockSampler))
        , mWriter(std::move(other.mWriter))
    {
    }

    Converter::Converter(gpufmt::Format sourceFormat, gpufmt::Format destFormat) noexcept
        : mBlockSampler(sourceFormat)
        , mWriter(destFormat)
    {
    }

    Converter::~Converter() {}

    Converter &Converter::operator=(Converter &&other) noexcept {
        mBlockSampler = std::move(other.mBlockSampler);
        mWriter = std::move(other.mWriter);

        return *this;
    }

    cputex::UniqueTexture Converter::convert(cputex::SurfaceView source, ConvertError &error) const noexcept {
        if(mBlockSampler.format() != source.format()) {
            error = ConvertError::SourceFormatsMismatch;
            return {};
        }

        const gpufmt::FormatInfo &destInfo = gpufmt::formatInfo(mWriter.format());
        
        if(!destInfo.writeable) {
            error = ConvertError::FormatNotWriteable;
        }

        cputex::TextureParams params;
        params.arraySize = 1;
        params.dimension = source.dimension();
        params.extent = source.extent();
        params.faces = 1;
        params.format = mWriter.format();
        params.mips = 1;
        params.surfaceByteAlignment = kDefaultSurfaceByteAlignment;//source.surfaceByteAlignment();

        cputex::UniqueTexture destTexture{ params };
        
        error = convertTo(source, (SurfaceSpan)destTexture.accessMipSurface());
        
        if(error == ConvertError::None) {
            return destTexture;
        } else {
            return {};
        }
    }

    cputex::UniqueTexture Converter::convert(cputex::TextureView source, ConvertError &error) const noexcept {
        if(mBlockSampler.format() != source.format()) {
            error = ConvertError::SourceFormatsMismatch;
            return {};
        }

        const gpufmt::FormatInfo &destInfo = gpufmt::formatInfo(mWriter.format());

        if(!destInfo.writeable) {
            error = ConvertError::FormatNotWriteable;
        }

        cputex::TextureParams params;
        params.arraySize = 1;
        params.dimension = source.dimension();
        params.extent = source.extent();
        params.faces = 1;
        params.format = mWriter.format();
        params.mips = 1;
        params.surfaceByteAlignment = source.surfaceByteAlignment();

        cputex::UniqueTexture destTexture{ params };

        error = convertTo(source, static_cast<cputex::TextureSpan>(destTexture));

        if(error == ConvertError::None) {
            return destTexture;
        } else {
            return {};
        }
    }

    cputex::ConvertError Converter::convertTo(cputex::SurfaceView source, cputex::SurfaceSpan dest) const noexcept {
        if(mBlockSampler.format() != source.format()) {
            return ConvertError::SourceFormatsMismatch;
        }

        if(mWriter.format() != dest.format()) {
            return ConvertError::DestinationFormatsMismatch;
        }

        const gpufmt::FormatInfo &sourceInfo = gpufmt::formatInfo(mBlockSampler.format());
        const gpufmt::FormatInfo &destInfo = gpufmt::formatInfo(mWriter.format());

        if(sourceInfo.decompressible &&
           (sourceInfo.decompressedFormat == dest.format() || sourceInfo.decompressedFormatAlt == dest.format()))
        {
            if(decompressSurfaceTo(source, dest)) {
                return cputex::ConvertError::None;
            }
        }

        if(!destInfo.writeable) {
            return ConvertError::FormatNotWriteable;
        }

        if(source.extent() != dest.extent()) {
            return ConvertError::SourceAndDestinationNotEquivalent;
        }

        cputex::Extent sourceBlockExtent = mBlockSampler.blockExtent();
        cputex::Extent surfaceBlockExtent = (source.extent() + (sourceBlockExtent - ExtentComponent(1))) / sourceBlockExtent;

        std::vector<gpufmt::SampleVariant> samples;
        samples.resize(mBlockSampler.blockTexelCount());
    
        gpufmt::span<const cputex::byte> sourceSpan = source.getData();

        gpufmt::Surface<const cputex::byte> blockSurface;
        blockSurface.blockData = sourceSpan;
        blockSurface.extentInBlocks = surfaceBlockExtent;

        cputex::SizeType sourceByteOffset = 0;
        for(ExtentComponent zBlock = 0; zBlock < surfaceBlockExtent.z; ++zBlock) {
            for(ExtentComponent yBlock = 0; yBlock < surfaceBlockExtent.y; ++yBlock) {
                for(ExtentComponent xBlock = 0; xBlock < surfaceBlockExtent.x; ++xBlock) {
                    gpufmt::BlockSampleError error = mBlockSampler.variantSampleTo(blockSurface, { xBlock, yBlock, zBlock }, samples);
                    sourceByteOffset += sourceInfo.blockByteSize;

                    switch(error)
                    {
                    case gpufmt::BlockSampleError::None:
                        break;
                    case gpufmt::BlockSampleError::SourceTooSmall:
                        return ConvertError::SourceTooSmall;
                    case gpufmt::BlockSampleError::DestinationTooSmall:
                        return ConvertError::DestinationTooSmall;
                    case gpufmt::BlockSampleError::DepthStencilUnsupported:
                        return ConvertError::DepthStencilUnsupported;
                    case gpufmt::BlockSampleError::InvalidFormat:
                        return ConvertError::InvalidFormat;
                    default:
                        break;
                    }

                    gpufmt::Extent destTexel{ xBlock * sourceBlockExtent.x, yBlock * sourceBlockExtent.y, zBlock * sourceBlockExtent.z };
                    SizeType destTexelOffset = destTexel.z * (dest.extent().y * dest.extent().x) + destTexel.y * dest.extent().x + destTexel.x;
                    gpufmt::span<gpufmt::byte> destSpan = dest.accessDataAs<gpufmt::byte>();
                    destSpan = destSpan.subspan(destTexelOffset * destInfo.blockByteSize);

                    cputex::SizeType byteOffset = 0;
                    for(ExtentComponent y = 0; y < sourceBlockExtent.y; ++y) {
                        for(ExtentComponent x = 0; x < sourceBlockExtent.x; ++x) {
                            gpufmt::WriteError writeError = mWriter.writeTo(samples[y * sourceBlockExtent.x + x], destSpan.subspan(byteOffset, destInfo.blockByteSize));

                            switch(writeError)
                            {
                            case gpufmt::WriteError::None:
                                break;
                            case gpufmt::WriteError::FormatNotWriteable:
                                return ConvertError::FormatNotWriteable;
                            case gpufmt::WriteError::DestinationTooSmall:
                                return ConvertError::DestinationTooSmall;
                            default:
                                break;
                            }

                            byteOffset += destInfo.blockByteSize;
                        }

                        byteOffset += destInfo.blockByteSize * (dest.extent().x - sourceBlockExtent.x);
                    }
                }
            }
        }

        return ConvertError::None;
    }

    cputex::ConvertError Converter::convertTo(cputex::TextureView source, cputex::TextureSpan dest) const noexcept {
        if(mBlockSampler.format() != source.format()) {
            return ConvertError::SourceFormatsMismatch;
        }

        if(mWriter.format() != dest.format()) {
            return ConvertError::DestinationFormatsMismatch;
        }

        const gpufmt::FormatInfo &destInfo = gpufmt::formatInfo(mWriter.format());

        if(!destInfo.writeable) {
            return ConvertError::FormatNotWriteable;
        }

        if(!source.equivalentDimensions(dest)) {
            return ConvertError::SourceAndDestinationNotEquivalent;
        }

        for(CountType arraySlice = 0; arraySlice < dest.arraySize(); ++arraySlice) {
            for(CountType face = 0; face < dest.faces(); ++face) {
                for(CountType mip = 0; mip < dest.mips(); ++mip) {
                    ConvertError error = convertTo((SurfaceView)source.getMipSurface(arraySlice, face, mip),
                                                   (SurfaceSpan)dest.accessMipSurface(arraySlice, face, mip));

                    if(error != ConvertError::None) {
                        return error;
                    }
                }
            }
        }

        return ConvertError::None;
    }
}