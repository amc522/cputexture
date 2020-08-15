#pragma once

#include <cputex/config.h>
#include <cputex/definitions.h>

#include <gpufmt/format.h>
#include <gpufmt/traits.h>

#include <atomic>
#include <memory>
#include <vector>

namespace cputex::internal {
    class TextureStorage {
    private:
        struct SurfaceInfo {
            size_t offset;
            size_t sizeInBytes;
        };
    public:
        struct Header {
            Header(int strongCount_, const TextureParams &params_, uint32_t surfaceCount_, size_t sizeInBytes_)
                : strongCount(strongCount_)
                , params(params_)
                , surfaceCount(surfaceCount_)
                , sizeInBytes(sizeInBytes_)
                , mipExtentsOffset(sizeof(Header))
            {
                surfaceInfoOffset = mipExtentsOffset + sizeof(Extent) * params.mips;
                surfaceDataOffset = surfaceInfoOffset + sizeof(SurfaceInfo) * surfaceCount;
            }

            mutable std::atomic_int strongCount;
            mutable std::atomic_int weakCount;
            TextureParams params;
            uint32_t surfaceCount;
            size_t sizeInBytes;
            size_t mipExtentsOffset;
            size_t surfaceInfoOffset;
            size_t surfaceDataOffset;
        };

        TextureStorage() noexcept= default;

        TextureStorage(const TextureParams &params, bool shared)
            : TextureStorage(params, cputex::span<const cputex::byte>(), shared)
        {}

