#pragma once

#include <cputex/internal/texture_storage.h>

#include <gpufmt/format.h>

namespace cputex {
    namespace internal {
        template<bool IsConst>
        class BaseSurfaceSpan {
        public:
            using TextureStorageType = std::conditional_t<IsConst, const internal::TextureStorage, internal::TextureStorage>;

            BaseSurfaceSpan() = default;
            
            BaseSurfaceSpan(TextureStorageType &textureStorage, uint32_t arraySlice, uint32_t face, uint32_t mip)
                : mStorage(&textureStorage)
                , mArraySlice(static_cast<uint16_t>(arraySlice))
                , mFace(static_cast<uint8_t>(face))
                , mMip(static_cast<uint8_t>(mip))
            {}

            [[nodiscard]]
            bool operator==(std::nullptr_t) const noexcept {
                return mStorage == nullptr || (*mStorage) == nullptr;
            }

            [[nodiscard]]
            bool operator!=(std::nullptr_t) const noexcept {
                return !operator==(nullptr);
            }

            [[nodiscard]]
            operator bool() const noexcept {
                return mStorage != nullptr && mStorage->isValid();
            }

            [[nodiscard]]
            bool empty() const noexcept {
                return mStorage == nullptr || !mStorage->isValid();
            }

            [[nodiscard]]
            const Extent &extent() const noexcept {
                static constexpr Extent zero{ 0, 0, 0 };
                return (mStorage) ? mStorage->extent(mMip) : zero;
            }

            [[nodiscard]]
            uint32_t arraySlice() const noexcept {
                return mArraySlice;
            }

            [[nodiscard]]
            uint32_t face() const noexcept {
                return mFace;
            }

            [[nodiscard]]
            uint32_t mip() const noexcept {
                return mMip;
            }

            [[nodiscard]]
            TextureDimension dimension() const noexcept {
                return (mStorage) ? mStorage->dimension() : TextureDimension::Texture1D;
            }

            [[nodiscard]]
            gpufmt::Format format() const noexcept {
                return (mStorage) ? mStorage->format() : gpufmt::Format::UNDEFINED;
            }

            [[nodiscard]]
            uint32_t surfaceByteAlignment() const noexcept {
                return (mStorage) ? mStorage->surfaceByteAligment() : 0u;
            }

            [[nodiscard]]
            size_t sizeInBytes() const noexcept {
                return (mStorage) ? mStorage->sizeInBytes(mMip, mVolumeSlice) : 0u;
            }

            [[nodiscard]]
            cputex::span<const cputex::byte> getData() const noexcept {
                return (mStorage) ? mStorage->getMipSurfaceData(mArraySlice, mFace, mMip) : cputex::span<const cputex::byte>{};
            }

            template<class T>
            [[nodiscard]]
            cputex::span<const T> getDataAs() const noexcept {
                return (mStorage) ? mStorage->getMipSurfaceDataAs<T>(mArraySlice, mFace, mMip) : cputex::span<const T>{};
            }

            [[nodiscard]]
            bool equivalentLayout(const BaseSurfaceSpan &other) const noexcept {
                return format() == other.format() &&
                    equivalentDimension(other);
            }

            [[nodiscard]]
            bool equivalentDimensions(const BaseSurfaceSpan &other) const noexcept {
                return dimension() == other.dimension() &&
                    extent() == other.extent();
            }

        protected:
            virtual ~BaseSurfaceSpan() = default;

            TextureStorageType *mStorage = nullptr;
            uint16_t mArraySlice = 0u;
            uint8_t mFace = 0u;
            uint8_t mMip = 0u;
        };
    }

    class SurfaceView : public internal::BaseSurfaceSpan<true> {
    public:
        SurfaceView() = default;
        SurfaceView(const internal::TextureStorage &textureStorage, uint32_t arraySlice, uint32_t face, uint32_t mip)
            : BaseSurfaceSpan(textureStorage, arraySlice, face, mip)
        {}

        ~SurfaceView() override = default;
    };

    namespace internal {
        template<bool IsConst>
        class BaseTextureSpan {
        public:
            using TextureStorageType = std::conditional_t<IsConst, const internal::TextureStorage, internal::TextureStorage>;

            BaseTextureSpan() = default;
            explicit BaseTextureSpan(TextureStorageType &textureStorage)
                : mStorage(&textureStorage)
            {}

