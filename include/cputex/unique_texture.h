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
        const Extent &extent(cputex::CountType mip = 0) const noexcept;

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
        cputex::span<cputex::byte> access2DSurfaceData(cputex::CountType arraySlice = 0, cputex::CountType face = 0, cputex::CountType mip = 0, cputex::CountType volumeSlice = 0) noexcept;

        template<class T>
        [[nodiscard]]
        cputex::span<T> access2DSurfaceDataAs(cputex::CountType arraySlice = 0, cputex::CountType face = 0, cputex::CountType mip = 0, cputex::CountType volumeSlice = 0) noexcept {
            return mTextureStorage.access2DSurfaceDataAs<T>(arraySlice, face, mip, volumeSlice);
        }

        [[nodiscard]]
        cputex::span<const cputex::byte> getMipSurfaceData(cputex::CountType arraySlice = 0, cputex::CountType face = 0, cputex::CountType mip = 0) const noexcept;

        template<class T>
        [[nodiscard]]
        cputex::span<const T> getMipSurfaceDataAs(cputex::CountType arraySlice = 0, cputex::CountType face = 0, cputex::CountType mip = 0) const noexcept {
            return mTextureStorage.getMipSurfaceDataAs<T>(arraySlice, face, mip);
        }

        [[nodiscard]]
        cputex::TextureSurfaceView getMipSurface(cputex::CountType arraySlice = 0, cputex::CountType face = 0, cputex::CountType mip = 0) const noexcept;

        [[nodiscard]]
        cputex::span<cputex::byte> accessMipSurfaceData(cputex::CountType arraySlice = 0, cputex::CountType face = 0, cputex::CountType mip = 0) noexcept;

        template<class T>
        [[nodiscard]]
        cputex::span<T> accessMipSurfaceDataAs(cputex::CountType arraySlice = 0, cputex::CountType face = 0, cputex::CountType mip = 0) noexcept {
            return mTextureStorage.accessMipSurfaceDataAs<T>(arraySlice, face, mip);
        }

        [[nodiscard]]
        cputex::TextureSurfaceSpan accessMipSurface(cputex::CountType arraySlice = 0, cputex::CountType face = 0, cputex::CountType mip = 0) noexcept;

        [[nodiscard]]
		UniqueTexture clone() const noexcept;

	private:
        internal::TextureStorage mTextureStorage;
	};
}