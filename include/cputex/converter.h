#pragma once

#include <cputex/texture_view.h>
#include <cputex/unique_texture.h>

#include <gpufmt/sample.h>
#include <gpufmt/write.h>

namespace cputex {
    enum class ConvertError {
        None,
        SourceTooSmall,
        DestinationTooSmall,
        DepthStencilUnsupported,
        InvalidFormat,
        FormatNotWriteable,
        SourceFormatsMismatch,
        DestinationFormatsMismatch,
        SourceAndDestinationNotEquivalent,
    };

    struct ConversionError {
        gpufmt::BlockSampleError sampleError = gpufmt::BlockSampleError::None;
        gpufmt::WriteError writeError = gpufmt::WriteError::None;
    };

    class Converter {
    public:
        Converter();
        Converter(const Converter &) = delete;
        Converter(Converter &&other) noexcept;
        Converter(gpufmt::Format sourceFormat, gpufmt::Format destFormat) noexcept;
        ~Converter();
        
        Converter& operator=(const Converter &) = delete;
        Converter& operator=(Converter &&other) noexcept;

        [[nodiscard]]
        cputex::UniqueTexture convert(cputex::SurfaceView source, ConvertError &error) const noexcept;

        [[nodiscard]]
        cputex::UniqueTexture convert(cputex::TextureView source, ConvertError &error) const noexcept;
        
        cputex::ConvertError convertTo(cputex::SurfaceView source, cputex::SurfaceSpan dest) const noexcept;
        cputex::ConvertError convertTo(cputex::TextureView source, cputex::TextureSpan dest) const noexcept;

    private:
        gpufmt::BlockSampler mBlockSampler;
        gpufmt::Writer mWriter;
    };
}