            [[nodiscard]]
            bool operator==(std::nullptr_t) const noexcept {
                return mStorage == nullptr || (*mStorage) == nullptr;
            }

            [[nodiscard]]
            bool operator!=(std::nullptr_t) const noexcept {
                return !operator==(nullptr);
            }

            [[nodiscard]]
            operator bool() const noexcept {
                return mStorage != nullptr && mStorage->isValid();
            }

            [[nodiscard]]
            bool empty() const noexcept {
                return mStorage == nullptr || !mStorage->isValid();
            }

            [[nodiscard]]
            const Extent &extent(uint32_t mip = 0) const noexcept {
                static constexpr Extent zero{ 0, 0, 0 };
                return (mStorage) ? mStorage->extent(mip) : zero;
            }

            [[nodiscard]]
            uint32_t arraySize() const noexcept {
                return (mStorage) ? mStorage->arraySize() : 0u;
            }

            [[nodiscard]]
            uint32_t faces() const noexcept {
                return (mStorage) ? mStorage->faces() : 0u;
            }

            [[nodiscard]]
            uint32_t mips() const noexcept {
                return (mStorage) ? mStorage->mips() : 0u;
            }

            [[nodiscard]]
            TextureDimension dimension() const noexcept {
                return (mStorage) ? mStorage->dimension() : TextureDimension::Texture1D;
            }

            [[nodiscard]]
            gpufmt::Format format() const noexcept {
                return (mStorage) ? mStorage->format() : gpufmt::Format::UNDEFINED;
            }

            [[nodiscard]]
            uint32_t surfaceByteAlignment() const noexcept {
                return (mStorage) ? mStorage->surfaceByteAligment() : 0u;
            }

            [[nodiscard]]
            size_t sizeInBytes() const noexcept {
                return (mStorage) ? mStorage->sizeInBytes() : 0u;
            }

            [[nodiscard]]
            size_t sizeInBytes(uint32_t mip) const noexcept {
                return (mStorage) ? mStorage->sizeInBytes(mip) : 0u;
            }

            [[nodiscard]]
            uint32_t surfaceCount() const noexcept {
                return (mStorage) ? mStorage->surfaceCount() : 0u;
            }

            [[nodiscard]]
            cputex::span<const cputex::byte> get2DSurfaceData(uint32_t arraySlice = 0, uint32_t face = 0, uint32_t mip = 0, uint32_t volumeSlice = 0) const noexcept {
                return (mStorage) ? mStorage->get2DSurfaceData(arraySlice, face, mip, volumeSlice) : cputex::span<const cputex::byte>{};
            }

            template<class T>
            [[nodiscard]]
            cputex::span<const T> get2DSurfaceDataAs(uint32_t arraySlice = 0, uint32_t face = 0, uint32_t mip = 0, uint32_t volumeSlice = 0) const noexcept {
                return (mStorage) ? mStorage->get2DSurfaceDataAs<T>(arraySlice, face, mip, volumeSlice) : cputex::span<const T>{};
            }

            [[nodiscard]]
            cputex::SurfaceView getMipSurface(uint32_t arraySlice = 0, uint32_t face = 0, uint32_t mip = 0) const noexcept {
                return (mStorage) ? SurfaceView(*mStorage, arraySlice, face, mip) : SurfaceView();
            }

            [[nodiscard]]
            cputex::span<const cputex::byte> getMipSurfaceData(uint32_t arraySlice = 0, uint32_t face = 0, uint32_t mip = 0) const noexcept {
                return (mStorage) ? mStorage->getMipSurfaceData(arraySlice, face, mip) : cputex::span<const cputex::byte>{};
            }

            template<class T>
            [[nodiscard]]
            cputex::span<const T> getMipSurfaceDataAs(uint32_t arraySlice = 0, uint32_t face = 0, uint32_t mip = 0) const noexcept {
                return (mStorage) ? mStorage->get2DSurfaceDataAs<T>(arraySlice, face, mip) : cputex::span<const T>{};
            }

            [[nodiscard]]
            bool equivalentLayout(const BaseTextureSpan &other) const noexcept {
                return format() == other.format() &&
                    equivalentDimensions(other);
            }

            [[nodiscard]]
            bool equivalentDimensions(const BaseTextureSpan &other) const noexcept {
                return dimension() == other.dimension() &&
                    arraySize() == other.arraySize() &&
                    faces() == other.faces() &&
                    mips() == other.mips() &&
                    extent() == other.extent();
            }

