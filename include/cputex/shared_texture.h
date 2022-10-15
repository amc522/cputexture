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
        const Extent &extent(cputex::CountType mip = 0) const noexcept {
            return mStorage.extent(mip);
        }

        [[nodiscard]]
        cputex::CountType arraySize() const noexcept {
            return mStorage.arraySize();
        }

        [[nodiscard]]
        cputex::CountType faces() const noexcept {
            return mStorage.faces();
        }

        [[nodiscard]]
        cputex::CountType mips() const noexcept {
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
        cputex::CountType surfaceByteAlignment() const noexcept {
            return mStorage.surfaceByteAligment();
        }

        [[nodiscard]]
        cputex::SizeType sizeInBytes() const noexcept {
            return mStorage.sizeInBytes();
        }

        [[nodiscard]]
        cputex::SizeType sizeInBytes(cputex::CountType mip) const noexcept {
            return mStorage.sizeInBytes(mip);
        }

        [[nodiscard]]
        cputex::CountType surfaceCount() const noexcept {
            return mStorage.surfaceCount();
        }

        [[nodiscard]]
        cputex::span<cputex::byte> access2DSurfaceData(cputex::CountType arraySlice, cputex::CountType face, cputex::CountType mip, cputex::CountType volumeSlice) noexcept {
            return mStorage.access2DSurfaceData(arraySlice, face, mip, volumeSlice);
        }

        template<class T>
        [[nodiscard]]
        cputex::span<T> access2DSurfaceDataAs(cputex::CountType arraySlice, cputex::CountType face, cputex::CountType mip, cputex::CountType volumeSlice) noexcept {
            return mStorage.access2DSurfaceDataAs<T>(arraySlice, face, mip, volumeSlice);
        }

        template<class T>
        [[nodiscard]]
        cputex::TextureSurfaceSpan accessMipSurface(cputex::CountType arraySlice, cputex::CountType face, cputex::CountType mip) noexcept {
            return cputex::TextureSurfaceSpan{ mStorage, arraySlice, face, mip };
        }

        [[nodiscard]]
        cputex::span<cputex::byte> accessMipSurfaceData(cputex::CountType arraySlice, cputex::CountType face, cputex::CountType mip) noexcept {
            return mStorage.accessMipSurfaceData(arraySlice, face, mip);
        }

        template<class T>
        [[nodiscard]]
        cputex::span<T> accessMipSurfaceDataAs(cputex::CountType arraySlice, cputex::CountType face, cputex::CountType mip) noexcept {
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
        const Extent& extent(cputex::CountType mip = 0) const noexcept;

        [[nodiscard]]
        cputex::CountType arraySize() const noexcept;

        [[nodiscard]]
        cputex::CountType faces() const noexcept;

        [[nodiscard]]
        cputex::CountType mips() const noexcept;

        [[nodiscard]]
        TextureDimension dimension() const noexcept;

        [[nodiscard]]
        gpufmt::Format format() const noexcept;

        [[nodiscard]]
        cputex::CountType surfaceByteAlignment() const noexcept;

        [[nodiscard]]
        cputex::SizeType sizeInBytes() const noexcept;

        [[nodiscard]]
        cputex::SizeType sizeInBytes(cputex::CountType mip) const noexcept;

        [[nodiscard]]
        cputex::CountType surfaceCount() const noexcept;

        [[nodiscard]]
        cputex::span<const cputex::byte> get2DSurfaceData(cputex::CountType arraySlice = 0, cputex::CountType face = 0, cputex::CountType mip = 0, cputex::CountType volumeSlice = 0) const noexcept;

        template<class T>
        [[nodiscard]]
        cputex::span<const T> get2DSurfaceDataAs(cputex::CountType arraySlice = 0, cputex::CountType face = 0, cputex::CountType mip = 0, cputex::CountType volumeSlice = 0) const noexcept {
            return mTextureStorage.get2DSurfaceDataAs<T>(arraySlice, face, mip, volumeSlice);
        }

        [[nodiscard]]
        cputex::TextureSurfaceView getMipSurface(cputex::CountType arraySlice = 0, cputex::CountType face = 0, cputex::CountType mip = 0) const noexcept;

        [[nodiscard]]
        cputex::span<const cputex::byte> getMipSurfaceData(cputex::CountType arraySlice = 0, cputex::CountType face = 0, cputex::CountType mip = 0) const noexcept;

        template<class T>
        [[nodiscard]]
        cputex::span<const T> getMipSurfaceDataAs(cputex::CountType arraySlice = 0, cputex::CountType face = 0, cputex::CountType mip = 0) const noexcept {
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