        TextureStorage(TextureParams params, cputex::span<const cputex::byte> initialData, bool shared)
        {
            if(params.format == gpufmt::Format::UNDEFINED) {
                return;
            }

            if(params.mips == 0) {
                return;
            }

            if(params.arraySize == 0) {
                return;
            }

            if(params.extent.x <= 0) {
                return;
            }

            if(params.dimension != TextureDimension::Texture1D && params.extent.y <= 0) {
                return;
            }

            if(params.dimension == TextureDimension::Texture3D && params.extent.z <= 0) {
                return;
            }

            if(params.dimension == TextureDimension::TextureCube && params.faces != 6) {
                return;
            }

            params.surfaceByteAlignment = std::max(params.surfaceByteAlignment, 1u);

            params.extent.y = std::max(params.extent.y, decltype(params.extent.y)(1));
            params.extent.z = std::max(params.extent.z, decltype(params.extent.y)(1));

            const gpufmt::FormatInfo &info = gpufmt::formatInfo(params.format);

            std::vector<Extent> tempMipExtents;
            tempMipExtents.reserve(std::min(params.mips, 15u)); //max number of mips for a 16384 texture

            {
                Extent mipExtent = params.extent;

                while(tempMipExtents.size() < tempMipExtents.capacity() &&
                      (mipExtent.x > 1 || mipExtent.y > 1 || mipExtent.z > 1))
                {
                    tempMipExtents.emplace_back(mipExtent);

                    switch(params.dimension) {
                    case TextureDimension::Texture1D:
                        mipExtent.x >>= 1;
                        break;
                    case TextureDimension::Texture2D:
                        [[fallthrough]];
                    case TextureDimension::TextureCube:
                        mipExtent.x >>= 1;
                        mipExtent.y >>= 1;
                        break;
                    case TextureDimension::Texture3D:
                        mipExtent.x >>= 1;
                        mipExtent.y >>= 1;
                        mipExtent.z >>= 1;
                        break;
                    }

                    mipExtent.x = std::max(mipExtent.x, 1u);
                    mipExtent.y = std::max(mipExtent.y, 1u);
                    mipExtent.z = std::max(mipExtent.z, 1u);
                }

                if(tempMipExtents.size() < params.mips && mipExtent.x == 1 && mipExtent.y == 1 && mipExtent.z == 1) {
                    tempMipExtents.emplace_back(1, 1, 1);
                }

                params.mips = static_cast<uint32_t>(tempMipExtents.size());
            }

            params.faces = std::max(params.faces, 1u);
            std::vector<SurfaceInfo> tempSurfaceInfos;
            size_t sizeInBytes = 0u;

            tempSurfaceInfos.reserve(static_cast<size_t>(params.arraySize) * static_cast<size_t>(params.faces) * static_cast<size_t>(params.mips));
                
            for(uint32_t arraySlice = 0; arraySlice < params.arraySize; ++arraySlice) {
                for(uint32_t face = 0; face < params.faces; ++face) {
                    for(const Extent &mipExtent : tempMipExtents) {
                        SurfaceInfo surfaceInfo;
                        surfaceInfo.offset = sizeInBytes;
                        surfaceInfo.sizeInBytes = static_cast<size_t>((mipExtent.x + (info.blockExtent.x - 1)) / info.blockExtent.x) * ((mipExtent.y + (info.blockExtent.y - 1)) / info.blockExtent.y) * ((mipExtent.z + (info.blockExtent.z - 1)) / info.blockExtent.z) * info.blockByteSize;
                        surfaceInfo.sizeInBytes = std::max(surfaceInfo.sizeInBytes, static_cast<size_t>(info.blockByteSize));

                        //make sure everything is byte aligned
                        surfaceInfo.sizeInBytes = ((surfaceInfo.sizeInBytes + (params.surfaceByteAlignment - 1u)) / params.surfaceByteAlignment) * params.surfaceByteAlignment;
                        sizeInBytes += surfaceInfo.sizeInBytes;
                        tempSurfaceInfos.emplace_back(surfaceInfo);
                    }
                }
            }

            if(sizeInBytes == 0) {
                return;
            }

            std::unique_ptr<cputex::byte[]> storage = std::make_unique<cputex::byte[]>(sizeof(Header) + sizeof(SurfaceInfo) * tempSurfaceInfos.size() + sizeof(Extent) * tempMipExtents.size() + sizeInBytes);
            
            Header *header = new(storage.get()) Header(shared ? 1 : 0,
                                                       params,
                                                       static_cast<uint32_t>(tempSurfaceInfos.size()),
                                                       sizeInBytes);
            
            cputex::span<Extent> mipExtents{ reinterpret_cast<Extent*>(storage.get() + header->mipExtentsOffset), header->params.mips };
            std::copy(tempMipExtents.cbegin(), tempMipExtents.cend(), mipExtents.begin());

            cputex::span<SurfaceInfo> surfaceInfos{ reinterpret_cast<SurfaceInfo*>(storage.get() + header->surfaceInfoOffset), header->surfaceCount };
            std::copy(tempSurfaceInfos.cbegin(), tempSurfaceInfos.cend(), surfaceInfos.begin());

            cputex::span<cputex::byte> surfaceData{ storage.get() + header->surfaceDataOffset, header->sizeInBytes };
            std::copy_n(initialData.begin(), std::min(static_cast<size_t>(initialData.size_bytes()), sizeInBytes), surfaceData.begin());

            mStorage = storage.release();
        }

        TextureStorage(const TextureStorage &) noexcept = default;

        TextureStorage(TextureStorage &&other) noexcept
            : mStorage(other.mStorage)
        {
            other.mStorage = nullptr;
        }

        ~TextureStorage() = default;

        TextureStorage &operator=(const TextureStorage &) = default;
        TextureStorage &operator=(TextureStorage &&other) noexcept {
            if(mStorage != nullptr) {
                delete mStorage;
            }

            mStorage = other.mStorage;
            other.mStorage = nullptr;
            return *this;
        }
        
        [[nodiscard]]
        bool operator==(std::nullptr_t) const noexcept {
            return !isValid();
        }

        [[nodiscard]]
        bool operator!=(std::nullptr_t) const noexcept {
            return !operator==(nullptr);
        }

        void destroy() noexcept {
            if(isValid()) {
                delete[] mStorage;
                mStorage = nullptr;
            }
        }

        void addRef() const noexcept {
            if(mStorage) {
                ++getHeader()->strongCount;
            }
        }

        bool decRef() noexcept {
            if(mStorage) {
                if((--getHeader()->strongCount) == 0) {
                    return true;
                }
            }

            return false;
        }

