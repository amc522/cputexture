#pragma once

#include <cputex/internal/texture_storage.h>

#include <gpufmt/format.h>

namespace cputex {
    namespace internal {
        class BaseSurfaceSpan {
        public:
            BaseSurfaceSpan() noexcept = default;
            BaseSurfaceSpan(const BaseSurfaceSpan&) noexcept = default;
            BaseSurfaceSpan(BaseSurfaceSpan&&) noexcept = default;

            BaseSurfaceSpan(const cputex::internal::TextureStorage &textureStorage, uint32_t arraySlice, uint32_t face, uint32_t mip) noexcept
                : mStorage(textureStorage)
                , mArraySlice(static_cast<uint16_t>(arraySlice))
                , mFace(static_cast<uint8_t>(face))
                , mMip(static_cast<uint8_t>(mip))
            {}

            BaseSurfaceSpan& operator=(const BaseSurfaceSpan&) noexcept = default;
            BaseSurfaceSpan& operator=(BaseSurfaceSpan&&) noexcept = default;

            [[nodiscard]]
            bool operator==(std::nullptr_t) const noexcept {
                return mStorage == nullptr;
            }

            [[nodiscard]]
            bool operator!=(std::nullptr_t) const noexcept {
                return !operator==(nullptr);
            }

            [[nodiscard]]
            operator bool() const noexcept {
                return mStorage.isValid();
            }

            [[nodiscard]]
            bool empty() const noexcept {
                return !mStorage.isValid();
            }

            [[nodiscard]]
            const Extent &extent() const noexcept {
                static constexpr Extent zero{ 0, 0, 0 };
                return (mStorage.isValid()) ? mStorage.extent(mMip) : zero;
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
                return (mStorage.isValid()) ? mStorage.dimension() : TextureDimension::Texture1D;
            }

            [[nodiscard]]
            gpufmt::Format format() const noexcept {
                return (mStorage.isValid()) ? mStorage.format() : gpufmt::Format::UNDEFINED;
            }

            [[nodiscard]]
            uint32_t surfaceByteAlignment() const noexcept {
                return (mStorage.isValid()) ? mStorage.surfaceByteAligment() : 0u;
            }

            [[nodiscard]]
            size_t sizeInBytes() const noexcept {
                return (mStorage.isValid()) ? mStorage.sizeInBytes(mMip) : 0u;
            }

            [[nodiscard]]
            cputex::span<const cputex::byte> getData() const noexcept {
                return (mStorage.isValid()) ? mStorage.getMipSurfaceData(mArraySlice, mFace, mMip) : cputex::span<const cputex::byte>{};
            }

            template<class T>
            [[nodiscard]]
            cputex::span<const T> getDataAs() const noexcept {
                return (mStorage.isValid()) ? mStorage.getMipSurfaceDataAs<T>(mArraySlice, mFace, mMip) : cputex::span<const T>{};
            }

            [[nodiscard]]
            bool equivalentLayout(const BaseSurfaceSpan &other) const noexcept {
                return format() == other.format() &&
                    equivalentDimensions(other);
            }

            [[nodiscard]]
            bool equivalentDimensions(const BaseSurfaceSpan &other) const noexcept {
                return dimension() == other.dimension() &&
                    extent() == other.extent();
            }

        protected:
            virtual ~BaseSurfaceSpan() = default;

            cputex::internal::TextureStorage mStorage;
            uint16_t mArraySlice = 0u;
            uint8_t mFace = 0u;
            uint8_t mMip = 0u;
        };
    }

    class SurfaceView : public internal::BaseSurfaceSpan {
    public:
        SurfaceView() noexcept = default;
        SurfaceView(const internal::TextureStorage &textureStorage, uint32_t arraySlice, uint32_t face, uint32_t mip) noexcept
            : BaseSurfaceSpan(textureStorage, arraySlice, face, mip)
        {}

        ~SurfaceView() override = default;
    };

    namespace internal {
        class BaseTextureSpan {
        public:
            BaseTextureSpan() noexcept = default;
            BaseTextureSpan(const BaseTextureSpan&) noexcept = default;
            BaseTextureSpan(BaseTextureSpan&&) noexcept = default;
            explicit BaseTextureSpan(const cputex::internal::TextureStorage &textureStorage) noexcept
                : mStorage(textureStorage)
            {}

