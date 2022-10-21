#pragma once

#include <cputex/internal/texture_storage.h>

#include <gpufmt/format.h>

namespace cputex {
    class SurfaceView;
    class SurfaceSpan;

    namespace internal {
        class BaseTextureSurfaceSpan {
        public:
            BaseTextureSurfaceSpan() noexcept = default;
            BaseTextureSurfaceSpan(const BaseTextureSurfaceSpan&) noexcept = default;
            BaseTextureSurfaceSpan(BaseTextureSurfaceSpan&&) noexcept = default;

            BaseTextureSurfaceSpan(const cputex::internal::TextureStorage &textureStorage, cputex::CountType arraySlice, cputex::CountType face, cputex::CountType mip) noexcept
                : mStorage(textureStorage)
                , mArraySlice(static_cast<uint16_t>(arraySlice))
                , mFace(static_cast<uint8_t>(face))
                , mMip(static_cast<uint8_t>(mip))
            {}

            BaseTextureSurfaceSpan& operator=(const BaseTextureSurfaceSpan&) noexcept = default;
            BaseTextureSurfaceSpan& operator=(BaseTextureSurfaceSpan&&) noexcept = default;

            [[nodiscard]]
            bool operator==(std::nullptr_t) const noexcept {
                return mStorage == nullptr;
            }

            [[nodiscard]]
            bool operator!=(std::nullptr_t) const noexcept {
                return !operator==(nullptr);
            }

            [[nodiscard]]
            constexpr operator SurfaceView() const noexcept;

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
            cputex::CountType arraySlice() const noexcept {
                return mArraySlice;
            }

            [[nodiscard]]
            cputex::CountType face() const noexcept {
                return mFace;
            }

            [[nodiscard]]
            cputex::CountType mip() const noexcept {
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
            cputex::CountType surfaceByteAlignment() const noexcept {
                return (mStorage.isValid()) ? mStorage.surfaceByteAligment() : cputex::CountType(0);
            }

            [[nodiscard]]
            cputex::SizeType sizeInBytes() const noexcept {
                return (mStorage.isValid()) ? mStorage.sizeInBytes(mMip) : cputex::SizeType(0);
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
            bool equivalentLayout(const BaseTextureSurfaceSpan &other) const noexcept {
                return format() == other.format() &&
                    equivalentDimensions(other);
            }

            [[nodiscard]]
            bool equivalentDimensions(const BaseTextureSurfaceSpan &other) const noexcept {
                return dimension() == other.dimension() &&
                    extent() == other.extent();
            }

        protected:
            virtual ~BaseTextureSurfaceSpan() = default;

            cputex::internal::TextureStorage mStorage;
            uint16_t mArraySlice = 0u;
            uint8_t mFace = 0u;
            uint8_t mMip = 0u;
        };
    }

    class TextureSurfaceView : public internal::BaseTextureSurfaceSpan {
    public:
        TextureSurfaceView() noexcept = default;
        TextureSurfaceView(const internal::TextureStorage &textureStorage, cputex::CountType arraySlice, cputex::CountType face, cputex::CountType mip) noexcept
            : BaseTextureSurfaceSpan(textureStorage, arraySlice, face, mip)
        {}

        ~TextureSurfaceView() override = default;
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
            const Extent &extent(cputex::CountType mip = 0) const noexcept {
                static constexpr Extent zero{ 0, 0, 0 };
                return (mStorage.isValid()) ? mStorage.extent(mip) : zero;
            }

            [[nodiscard]]
            cputex::CountType arraySize() const noexcept {
                return (mStorage.isValid()) ? mStorage.arraySize() : 0u;
            }

            [[nodiscard]]
            cputex::CountType faces() const noexcept {
                return (mStorage.isValid()) ? mStorage.faces() : 0u;
            }