        [[nodiscard]]
        const Extent &extent(uint32_t mip) const noexcept {
            static constexpr Extent zero{ 0u, 0u, 0u };

            if(mStorage == nullptr) {
                return zero;
            }

            if(mip >= getHeader()->params.mips) {
                return zero;
            }

            return getMipExtents()[mip];
        }

        [[nodiscard]]
        uint32_t arraySize() const noexcept {
            return (mStorage) ? getHeader()->params.arraySize : 0u;
        }

        [[nodiscard]]
        uint32_t faces() const noexcept {
            return (mStorage) ? getHeader()->params.faces : 0u;
        }

        [[nodiscard]]
        uint32_t mips() const noexcept {
            return (mStorage) ? getHeader()->params.mips : 0u;
        }

        [[nodiscard]]
        TextureDimension dimension() const noexcept {
            return (mStorage) ? getHeader()->params.dimension : TextureDimension::Texture2D;
        }

        [[nodiscard]]
        gpufmt::Format format() const noexcept {
            return (mStorage) ? getHeader()->params.format : gpufmt::Format::UNDEFINED;
        }
        
        [[nodiscard]]
        uint32_t surfaceByteAligment() const noexcept {
            return (mStorage) ? getHeader()->params.surfaceByteAlignment : 0u;
        }

        [[nodiscard]]
        size_t sizeInBytes() const noexcept {
            return (mStorage) ? getHeader()->sizeInBytes : 0u;
        }

        [[nodiscard]]
        size_t sizeInBytes(uint32_t mip) const noexcept {
            return (mStorage) ? getSurfaceInfo(getSurfaceIndex(0u, 0u, mip)).sizeInBytes : 0u;
        }

        [[nodiscard]]
        uint32_t surfaceCount() const noexcept {
            return (mStorage) ? getHeader()->surfaceCount : 0u;
        }

        [[nodiscard]]
        size_t getSurfaceIndex(uint32_t arraySlice, uint32_t face, uint32_t mip) const noexcept {
            const Header *header = getHeader();

            if(header == nullptr) {
                return 0u;
            }

            return static_cast<size_t>(header->params.faces) * static_cast<size_t>(header->params.mips) * static_cast<size_t>(arraySlice) +
                static_cast<size_t>(header->params.mips) * static_cast<size_t>(face) +
                static_cast<size_t>(mip);
        }

        [[nodiscard]]
        size_t getSurfaceIndexUnsafe(uint32_t arraySlice, uint32_t face, uint32_t mip) const noexcept {
            const Header *header = getHeader();

            return static_cast<size_t>(header->params.faces) *static_cast<size_t>(header->params.mips) *static_cast<size_t>(arraySlice) +
                static_cast<size_t>(header->params.mips) *static_cast<size_t>(face) +
                static_cast<size_t>(mip);
        }

        [[nodiscard]]
        cputex::span<const cputex::byte> get2DSurfaceData(uint32_t arraySlice, uint32_t face, uint32_t mip, uint32_t volumeSlice) const noexcept {
            const Header *header = getHeader();

            if(header == nullptr) {
                return {};
            }
            
            if(arraySlice >= header->params.arraySize) {
                return {};
            }

            if(face >= header->params.faces) {
                return {};
            }

            if(mip >= header->params.mips) {
                return {};
            }

            const Extent &mipExtent = extent(mip);
            if(volumeSlice >= mipExtent.z) {
                return {};
            }

            const SurfaceInfo &surfaceInfo = getSurfaceInfo(getSurfaceIndexUnsafe(arraySlice, face, mip));
            size_t volumeSliceByteSize = surfaceInfo.sizeInBytes / mipExtent.z;
            return getData().subspan(surfaceInfo.offset + (volumeSliceByteSize * volumeSlice), volumeSliceByteSize);
        }

