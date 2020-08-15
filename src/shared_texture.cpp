#include <cputex/shared_texture.h>
#include <cputex/texture_view.h>

namespace cputex {
    SharedTexture::SharedTexture(const TextureParams &params)
        : mTextureStorage(params, true)
    {}

    SharedTexture::SharedTexture(const TextureParams &params, cputex::span<const cputex::byte> initialData)
        : mTextureStorage(params, initialData, true)
    {}

    SharedTexture::SharedTexture(const SharedTexture &other) noexcept
        : mTextureStorage(other.mTextureStorage)
    {
        mTextureStorage.addRef();
    }

    SharedTexture::SharedTexture(SharedTexture &&other) noexcept
        : mTextureStorage(std::move(other.mTextureStorage))
    {
    }

    SharedTexture::~SharedTexture() noexcept {
        if(mTextureStorage.decRef()) {
            std::lock_guard writeLock(mBufferMutex);
            mTextureStorage.destroy();
        }
    }

    SharedTexture& SharedTexture::operator=(const SharedTexture &other) noexcept {
        other.mTextureStorage.addRef();
        
        if(mTextureStorage.decRef()) {
            std::lock_guard writeLock(mBufferMutex);
            mTextureStorage.destroy();
        }

        mTextureStorage = other.mTextureStorage;

        return *this;
    }

    SharedTexture& SharedTexture::operator=(SharedTexture &&other) noexcept {
        if(mTextureStorage.decRef()) {
            std::lock_guard writeLock(mBufferMutex);
            mTextureStorage.destroy();
        }

        mTextureStorage = std::move(other.mTextureStorage);

        return *this;
    }

    bool SharedTexture::operator==(std::nullptr_t) const noexcept {
        return mTextureStorage == nullptr;
    }

    bool SharedTexture::operator!=(std::nullptr_t) const noexcept {
        return !operator==(nullptr);
    }

    SharedTexture::operator bool() const noexcept {
        return mTextureStorage.isValid();
    }

    SharedTexture::operator TextureView() const noexcept {
        return TextureView{ mTextureStorage };
    }

    bool SharedTexture::empty() const noexcept {
        return !mTextureStorage.isValid();
    }

    const Extent& SharedTexture::extent(uint32_t mip) const noexcept {
        return mTextureStorage.extent(mip);
    }

    uint32_t SharedTexture::arraySize() const noexcept {
        return mTextureStorage.arraySize();
    }

    uint32_t SharedTexture::faces() const noexcept {
        return mTextureStorage.faces();
    }

    uint32_t SharedTexture::mips() const noexcept {
        return mTextureStorage.mips();
    }

    TextureDimension SharedTexture::dimension() const noexcept {
        return mTextureStorage.dimension();
    }

    gpufmt::Format SharedTexture::format() const noexcept {
        return mTextureStorage.format();
    }

    uint32_t SharedTexture::surfaceByteAlignment() const noexcept {
        return mTextureStorage.surfaceByteAligment();
    }

    size_t SharedTexture::sizeInBytes() const noexcept {
        return mTextureStorage.sizeInBytes();
    }

    size_t SharedTexture::sizeInBytes(uint32_t mip) const noexcept {
        return mTextureStorage.sizeInBytes(mip);
    }

    uint32_t SharedTexture::surfaceCount() const noexcept {
        return mTextureStorage.surfaceCount();
    }

    cputex::span<const cputex::byte> SharedTexture::get2DSurfaceData(uint32_t arraySlice, uint32_t face, uint32_t mip, uint32_t volumeSlice) const noexcept {
        return mTextureStorage.get2DSurfaceData(arraySlice, face, mip, volumeSlice);
    }

    cputex::SurfaceView SharedTexture::getMipSurface(uint32_t arraySlice, uint32_t face, uint32_t mip) const noexcept {
        return cputex::SurfaceView(mTextureStorage, arraySlice, face, mip);
    }

    cputex::span<const cputex::byte> SharedTexture::getMipSurfaceData(uint32_t arraySlice, uint32_t face, uint32_t mip) const noexcept {
        return mTextureStorage.getMipSurfaceData(arraySlice, face, mip);
    }

    SharedTextureLock SharedTexture::lock() {
        return SharedTextureLock{ mTextureStorage, mBufferMutex };
    }

    SharedTexture SharedTexture::clone() const noexcept {
        if(!mTextureStorage.isValid()) {
            return SharedTexture();
        }

        SharedTexture clonedTexture;
        clonedTexture.mTextureStorage = internal::TextureStorage(mTextureStorage.getHeader()->params, true);

        if(!clonedTexture) {
            return SharedTexture();
        }

        auto sourceData = mTextureStorage.getData();
        std::copy(sourceData.begin(), sourceData.end(), clonedTexture.mTextureStorage.accessData().data());

        return clonedTexture;
    }
}