            [[nodiscard]]
            cputex::CountType mips() const noexcept {
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
            cputex::CountType surfaceByteAlignment() const noexcept {
                return (mStorage.isValid()) ? mStorage.surfaceByteAligment() : 0u;
            }

            [[nodiscard]]
            cputex::SizeType sizeInBytes() const noexcept {
                return (mStorage.isValid()) ? mStorage.sizeInBytes() : cputex::SizeType(0);
            }

            [[nodiscard]]
            cputex::SizeType sizeInBytes(cputex::CountType mip) const noexcept {
                return (mStorage.isValid()) ? mStorage.sizeInBytes(mip) : cputex::SizeType(0);
            }

            [[nodiscard]]
            cputex::CountType surfaceCount() const noexcept {
                return (mStorage.isValid()) ? mStorage.surfaceCount() : 0u;
            }

            [[nodiscard]]
            cputex::span<const cputex::byte> getData() const noexcept {
                return (mStorage.isValid()) ? mStorage.getData() : cputex::span<const cputex::byte>{};
            }

            template<class T>
            [[nodiscard]]
            cputex::span<const T> getDataAs() const noexcept {
                return (mStorage.isValid()) ? mStorage.getDataAs<T>() : cputex::span<const T>{};
            }

            [[nodiscard]]
            cputex::span<const cputex::byte> get2DSurfaceData(cputex::CountType arraySlice = 0, cputex::CountType face = 0, cputex::CountType mip = 0, cputex::CountType volumeSlice = 0) const noexcept {
                return (mStorage.isValid()) ? mStorage.get2DSurfaceData(arraySlice, face, mip, volumeSlice) : cputex::span<const cputex::byte>{};
            }

            template<class T>
            [[nodiscard]]
            cputex::span<const T> get2DSurfaceDataAs(cputex::CountType arraySlice = 0, cputex::CountType face = 0, cputex::CountType mip = 0, cputex::CountType volumeSlice = 0) const noexcept {
                return (mStorage.isValid()) ? mStorage.get2DSurfaceDataAs<T>(arraySlice, face, mip, volumeSlice) : cputex::span<const T>{};
            }

            [[nodiscard]]
            cputex::TextureSurfaceView getMipSurface(cputex::CountType arraySlice = 0, cputex::CountType face = 0, cputex::CountType mip = 0) const noexcept {
                return (mStorage.isValid()) ? TextureSurfaceView(mStorage, arraySlice, face, mip) : TextureSurfaceView();
            }

            [[nodiscard]]
            cputex::span<const cputex::byte> getMipSurfaceData(cputex::CountType arraySlice = 0, cputex::CountType face = 0, cputex::CountType mip = 0) const noexcept {
                return (mStorage.isValid()) ? mStorage.getMipSurfaceData(arraySlice, face, mip) : cputex::span<const cputex::byte>{};
            }

            template<class T>
            [[nodiscard]]
            cputex::span<const T> getMipSurfaceDataAs(cputex::CountType arraySlice = 0, cputex::CountType face = 0, cputex::CountType mip = 0) const noexcept {
                return (mStorage.isValid()) ? mStorage.getMipSurfaceDataAs<T>(arraySlice, face, mip) : cputex::span<const T>{};
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

    class TextureSurfaceSpan : public internal::BaseTextureSurfaceSpan {
    public:
        TextureSurfaceSpan() noexcept = default;
        TextureSurfaceSpan(internal::TextureStorage &textureStorage, cputex::CountType arraySlice, cputex::CountType face, cputex::CountType mip) noexcept
            : BaseTextureSurfaceSpan(textureStorage, arraySlice, face, mip)
        {}

        ~TextureSurfaceSpan() override = default;

        [[nodiscard]]
        constexpr operator SurfaceSpan() noexcept;

        [[nodiscard]]
        operator TextureSurfaceView() const noexcept {
            return (mStorage.isValid()) ? TextureSurfaceView{ mStorage, mArraySlice, mFace, mMip } : TextureSurfaceView();
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
        cputex::span<cputex::byte> accessData(cputex::CountType volumeSlice) noexcept {
            return (mStorage.isValid()) ? mStorage.access2DSurfaceData(mArraySlice, mFace, mMip, volumeSlice) : cputex::span<cputex::byte>{};
        }

        template<class T>
        [[nodiscard]]
        cputex::span<T> accessDataAs(cputex::CountType volumeSlice) noexcept {
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
        cputex::span<cputex::byte> accessData() noexcept {
            return (mStorage.isValid()) ? mStorage.accessData() : cputex::span<cputex::byte>{};
        }

        template<class T>
        [[nodiscard]]
        cputex::span<T> accessDataAs() noexcept {
            return (mStorage.isValid()) ? mStorage.accessDataAs<T>() : cputex::span<T>{};
        }

        [[nodiscard]]
        cputex::span<cputex::byte> access2DSurfaceData(cputex::CountType arraySlice = 0, cputex::CountType face = 0, cputex::CountType mip = 0, cputex::CountType volumeSlice = 0) noexcept {
            return (mStorage.isValid()) ? mStorage.access2DSurfaceData(arraySlice, face, mip, volumeSlice) : cputex::span<cputex::byte>{};
        }

        template<class T>
        [[nodiscard]]
        cputex::span<T> access2DSurfaceDataAs(cputex::CountType arraySlice = 0, cputex::CountType face = 0, cputex::CountType mip = 0, cputex::CountType volumeSlice = 0) noexcept {
            return (mStorage.isValid()) ? mStorage.access2DSurfaceDataAs<T>(arraySlice, face, mip, volumeSlice) : cputex::span<T>{};
        }

        [[nodiscard]]
        cputex::TextureSurfaceSpan accessMipSurface(cputex::CountType arraySlice = 0, cputex::CountType face = 0, cputex::CountType mip = 0) noexcept {
            return (mStorage.isValid()) ? TextureSurfaceSpan(mStorage, arraySlice, face, mip) : TextureSurfaceSpan();
        }

        [[nodiscard]]
        cputex::span<cputex::byte> accessMipSurfaceData(cputex::CountType arraySlice = 0, cputex::CountType face = 0, cputex::CountType mip = 0) noexcept {
            return (mStorage.isValid()) ? mStorage.accessMipSurfaceData(arraySlice, face, mip) : cputex::span<cputex::byte>{};
        }

        template<class T>
        [[nodiscard]]
        cputex::span<T> accessMipSurfaceDataAs(cputex::CountType arraySlice = 0, cputex::CountType face = 0, cputex::CountType mip = 0) noexcept {
            return (mStorage.isValid()) ? mStorage.accessMipSurfaceDataAs<T>(arraySlice, face, mip) : cputex::span<T>{};
        }
    };

    class SurfaceView
    {
    public:
        constexpr SurfaceView() noexcept = default;
        constexpr SurfaceView(gpufmt::Format format, cputex::TextureDimension dimension, const cputex::Extent& extent, cputex::span<const cputex::byte> data) noexcept
            : mFormat(format)
            , mData(data.data())
        {
            if(dimension == TextureDimension::Texture1D)
            {
                mExtent.x = static_cast<SmallExtentComponent>(std::max<cputex::ExtentComponent>(extent.x, 1));
                mExtent.y = 0;
                mExtent.z = 0;
            }
            else if(dimension == TextureDimension::Texture2D || dimension == TextureDimension::TextureCube)
            {
                mExtent.x = static_cast<SmallExtentComponent>(std::max<cputex::ExtentComponent>(extent.x, 1));
                mExtent.y = static_cast<SmallExtentComponent>(std::max<cputex::ExtentComponent>(extent.y, 1));
                mExtent.z = 0;
            }
            else
            {
                mExtent.x = static_cast<SmallExtentComponent>(std::max<cputex::ExtentComponent>(extent.x, 1));
                mExtent.y = static_cast<SmallExtentComponent>(std::max<cputex::ExtentComponent>(extent.y, 1));
                mExtent.z = static_cast<SmallExtentComponent>(std::max<cputex::ExtentComponent>(extent.z, 1));
            }
        }

        SurfaceView(TextureSurfaceView surfaceView) noexcept
            : SurfaceView(surfaceView.format(), surfaceView.dimension(), surfaceView.extent(), surfaceView.getData())
        {}

        constexpr SurfaceView(const SurfaceView&) noexcept = default;

        constexpr SurfaceView(SurfaceView&& other) noexcept
            : mFormat(other.mFormat)
            , mExtent(other.mExtent)
            , mData(other.mData)
        {
            other.mFormat = gpufmt::Format::UNDEFINED;
            other.mExtent = { 0, 0, 0 };
            other.mData = nullptr;
        }

        ~SurfaceView() = default;

        constexpr SurfaceView& operator=(const SurfaceView&) noexcept = default;

        constexpr SurfaceView& operator=(SurfaceView&& other) noexcept {
            mFormat = other.mFormat;
            mExtent = other.mExtent;
            mData = other.mData;

            other.mFormat = gpufmt::Format::UNDEFINED;
            other.mExtent = { 0, 0, 0 };
            other.mData = nullptr;

            return *this;
        }

        [[nodiscard]] constexpr bool operator==(std::nullptr_t)  const noexcept {
            return mData == nullptr;
        }

        [[nodiscard]] constexpr bool operator!=(std::nullptr_t) const noexcept {
            return mData != nullptr;
        }

        [[nodiscard]] constexpr operator bool() const noexcept {
            return mData != nullptr;
        }

        [[nodiscard]] constexpr bool empty() const noexcept {
            return mData == nullptr;
        }

        [[nodiscard]] constexpr Extent extent() const noexcept {
            return {mExtent.x, std::max<cputex::ExtentComponent>(mExtent.y, 1), std::max<cputex::ExtentComponent>(mExtent.z, 1)};
        }
        
        [[nodiscard]] constexpr TextureDimension dimension() const noexcept {
            if (mExtent.y < 1) { return TextureDimension::Texture1D; }
            else if (mExtent.z < 1) { return TextureDimension::Texture2D; }
            else { return TextureDimension::Texture3D; }
        }

        [[nodiscard]] constexpr gpufmt::Format format() const noexcept {
            return mFormat;
        }

        [[nodiscard]] cputex::SizeType sizeInBytes() const noexcept {
            const gpufmt::FormatInfo& formatInfo = gpufmt::formatInfo(mFormat);
            const auto blockExtent = extent() / formatInfo.blockExtent;

            return blockExtent.x * blockExtent.y * blockExtent.z * formatInfo.blockByteSize;
        }

        [[nodiscard]] cputex::SizeType volumeSliceByteSize() const noexcept {
            const gpufmt::FormatInfo& formatInfo = gpufmt::formatInfo(mFormat);
            const auto blockExtent = extent() / formatInfo.blockExtent;
            
            return blockExtent.x * blockExtent.y * formatInfo.blockByteSize;
        }

        [[nodiscard]] cputex::span<const cputex::byte> getData() const noexcept {
            return cputex::span<const cputex::byte>(mData, (size_t)sizeInBytes());
        }

        template<class T>
        [[nodiscard]] cputex::span<const T> getDataAs() const noexcept {
            return cputex::span<const T>(reinterpret_cast<const T*>(mData), ((size_t)sizeInBytes()) / sizeof(T));
        }

        // For 1d and 2d textures, calling with an index of 0 will be the same as calling getData().
        [[nodiscard]] cputex::SurfaceView getVolumeSlice(cputex::CountType volumeSlice) {
            if (volumeSlice >= mExtent.z) { return {}; }

            const TextureDimension viewDimension = dimension();
            cputex::Extent newExtent = extent();
            newExtent.z = 1;
            const cputex::SizeType sliceByteSize = volumeSliceByteSize();
            return SurfaceView(mFormat, (viewDimension == TextureDimension::Texture3D) ? TextureDimension::Texture2D : viewDimension, newExtent, getData().subspan(sliceByteSize * volumeSlice, sliceByteSize));
        }

        [[nodiscard]] constexpr bool equivalentLayout(const SurfaceView &other) const noexcept {
            return mFormat == other.mFormat && mExtent == other.mExtent;
        }

        [[nodiscard]] constexpr bool equivalentDimensions(const SurfaceView &other) const noexcept {
            return mExtent == other.mExtent;
        }

    private:
        using SmallExtentComponent = std::conditional_t<std::is_signed_v<cputex::ExtentComponent>, int16_t, uint16_t>;
        using SmallExtent = glm::tvec3<typename SmallExtentComponent>;

        gpufmt::Format mFormat = gpufmt::Format::UNDEFINED;
        // In other parts of the library, extent is generally initialized to be (1, 1, 1) and each component is always
        // assumed to be at least 1. Here, extent components can have a value of 0, to help encode the intended
        // dimension of the surface. For example, it is impossible to tell if an extent of (1, 1, 1) is a 1d, 2d, or 3d
        // surface. This deviation is to help reduce the size of the class to stay within two pointres width.
        SmallExtent mExtent{ 0, 0, 0 };
        const cputex::byte* mData = nullptr;
    };

    class SurfaceSpan
    {
    public:
        constexpr SurfaceSpan() noexcept = default;
        constexpr SurfaceSpan(gpufmt::Format format, cputex::TextureDimension dimension, const cputex::Extent& extent, cputex::span<cputex::byte> data) noexcept
            : mFormat(format)
            , mData(data.data())
        {
            if(dimension == TextureDimension::Texture1D)
            {
                mExtent.x = static_cast<SmallExtentComponent>(std::max<cputex::ExtentComponent>(extent.x, 1));
                mExtent.y = 0;
                mExtent.z = 0;
            }
            else if(dimension == TextureDimension::Texture2D || dimension == TextureDimension::TextureCube)
            {
                mExtent.x = static_cast<SmallExtentComponent>(std::max<cputex::ExtentComponent>(extent.x, 1));
                mExtent.y = static_cast<SmallExtentComponent>(std::max<cputex::ExtentComponent>(extent.y, 1));
                mExtent.z = 0;
            }
            else
            {
                mExtent.x = static_cast<SmallExtentComponent>(std::max<cputex::ExtentComponent>(extent.x, 1));
                mExtent.y = static_cast<SmallExtentComponent>(std::max<cputex::ExtentComponent>(extent.y, 1));
                mExtent.z = static_cast<SmallExtentComponent>(std::max<cputex::ExtentComponent>(extent.z, 1));
            }
        }

        SurfaceSpan(TextureSurfaceSpan surfaceSpan) noexcept
        {}

        constexpr SurfaceSpan(const SurfaceSpan&) noexcept = default;

        constexpr SurfaceSpan(SurfaceSpan&& other) noexcept
            : mFormat(other.mFormat)
            , mExtent(other.mExtent)
            , mData(other.mData)
        {
            other.mFormat = gpufmt::Format::UNDEFINED;
            other.mExtent = { 0, 0, 0 };
            other.mData = nullptr;
        }

        ~SurfaceSpan() = default;

        constexpr SurfaceSpan& operator=(const SurfaceSpan&) noexcept = default;

        constexpr SurfaceSpan& operator=(SurfaceSpan&& other) noexcept {
            mFormat = other.mFormat;
            mExtent = other.mExtent;
            mData = other.mData;

            other.mFormat = gpufmt::Format::UNDEFINED;
            other.mExtent = { 0, 0, 0 };
            other.mData = nullptr;

            return *this;
        }

        [[nodiscard]] constexpr bool operator==(std::nullptr_t)  const noexcept {
            return mData == nullptr;
        }

        [[nodiscard]] constexpr bool operator!=(std::nullptr_t) const noexcept {
            return mData != nullptr;
        }

        [[nodiscard]] constexpr operator SurfaceView() const noexcept {
            return SurfaceView(mFormat, dimension(), extent(), getData());
        }

        [[nodiscard]] constexpr operator bool() const noexcept {
            return mData != nullptr;
        }

        [[nodiscard]] constexpr bool empty() const noexcept {
            return mData == nullptr;
        }

        [[nodiscard]] constexpr Extent extent() const noexcept {
            return {mExtent.x, std::max<cputex::ExtentComponent>(mExtent.y, 1), std::max<cputex::ExtentComponent>(mExtent.z, 1)};
        }

        [[nodiscard]] constexpr TextureDimension dimension() const noexcept {
            if (mExtent.y < 1) { return TextureDimension::Texture1D; }
            else if (mExtent.z < 1) { return TextureDimension::Texture2D; }
            else { return TextureDimension::Texture3D; }
        }

        [[nodiscard]] constexpr gpufmt::Format format() const noexcept {
            return mFormat;
        }

        [[nodiscard]] cputex::SizeType sizeInBytes() const noexcept {
            const gpufmt::FormatInfo& formatInfo = gpufmt::formatInfo(mFormat);
            const auto blockExtent = extent() / formatInfo.blockExtent;

            return blockExtent.x * blockExtent.y * blockExtent.z * formatInfo.blockByteSize;
        }

        [[nodiscard]] cputex::SizeType volumeSliceByteSize() const noexcept {
            const gpufmt::FormatInfo& formatInfo = gpufmt::formatInfo(mFormat);
            const auto blockExtent = extent() / formatInfo.blockExtent;

            return blockExtent.x * blockExtent.y * formatInfo.blockByteSize;
        }

        [[nodiscard]] cputex::span<const cputex::byte> getData() const noexcept {
            return cputex::span<const cputex::byte>(mData, (size_t)sizeInBytes());
        }

        template<class T>
        [[nodiscard]] cputex::span<const T> getDataAs() const noexcept {
            return cputex::span<const T>(reinterpret_cast<const T*>(mData), ((size_t)sizeInBytes) / sizeof(T));
        }

        // For 1d and 2d textures, calling with an index of 0 will be the same as calling getData().
        [[nodiscard]] SurfaceView getVolumeSlice(cputex::CountType volumeSlice) const noexcept{
            if (volumeSlice >= mExtent.z) { return {}; }

            const TextureDimension viewDimension = dimension();
            cputex::Extent newExtent = extent();
            newExtent.z = 1;
            const cputex::SizeType sliceByteSize = volumeSliceByteSize();
            return SurfaceView(mFormat, (viewDimension == TextureDimension::Texture3D) ? TextureDimension::Texture2D : viewDimension, newExtent, getData().subspan(sliceByteSize * volumeSlice, sliceByteSize));
        }

        [[nodiscard]] cputex::span<cputex::byte> accessData() noexcept {
            return cputex::span<cputex::byte>(mData, (size_t)sizeInBytes());
        }

        template<class T>
        [[nodiscard]] cputex::span<T> accessDataAs() noexcept {
            return cputex::span<T>(reinterpret_cast<T*>(mData), ((size_t)sizeInBytes()) / sizeof(T));
        }

        // For 1d and 2d textures, calling with an index of 0 will be the same as calling getData().
        [[nodiscard]] SurfaceSpan accessVolumeSlice(cputex::CountType volumeSlice) noexcept {
            if (volumeSlice >= mExtent.z) { return {}; }

            const TextureDimension viewDimension = dimension();
            cputex::Extent newExtent = extent();
            newExtent.z = 1;
            const cputex::SizeType sliceByteSize = volumeSliceByteSize();
            return SurfaceSpan(mFormat, (viewDimension == TextureDimension::Texture3D) ? TextureDimension::Texture2D : viewDimension, newExtent, accessData().subspan(sliceByteSize * volumeSlice, sliceByteSize));
        }

        [[nodiscard]] constexpr bool equivalentLayout(const SurfaceSpan &other) const noexcept {
            return mFormat == other.mFormat && mExtent == other.mExtent;
        }

        [[nodiscard]] constexpr bool equivalentDimensions(const SurfaceSpan &other) const noexcept {
            return mExtent == other.mExtent;
        }

    private:
        using SmallExtentComponent = std::conditional_t<std::is_signed_v<cputex::ExtentComponent>, int16_t, uint16_t>;
        using SmallExtent = glm::tvec3<typename SmallExtentComponent>;

        gpufmt::Format mFormat = gpufmt::Format::UNDEFINED;
        // In other parts of the library, extent is generally initialized to be (1, 1, 1) and each component is always
        // assumed to be at least 1. Here, extent components can have a value of 0, to help encode the intended
        // dimension of the surface. For example, it is impossible to tell if an extent of (1, 1, 1) is a 1d, 2d, or 3d
        // surface. This deviation is to help reduce the size of the class to stay within two pointres width.
        SmallExtent mExtent{ 0, 0, 0 };
        cputex::byte* mData = nullptr;
    };

    static_assert(sizeof(SurfaceView) <= 16);
    static_assert(sizeof(SurfaceSpan) <= 16);


    namespace internal {
        constexpr BaseTextureSurfaceSpan::operator SurfaceView() const noexcept {
            return SurfaceView(format(), dimension(), extent(), getData());
        }
    }

    constexpr TextureSurfaceSpan::operator SurfaceSpan() noexcept {
        return SurfaceSpan(format(), dimension(), extent(), accessData());
    }
}