        template<class T>
        [[nodiscard]]
        cputex::span<const T> get2DSurfaceDataAs(uint32_t arraySlice, uint32_t face, uint32_t mip, uint32_t volumeSlice) const noexcept {
            const Header *header = getHeader();

            if(header == nullptr) {
                return {};
            }

            if(arraySlice >= header->params.arraySize) {
                return {};
            }

            if(face >= header->params.faces) {
                return {};
            }

            if(mip >= header->params.mips) {
                return {};
            }

            const Extent &mipExtent = extent(mip);
            if(volumeSlice >= extent(mip).z) {
                return {};
            }

            const SurfaceInfo &surfaceInfo = getSurfaceInfo(getSurfaceIndexUnsafe(arraySlice, face, mip));
            size_t volumeSliceByteSize = surfaceInfo.sizeInBytes / mipExtent.z;
            auto byteSpan = getData().subspan(surfaceInfo.offset + (volumeSliceByteSize * volumeSlice), volumeSliceByteSize);

            return cputex::span<const T>{reinterpret_cast<const T*>(byteSpan.data()), byteSpan.size_bytes() / sizeof(T)};
        }

        [[nodiscard]]
        cputex::span<cputex::byte> access2DSurfaceData(uint32_t arraySlice, uint32_t face, uint32_t mip, uint32_t volumeSlice) noexcept {
            const Header *header = getHeader();

            if(header == nullptr) {
                return {};
            }

            if(arraySlice >= header->params.arraySize) {
                return {};
            }

            if(face >= header->params.faces) {
                return {};
            }

            if(mip >= header->params.mips) {
                return {};
            }

            const Extent &mipExtent = extent(mip);
            if(volumeSlice >= extent(mip).z) {
                return {};
            }

            const SurfaceInfo &surfaceInfo = getSurfaceInfo(getSurfaceIndexUnsafe(arraySlice, face, mip));
            size_t volumeSliceByteSize = surfaceInfo.sizeInBytes / mipExtent.z;
            return accessData().subspan(surfaceInfo.offset + (volumeSliceByteSize * volumeSlice), volumeSliceByteSize);
        }

        template<class T>
        [[nodiscard]]
        cputex::span<T> access2DSurfaceDataAs(uint32_t arraySlice, uint32_t face, uint32_t mip, uint32_t volumeSlice) noexcept {
            const Header *header = getHeader();

            if(header == nullptr) {
                return {};
            }

            if(arraySlice >= header->params.arraySize) {
                return {};
            }

            if(face >= header->params.faces) {
                return {};
            }

            if(mip >= header->params.mips) {
                return {};
            }

            const Extent &mipExtent = extent(mip);
            if(volumeSlice >= extent(mip).z) {
                return {};
            }

            const SurfaceInfo &surfaceInfo = getSurfaceInfo(getSurfaceIndexUnsafe(arraySlice, face, mip));
            size_t volumeSliceByteSize = surfaceInfo.sizeInBytes / mipExtent.z;
            auto byteSpan = accessData().subspan(surfaceInfo.offset + (volumeSliceByteSize * volumeSlice), volumeSliceByteSize);

            return cputex::span<T>{reinterpret_cast<T*>(byteSpan.data()), byteSpan.size_bytes() / sizeof(T)};
        }

        [[nodiscard]]
        cputex::span<const cputex::byte> getMipSurfaceData(uint32_t arraySlice, uint32_t face, uint32_t mip) const noexcept {
            const Header *header = getHeader();

            if(header == nullptr) {
                return {};
            }

            if(arraySlice >= header->params.arraySize) {
                return {};
            }

            if(face >= header->params.faces) {
                return {};
            }

            if(mip >= header->params.mips) {
                return {};
            }

            const SurfaceInfo &surfaceInfo = getSurfaceInfo(getSurfaceIndexUnsafe(arraySlice, face, mip));
            return getData().subspan(surfaceInfo.offset, surfaceInfo.sizeInBytes);
        }

        template<class T>
        [[nodiscard]]
        cputex::span<const T> getMipSurfaceDataAs(uint32_t arraySlice, uint32_t face, uint32_t mip) const noexcept {
            const Header *header = getHeader();

            if(header == nullptr) {
                return {};
            }

            if(arraySlice >= header->params.arraySize) {
                return {};
            }

            if(face >= header->params.faces) {
                return {};
            }

            if(mip >= header->params.mips) {
                return {};
            }

            const SurfaceInfo &surfaceInfo = getSurfaceInfo(getSurfaceIndexUnsafe(arraySlice, face, mip));
            auto byteSpan = getData().subspan(surfaceInfo.offset, surfaceInfo.sizeInBytes);

            return cputex::span<const T>{reinterpret_cast<const T*>(byteSpan.data()), byteSpan.size_bytes() / sizeof(T)};
        }

