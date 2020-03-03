#pragma once

#include <cputex/internal/texture_storage.h>
#include <cputex/texture_view.h>

#include <atomic>
#include <memory>
#include <mutex>

namespace cputex {
    class SharedTextureLock {
    public:
        SharedTextureLock() noexcept = default;
        SharedTextureLock(cputex::internal::TextureStorage &storage, std::mutex &bufferMutex)
            : mLock(bufferMutex)
            , mStorage(storage)
        {}

        [[nodiscard]]
        operator cputex::TextureSpan() noexcept {
            return cputex::TextureSpan{ mStorage };
        }

        [[nodiscard]]
        const Extent &extent(uint32_t mip = 0) const noexcept {
            return mStorage.extent(mip);
        }

        [[nodiscard]]
        uint32_t arraySize() const noexcept {
            return mStorage.arraySize();
        }

        [[nodiscard]]
        uint32_t faces() const noexcept {
            return mStorage.faces();
        }

        [[nodiscard]]
        uint32_t mips() const noexcept {
            return mStorage.mips();
        }

        [[nodiscard]]
        TextureDimension dimension() const noexcept {
            return mStorage.dimension();
        }

        [[nodiscard]]
        gpufmt::Format format() const noexcept {
            return mStorage.format();
        }

        [[nodiscard]]
        uint32_t surfaceByteAlignment() const noexcept {
            return mStorage.surfaceByteAligment();
        }

        [[nodiscard]]
        size_t sizeInBytes() const noexcept {
            return mStorage.sizeInBytes();
        }

        [[nodiscard]]
        size_t sizeInBytes(uint32_t mip) const noexcept {
            return mStorage.sizeInBytes(mip);
        }

        [[nodiscard]]
        uint32_t surfaceCount() const noexcept {
            return mStorage.surfaceCount();
        }

        [[nodiscard]]
        cputex::span<cputex::byte> access2DSurfaceData(uint32_t arraySlice, uint32_t face, uint32_t mip, uint32_t volumeSlice) noexcept {
            return mStorage.access2DSurfaceData(arraySlice, face, mip, volumeSlice);
        }

        template<class T>
        [[nodiscard]]
        cputex::span<T> access2DSurfaceDataAs(uint32_t arraySlice, uint32_t face, uint32_t mip, uint32_t volumeSlice) noexcept {
            return mStorage.access2DSurfaceDataAs<T>(arraySlice, face, mip, volumeSlice);
        }

        template<class T>
        [[nodiscard]]
        cputex::SurfaceSpan accessMipSurface(uint32_t arraySlice, uint32_t face, uint32_t mip) noexcept {
            return cputex::SurfaceSpan{ mStorage, arraySlice, face, mip };
        }

        [[nodiscard]]
        cputex::span<cputex::byte> accessMipSurfaceData(uint32_t arraySlice, uint32_t face, uint32_t mip) noexcept {
            return mStorage.accessMipSurfaceData(arraySlice, face, mip);
        }

        template<class T>
        [[nodiscard]]
        cputex::span<T> accessMipSurfaceDataAs(uint32_t arraySlice, uint32_t face, uint32_t mip) noexcept {
            return mStorage.accessMipSurfaceDataAs<T>(arraySlice, face, mip);
        }

    private:
        std::unique_lock<std::mutex> mLock;
        cputex::internal::TextureStorage mStorage;
    };
    
	class SharedTexture {
	public:
		SharedTexture() noexcept = default;
		explicit SharedTexture(const TextureParams &params);
        SharedTexture(const TextureParams &params, cputex::span<const cputex::byte> initialData);
		SharedTexture(const SharedTexture &other) noexcept;
		SharedTexture(SharedTexture &&other) noexcept;
		~SharedTexture() noexcept;

		SharedTexture& operator=(const SharedTexture &other) noexcept;
		SharedTexture& operator=(SharedTexture &&other) noexcept;

        [[nodiscard]]
        bool operator==(std::nullptr_t) const noexcept;

        [[nodiscard]]
        bool operator!=(std::nullptr_t) const noexcept;

        [[nodiscard]]
        operator bool() const noexcept;

        [[nodiscard]]
        operator TextureView() const noexcept;

        [[nodiscard]]
        bool empty() const noexcept;

        [[nodiscard]]
        const Extent& extent(uint32_t mip = 0) const noexcept;

        [[nodiscard]]
        uint32_t arraySize() const noexcept;

        [[nodiscard]]
        uint32_t faces() const noexcept;

        [[nodiscard]]
        uint32_t mips() const noexcept;

        [[nodiscard]]
        TextureDimension dimension() const noexcept;

        [[nodiscard]]
        gpufmt::Format format() const noexcept;

        [[nodiscard]]
        uint32_t surfaceByteAlignment() const noexcept;

        [[nodiscard]]
        size_t sizeInBytes() const noexcept;

        [[nodiscard]]
        size_t sizeInBytes(uint32_t mip) const noexcept;

        [[nodiscard]]
        uint32_t surfaceCount() const noexcept;

        [[nodiscard]]
        cputex::span<const cputex::byte> get2DSurfaceData(uint32_t arraySlice = 0, uint32_t face = 0, uint32_t mip = 0, uint32_t volumeSlice = 0) const noexcept;

        template<class T>
        [[nodiscard]]
        cputex::span<const T> get2DSurfaceDataAs(uint32_t arraySlice = 0, uint32_t face = 0, uint32_t mip = 0, uint32_t volumeSlice = 0) const noexcept {
            return mTextureStorage.get2DSurfaceDataAs<T>(arraySlice, face, mip, volumeSlice);
        }

        [[nodiscard]]
        cputex::SurfaceView getMipSurface(uint32_t arraySlice = 0, uint32_t face = 0, uint32_t mip = 0) const noexcept;

        [[nodiscard]]
        cputex::span<const cputex::byte> getMipSurfaceData(uint32_t arraySlice = 0, uint32_t face = 0, uint32_t mip = 0) const noexcept;

        template<class T>
        [[nodiscard]]
        cputex::span<const T> getMipSurfaceDataAs(uint32_t arraySlice = 0, uint32_t face = 0, uint32_t mip = 0) const noexcept {
            return mTextureStorage.get2DSurfaceDataAs<T>(arraySlice, face, mip);
        }

        [[nodiscard]]
        SharedTextureLock lock();

        [[nodiscard]]
		SharedTexture clone() const noexcept;

	private:
        internal::TextureStorage mTextureStorage;
        std::mutex mBufferMutex;
	};
}