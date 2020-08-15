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

    const Extent &UniqueTexture::extent(uint32_t mip) const noexcept {
        return mTextureStorage.extent(mip);
    }

    uint32_t UniqueTexture::arraySize() const noexcept {
        return mTextureStorage.arraySize();
    }

    uint32_t UniqueTexture::faces() const noexcept {
        return mTextureStorage.faces();
    }

    uint32_t UniqueTexture::mips() const noexcept {
        return mTextureStorage.mips();
    }

    TextureDimension UniqueTexture::dimension() const noexcept {
        return mTextureStorage.dimension();
    }

    gpufmt::Format UniqueTexture::format() const noexcept {
        return mTextureStorage.format();
    }

    uint32_t UniqueTexture::surfaceByteAlignment() const noexcept {
        return mTextureStorage.surfaceByteAligment();
    }

    size_t UniqueTexture::sizeInBytes() const noexcept {
        return mTextureStorage.sizeInBytes();
    }

    size_t UniqueTexture::sizeInBytes(uint32_t mip) const noexcept {
        return mTextureStorage.sizeInBytes(mip);
    }

    uint32_t UniqueTexture::surfaceCount() const noexcept {
        return mTextureStorage.surfaceCount();
    }

    cputex::span<const cputex::byte> UniqueTexture::get2DSurfaceData(uint32_t arraySlice, uint32_t face, uint32_t mip, uint32_t volumeSlice) const noexcept {
        return mTextureStorage.get2DSurfaceData(arraySlice, face, mip, volumeSlice);
    }

    cputex::span<cputex::byte> UniqueTexture::access2DSurfaceData(uint32_t arraySlice, uint32_t face, uint32_t mip, uint32_t volumeSlice) noexcept {
        return mTextureStorage.access2DSurfaceData(arraySlice, face, mip, volumeSlice);
    }

    cputex::span<const cputex::byte> UniqueTexture::getMipSurfaceData(uint32_t arraySlice, uint32_t face, uint32_t mip) const noexcept {
        return mTextureStorage.getMipSurfaceData(arraySlice, face, mip);
    }

    cputex::SurfaceView UniqueTexture::getMipSurface(uint32_t arraySlice, uint32_t face, uint32_t mip) const noexcept {
        return cputex::SurfaceView(mTextureStorage, arraySlice, face, mip);
    }

    cputex::span<cputex::byte> UniqueTexture::accessMipSurfaceData(uint32_t arraySlice, uint32_t face, uint32_t mip) noexcept {
        return mTextureStorage.accessMipSurfaceData(arraySlice, face, mip);
    }

    cputex::SurfaceSpan UniqueTexture::accessMipSurface(uint32_t arraySlice, uint32_t face, uint32_t mip) noexcept {
        return cputex::SurfaceSpan(mTextureStorage, arraySlice, face, mip);
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