        [[nodiscard]]
        cputex::span<cputex::byte> accessMipSurfaceData(uint32_t arraySlice, uint32_t face, uint32_t mip) noexcept {
            const Header *header = getHeader();

            if(header == nullptr) {
                return {};
            }

            if(arraySlice >= header->params.arraySize) {
                return {};
            }

            if(face >= header->params.faces) {
                return {};
            }

            if(mip >= header->params.mips) {
                return {};
            }

            const SurfaceInfo &surfaceInfo = getSurfaceInfo(getSurfaceIndexUnsafe(arraySlice, face, mip));
            return accessData().subspan(surfaceInfo.offset, surfaceInfo.sizeInBytes);
        }

        template<class T>
        [[nodiscard]]
        cputex::span<T> accessMipSurfaceDataAs(uint32_t arraySlice, uint32_t face, uint32_t mip) noexcept {
            const Header *header = getHeader();

            if(header == nullptr) {
                return {};
            }

            if(arraySlice >= header->params.arraySize) {
                return {};
            }

            if(face >= header->params.faces) {
                return {};
            }

            if(mip >= header->params.mips) {
                return {};
            }
            
            const SurfaceInfo &surfaceInfo = getSurfaceInfo(getSurfaceIndexUnsafe(arraySlice, face, mip));
            auto byteSpan = accessData().subspan(surfaceInfo.offset, surfaceInfo.sizeInBytes);

            return cputex::span<T>{reinterpret_cast<T *>(byteSpan.data()), byteSpan.size_bytes() / sizeof(T)};
        }

        [[nodiscard]]
        bool isValid() const noexcept {
            return mStorage != nullptr;
        }

        [[nodiscard]]
        Header* getHeader() noexcept {
            return reinterpret_cast<Header*>(mStorage);
        }

        [[nodiscard]]
        const Header* getHeader() const noexcept {
            return reinterpret_cast<const Header*>(mStorage);
        }

        [[nodiscard]]
        cputex::span<SurfaceInfo> getSurfaceInfos() noexcept {
            const Header *header = getHeader();
            return cputex::span<SurfaceInfo>(reinterpret_cast<SurfaceInfo *>(mStorage + header->surfaceInfoOffset), header->surfaceCount);
        }

        [[nodiscard]]
        cputex::span<const SurfaceInfo> getSurfaceInfos() const noexcept {
            const Header *header = getHeader();
            return cputex::span<const SurfaceInfo>(reinterpret_cast<const SurfaceInfo*>(mStorage + header->surfaceInfoOffset), header->surfaceCount);
        }

        [[nodiscard]]
        const SurfaceInfo& getSurfaceInfo(size_t index) const noexcept {
            return getSurfaceInfos()[index];
        }

        [[nodiscard]]
        cputex::span<Extent> getMipExtents() noexcept {
            const Header *header = getHeader();
            return cputex::span<Extent>(reinterpret_cast<Extent*>(mStorage + header->mipExtentsOffset), header->params.mips);
        }

        [[nodiscard]]
        cputex::span<const Extent> getMipExtents() const noexcept {
            const Header *header = getHeader();
            return cputex::span<const Extent>(reinterpret_cast<const Extent*>(mStorage + header->mipExtentsOffset), header->params.mips);
        }

        [[nodiscard]]
        cputex::span<cputex::byte> accessData() noexcept {
            const Header *header = getHeader();
            return cputex::span<cputex::byte>(mStorage + header->surfaceDataOffset, header->sizeInBytes);
        }

        [[nodiscard]]
        cputex::span<const cputex::byte> getData() const noexcept {
            const Header *header = getHeader();
            return cputex::span<const cputex::byte>(mStorage + header->surfaceDataOffset, header->sizeInBytes);
        }

        cputex::byte *mStorage = nullptr;
    };
}