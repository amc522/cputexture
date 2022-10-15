#include <cputex/unique_texture.h>
#include <cputex/texture_view.h>

namespace cputex {
    UniqueTexture::UniqueTexture(const TextureParams &params)
        : mTextureStorage(params, false)
    {
    }

    UniqueTexture::UniqueTexture(const TextureParams &params, cputex::span<const cputex::byte> initialData)
        : mTextureStorage(params, initialData, false)
    {
    }

    UniqueTexture::~UniqueTexture() noexcept {
        mTextureStorage.destroy();
    }

    bool UniqueTexture::operator==(std::nullptr_t) const noexcept {
        return mTextureStorage == nullptr;
    }

    bool UniqueTexture::operator!=(std::nullptr_t) const noexcept {
        return !operator==(nullptr);
    }

    UniqueTexture::operator bool() const noexcept {
        return mTextureStorage.isValid();
    }

    UniqueTexture::operator TextureView() const noexcept {
        return TextureView{ mTextureStorage };
    }

    UniqueTexture::operator TextureSpan() noexcept {
        return TextureSpan{ mTextureStorage };
    }

    bool UniqueTexture::empty() const noexcept {
        return !mTextureStorage.isValid();
    }

    const Extent &UniqueTexture::extent(CountType mip) const noexcept {
        return mTextureStorage.extent(mip);
    }

    CountType UniqueTexture::arraySize() const noexcept {
        return mTextureStorage.arraySize();
    }

    CountType UniqueTexture::faces() const noexcept {
        return mTextureStorage.faces();
    }

    CountType UniqueTexture::mips() const noexcept {
        return mTextureStorage.mips();
    }

    TextureDimension UniqueTexture::dimension() const noexcept {
        return mTextureStorage.dimension();
    }

    gpufmt::Format UniqueTexture::format() const noexcept {
        return mTextureStorage.format();
    }

    CountType UniqueTexture::surfaceByteAlignment() const noexcept {
        return mTextureStorage.surfaceByteAligment();
    }

    SizeType UniqueTexture::sizeInBytes() const noexcept {
        return mTextureStorage.sizeInBytes();
    }

    SizeType UniqueTexture::sizeInBytes(CountType mip) const noexcept {
        return mTextureStorage.sizeInBytes(mip);
    }

    CountType UniqueTexture::surfaceCount() const noexcept {
        return mTextureStorage.surfaceCount();
    }

    cputex::span<const cputex::byte> UniqueTexture::get2DSurfaceData(CountType arraySlice, CountType face, CountType mip, CountType volumeSlice) const noexcept {
        return mTextureStorage.get2DSurfaceData(arraySlice, face, mip, volumeSlice);
    }

    cputex::span<cputex::byte> UniqueTexture::access2DSurfaceData(CountType arraySlice, CountType face, CountType mip, CountType volumeSlice) noexcept {
        return mTextureStorage.access2DSurfaceData(arraySlice, face, mip, volumeSlice);
    }

    cputex::span<const cputex::byte> UniqueTexture::getMipSurfaceData(CountType arraySlice, CountType face, CountType mip) const noexcept {
        return mTextureStorage.getMipSurfaceData(arraySlice, face, mip);
    }

    cputex::TextureSurfaceView UniqueTexture::getMipSurface(CountType arraySlice, CountType face, CountType mip) const noexcept {
        return cputex::TextureSurfaceView(mTextureStorage, arraySlice, face, mip);
    }

    cputex::span<cputex::byte> UniqueTexture::accessMipSurfaceData(CountType arraySlice, CountType face, CountType mip) noexcept {
        return mTextureStorage.accessMipSurfaceData(arraySlice, face, mip);
    }

    cputex::TextureSurfaceSpan UniqueTexture::accessMipSurface(CountType arraySlice, CountType face, CountType mip) noexcept {
        return cputex::TextureSurfaceSpan(mTextureStorage, arraySlice, face, mip);
    }

    UniqueTexture UniqueTexture::clone() const noexcept {
        if(!mTextureStorage.isValid()) {
            return UniqueTexture();
        }

        UniqueTexture clonedTexture;
        clonedTexture.mTextureStorage = internal::TextureStorage(mTextureStorage.getHeader()->params, false);

        if(!clonedTexture) {
            return UniqueTexture();
        }

        auto sourceData = mTextureStorage.getData();
        std::copy(sourceData.begin(), sourceData.end(), clonedTexture.mTextureStorage.accessData().data());

        return clonedTexture;
    }
}