            BaseTextureSpan& operator=(const BaseTextureSpan&) noexcept = default;
            BaseTextureSpan& operator=(BaseTextureSpan&&) noexcept = default;

            [[nodiscard]]
            bool operator==(std::nullptr_t) const noexcept {
                return mStorage == nullptr;
            }

            [[nodiscard]]
            bool operator!=(std::nullptr_t) const noexcept {
                return !operator==(nullptr);
            }

            [[nodiscard]]
            operator bool() const noexcept {
                return mStorage.isValid();
            }

            [[nodiscard]]
            bool empty() const noexcept {
                return !mStorage.isValid();
            }

            [[nodiscard]]
            const Extent &extent(uint32_t mip = 0) const noexcept {
                static constexpr Extent zero{ 0, 0, 0 };
                return (mStorage.isValid()) ? mStorage.extent(mip) : zero;
            }

            [[nodiscard]]
            uint32_t arraySize() const noexcept {
                return (mStorage.isValid()) ? mStorage.arraySize() : 0u;
            }

            [[nodiscard]]
            uint32_t faces() const noexcept {
                return (mStorage.isValid()) ? mStorage.faces() : 0u;
            }

            [[nodiscard]]
            uint32_t mips() const noexcept {
                return (mStorage.isValid()) ? mStorage.mips() : 0u;
            }

            [[nodiscard]]
            TextureDimension dimension() const noexcept {
                return (mStorage.isValid()) ? mStorage.dimension() : TextureDimension::Texture1D;
            }

            [[nodiscard]]
            gpufmt::Format format() const noexcept {
                return (mStorage.isValid()) ? mStorage.format() : gpufmt::Format::UNDEFINED;
            }

            [[nodiscard]]
            uint32_t surfaceByteAlignment() const noexcept {
                return (mStorage.isValid()) ? mStorage.surfaceByteAligment() : 0u;
            }

            [[nodiscard]]
            size_t sizeInBytes() const noexcept {
                return (mStorage.isValid()) ? mStorage.sizeInBytes() : 0u;
            }

            [[nodiscard]]
            size_t sizeInBytes(uint32_t mip) const noexcept {
                return (mStorage.isValid()) ? mStorage.sizeInBytes(mip) : 0u;
            }

            [[nodiscard]]
            uint32_t surfaceCount() const noexcept {
                return (mStorage.isValid()) ? mStorage.surfaceCount() : 0u;
            }

            [[nodiscard]]
            cputex::span<const cputex::byte> get2DSurfaceData(uint32_t arraySlice = 0, uint32_t face = 0, uint32_t mip = 0, uint32_t volumeSlice = 0) const noexcept {
                return (mStorage.isValid()) ? mStorage.get2DSurfaceData(arraySlice, face, mip, volumeSlice) : cputex::span<const cputex::byte>{};
            }

            template<class T>
            [[nodiscard]]
            cputex::span<const T> get2DSurfaceDataAs(uint32_t arraySlice = 0, uint32_t face = 0, uint32_t mip = 0, uint32_t volumeSlice = 0) const noexcept {
                return (mStorage.isValid()) ? mStorage.get2DSurfaceDataAs<T>(arraySlice, face, mip, volumeSlice) : cputex::span<const T>{};
            }

            [[nodiscard]]
            cputex::SurfaceView getMipSurface(uint32_t arraySlice = 0, uint32_t face = 0, uint32_t mip = 0) const noexcept {
                return (mStorage.isValid()) ? SurfaceView(mStorage, arraySlice, face, mip) : SurfaceView();
            }

            [[nodiscard]]
            cputex::span<const cputex::byte> getMipSurfaceData(uint32_t arraySlice = 0, uint32_t face = 0, uint32_t mip = 0) const noexcept {
                return (mStorage.isValid()) ? mStorage.getMipSurfaceData(arraySlice, face, mip) : cputex::span<const cputex::byte>{};
            }

            template<class T>
            [[nodiscard]]
            cputex::span<const T> getMipSurfaceDataAs(uint32_t arraySlice = 0, uint32_t face = 0, uint32_t mip = 0) const noexcept {
                return (mStorage.isValid()) ? mStorage.get2DSurfaceDataAs<T>(arraySlice, face, mip) : cputex::span<const T>{};
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

            internal::TextureStorage mStorage;
        };
    }

    class TextureView : public internal::BaseTextureSpan {
    public:
        TextureView() noexcept = default;
        TextureView(const TextureView&) noexcept = default;
        TextureView(TextureView&&) noexcept = default;
        explicit TextureView(const internal::TextureStorage &textureStorage) noexcept
            : BaseTextureSpan(textureStorage)
        {}

