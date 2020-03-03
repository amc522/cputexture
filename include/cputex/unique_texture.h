#pragma once

#include <cputex/internal/texture_storage.h>
#include <cputex/texture_view.h>

#include <gpufmt/format.h>

#include <memory>

namespace cputex {
	class UniqueTexture {
	public:
		UniqueTexture() noexcept = default;
		explicit UniqueTexture(const TextureParams &params);
        UniqueTexture(const TextureParams &params, gpufmt::span<const cputex::byte> initialData);
		UniqueTexture(const UniqueTexture&) = delete;
		UniqueTexture(UniqueTexture &&other) noexcept = default;
		~UniqueTexture() noexcept;

		UniqueTexture& operator=(const UniqueTexture&) = delete;
		UniqueTexture& operator=(UniqueTexture &&other) noexcept = default;

        [[nodiscard]]
        bool operator==(std::nullptr_t) const noexcept;

        [[nodiscard]]
        bool operator!=(std::nullptr_t) const noexcept;

        [[nodiscard]]
        operator bool() const noexcept;

        [[nodiscard]]
        operator cputex::TextureView() const noexcept;

        [[nodiscard]]
        explicit operator cputex::TextureSpan() noexcept;

        [[nodiscard]]
        bool empty() const noexcept;

        [[nodiscard]]
        const Extent &extent(uint32_t mip = 0) const noexcept;

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
        cputex::span<cputex::byte> access2DSurfaceData(uint32_t arraySlice = 0, uint32_t face = 0, uint32_t mip = 0, uint32_t volumeSlice = 0) noexcept;

        template<class T>
        [[nodiscard]]
        cputex::span<T> access2DSurfaceDataAs(uint32_t arraySlice = 0, uint32_t face = 0, uint32_t mip = 0, uint32_t volumeSlice = 0) noexcept {
            return mTextureStorage.access2DSurfaceDataAs<T>(arraySlice, face, mip, volumeSlice);
        }

        [[nodiscard]]
        cputex::span<const cputex::byte> getMipSurfaceData(uint32_t arraySlice = 0, uint32_t face = 0, uint32_t mip = 0) const noexcept;

        template<class T>
        [[nodiscard]]
        cputex::span<const T> getMipSurfaceDataAs(uint32_t arraySlice = 0, uint32_t face = 0, uint32_t mip = 0) const noexcept {
            return mTextureStorage.getMipSurfaceDataAs<T>(arraySlice, face, mip);
        }

        [[nodiscard]]
        cputex::SurfaceView getMipSurface(uint32_t arraySlice = 0, uint32_t face = 0, uint32_t mip = 0) const noexcept;

        [[nodiscard]]
        cputex::span<cputex::byte> accessMipSurfaceData(uint32_t arraySlice = 0, uint32_t face = 0, uint32_t mip = 0) noexcept;

        template<class T>
        [[nodiscard]]
        cputex::span<T> accessMipSurfaceDataAs(uint32_t arraySlice = 0, uint32_t face = 0, uint32_t mip = 0) noexcept {
            return mTextureStorage.accessMipSurfaceDataAs<T>(arraySlice, face, mip);
        }

        [[nodiscard]]
        cputex::SurfaceSpan accessMipSurface(uint32_t arraySlice = 0, uint32_t face = 0, uint32_t mip = 0) noexcept;

        [[nodiscard]]
		UniqueTexture clone() const noexcept;

	private:
        internal::TextureStorage mTextureStorage;
	};
}