        protected:
            virtual ~BaseTextureSpan() = default;

            TextureStorageType *mStorage = nullptr;
        };
    }

    class TextureView : public internal::BaseTextureSpan<true> {
    public:
        TextureView() = default;
        explicit TextureView(const internal::TextureStorage &textureStorage)
            : BaseTextureSpan(textureStorage)
        {}

        ~TextureView() override = default;
    };

    class SurfaceSpan : public internal::BaseSurfaceSpan<false> {
    public:
        SurfaceSpan() = default;
        SurfaceSpan(internal::TextureStorage &textureStorage, uint32_t arraySlice, uint32_t face, uint32_t mip)
            : BaseSurfaceSpan(textureStorage, arraySlice, face, mip)
        {}

        ~SurfaceSpan() override = default;

        [[nodiscard]]
        operator SurfaceView() const {
            return (mStorage != nullptr) ? SurfaceView{ *mStorage, mArraySlice, mFace, mMip } : SurfaceView();
        }

        [[nodiscard]]
        cputex::span<cputex::byte> accessData() noexcept {
            return (mStorage) ? mStorage->accessMipSurfaceData(mArraySlice, mFace, mMip) : cputex::span<cputex::byte>{};
        }

        template<class T>
        [[nodiscard]]
        cputex::span<T> accessDataAs() noexcept {
            return (mStorage) ? mStorage->accessMipSurfaceDataAs<T>(mArraySlice, mFace, mMip) : cputex::span<T>{};
        }

        [[nodiscard]]
        cputex::span<cputex::byte> accessData(uint32_t volumeSlice) noexcept {
            return (mStorage) ? mStorage->access2DSurfaceData(mArraySlice, mFace, mMip, volumeSlice) : cputex::span<cputex::byte>{};
        }

        template<class T>
        [[nodiscard]]
        cputex::span<T> accessDataAs(uint32_t volumeSlice) noexcept {
            return (mStorage) ? mStorage->accessMipSurfaceDataAs<T>(mArraySlice, mFace, mMip, volumeSlice) : cputex::span<T>{};
        }
    };

    class TextureSpan : public internal::BaseTextureSpan<false> {
    public:
        TextureSpan() = default;
        explicit TextureSpan(internal::TextureStorage &textureStorage)
            : BaseTextureSpan(textureStorage)
        {}

        ~TextureSpan() override = default;

        [[nodiscard]]
        operator TextureView() const noexcept {
            return (mStorage != nullptr) ? TextureView{ *mStorage } : TextureView{};
        }

        [[nodiscard]]
        cputex::span<cputex::byte> access2DSurfaceData(uint32_t arraySlice = 0, uint32_t face = 0, uint32_t mip = 0, uint32_t volumeSlice = 0) noexcept {
            return (mStorage) ? mStorage->access2DSurfaceData(arraySlice, face, mip, volumeSlice) : cputex::span<cputex::byte>{};
        }

        template<class T>
        [[nodiscard]]
        cputex::span<T> access2DSurfaceDataAs(uint32_t arraySlice = 0, uint32_t face = 0, uint32_t mip = 0, uint32_t volumeSlice = 0) noexcept {
            return (mStorage) ? mStorage->access2DSurfaceDataAs<T>(arraySlice, face, mip, volumeSlice) : cputex::span<T>{};
        }

        [[nodiscard]]
        cputex::SurfaceSpan accessMipSurface(uint32_t arraySlice = 0, uint32_t face = 0, uint32_t mip = 0) noexcept {
            return (mStorage) ? SurfaceSpan(*mStorage, arraySlice, face, mip) : SurfaceSpan();
        }

        [[nodiscard]]
        cputex::span<cputex::byte> accessMipSurfaceData(uint32_t arraySlice = 0, uint32_t face = 0, uint32_t mip = 0) noexcept {
            return (mStorage) ? mStorage->accessMipSurfaceData(arraySlice, face, mip) : cputex::span<cputex::byte>{};
        }

        template<class T>
        [[nodiscard]]
        cputex::span<T> accessMipSurfaceDataAs(uint32_t arraySlice = 0, uint32_t face = 0, uint32_t mip = 0) noexcept {
            return (mStorage) ? mStorage->accessMipSurfaceDataAs<T>(arraySlice, face, mip) : cputex::span<T>{};
        }
    };
}