        ~TextureView() override = default;

        TextureView& operator=(const TextureView&) noexcept = default;
        TextureView& operator=(TextureView&&) noexcept = default;
    };

    class SurfaceSpan : public internal::BaseSurfaceSpan {
    public:
        SurfaceSpan() noexcept = default;
        SurfaceSpan(internal::TextureStorage &textureStorage, uint32_t arraySlice, uint32_t face, uint32_t mip) noexcept
            : BaseSurfaceSpan(textureStorage, arraySlice, face, mip)
        {}

        ~SurfaceSpan() override = default;

        [[nodiscard]]
        operator SurfaceView() const noexcept {
            return (mStorage.isValid()) ? SurfaceView{ mStorage, mArraySlice, mFace, mMip } : SurfaceView();
        }

        [[nodiscard]]
        cputex::span<cputex::byte> accessData() noexcept {
            return (mStorage.isValid()) ? mStorage.accessMipSurfaceData(mArraySlice, mFace, mMip) : cputex::span<cputex::byte>{};
        }

        template<class T>
        [[nodiscard]]
        cputex::span<T> accessDataAs() noexcept {
            return (mStorage.isValid()) ? mStorage.accessMipSurfaceDataAs<T>(mArraySlice, mFace, mMip) : cputex::span<T>{};
        }

        [[nodiscard]]
        cputex::span<cputex::byte> accessData(uint32_t volumeSlice) noexcept {
            return (mStorage.isValid()) ? mStorage.access2DSurfaceData(mArraySlice, mFace, mMip, volumeSlice) : cputex::span<cputex::byte>{};
        }

        template<class T>
        [[nodiscard]]
        cputex::span<T> accessDataAs(uint32_t volumeSlice) noexcept {
            return (mStorage.isValid()) ? mStorage.accessMipSurfaceDataAs<T>(mArraySlice, mFace, mMip, volumeSlice) : cputex::span<T>{};
        }
    };

    class TextureSpan : public internal::BaseTextureSpan {
    public:
        TextureSpan() noexcept = default;
        explicit TextureSpan(const internal::TextureStorage &textureStorage) noexcept
            : BaseTextureSpan(textureStorage)
        {}

        ~TextureSpan() override = default;

        [[nodiscard]]
        operator TextureView() const noexcept {
            return (mStorage.isValid()) ? TextureView{ mStorage } : TextureView{};
        }

        [[nodiscard]]
        cputex::span<cputex::byte> access2DSurfaceData(uint32_t arraySlice = 0, uint32_t face = 0, uint32_t mip = 0, uint32_t volumeSlice = 0) noexcept {
            return (mStorage.isValid()) ? mStorage.access2DSurfaceData(arraySlice, face, mip, volumeSlice) : cputex::span<cputex::byte>{};
        }

        template<class T>
        [[nodiscard]]
        cputex::span<T> access2DSurfaceDataAs(uint32_t arraySlice = 0, uint32_t face = 0, uint32_t mip = 0, uint32_t volumeSlice = 0) noexcept {
            return (mStorage.isValid()) ? mStorage.access2DSurfaceDataAs<T>(arraySlice, face, mip, volumeSlice) : cputex::span<T>{};
        }

        [[nodiscard]]
        cputex::SurfaceSpan accessMipSurface(uint32_t arraySlice = 0, uint32_t face = 0, uint32_t mip = 0) noexcept {
            return (mStorage.isValid()) ? SurfaceSpan(mStorage, arraySlice, face, mip) : SurfaceSpan();
        }

        [[nodiscard]]
        cputex::span<cputex::byte> accessMipSurfaceData(uint32_t arraySlice = 0, uint32_t face = 0, uint32_t mip = 0) noexcept {
            return (mStorage.isValid()) ? mStorage.accessMipSurfaceData(arraySlice, face, mip) : cputex::span<cputex::byte>{};
        }

        template<class T>
        [[nodiscard]]
        cputex::span<T> accessMipSurfaceDataAs(uint32_t arraySlice = 0, uint32_t face = 0, uint32_t mip = 0) noexcept {
            return (mStorage.isValid()) ? mStorage.accessMipSurfaceDataAs<T>(arraySlice, face, mip) : cputex::span<T>{};
        }
    };
}