#pragma once

#include <cputex/config.h>
#include <cputex/definitions.h>

#include <glm/gtx/component_wise.hpp>

#include <gpufmt/format.h>
#include <gpufmt/info.h>

#include <atomic>
#include <memory>
#include <vector>

namespace cputex::internal {
    class TextureStorage {
    private:
        struct SurfaceInfo {
            cputex::SizeType offset;
            cputex::SizeType sizeInBytes;
        };
    public:
        struct Header {
            Header(int strongCount_, const TextureParams &params_, CountType surfaceCount_, SizeType sizeInBytes_)
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
            cputex::CountType surfaceCount;
            cputex::SizeType sizeInBytes;
            cputex::SizeType mipExtentsOffset;
            cputex::SizeType surfaceInfoOffset;
            cputex::SizeType surfaceDataOffset;
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

            params.surfaceByteAlignment = std::max(params.surfaceByteAlignment, cputex::CountType(1));

            params.extent.y = std::max(params.extent.y, cputex::ExtentComponent(1));
            params.extent.z = std::max(params.extent.z, cputex::ExtentComponent(1));

            const gpufmt::FormatInfo &info = gpufmt::formatInfo(params.format);

            std::vector<Extent> tempMipExtents;
            tempMipExtents.reserve(std::min(params.mips, cputex::CountType(15))); //max number of mips for a 16384 texture

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

                    mipExtent.x = std::max(mipExtent.x, ExtentComponent(1));
                    mipExtent.y = std::max(mipExtent.y, ExtentComponent(1));
                    mipExtent.z = std::max(mipExtent.z, ExtentComponent(1));
                }

                if(tempMipExtents.size() < params.mips && mipExtent.x == 1 && mipExtent.y == 1 && mipExtent.z == 1) {
                    tempMipExtents.emplace_back(1, 1, 1);
                }

                params.mips = static_cast<cputex::CountType>(tempMipExtents.size());
            }

            params.faces = std::max(params.faces, cputex::CountType(1));
            std::vector<SurfaceInfo> tempSurfaceInfos;
            cputex::SizeType sizeInBytes = 0u;

            tempSurfaceInfos.reserve(static_cast<size_t>(params.arraySize) * static_cast<size_t>(params.faces) * static_cast<size_t>(params.mips));
                
            for(cputex::CountType arraySlice = 0; arraySlice < params.arraySize; ++arraySlice) {
                for(cputex::CountType face = 0; face < params.faces; ++face) {
                    for(const Extent &mipExtent : tempMipExtents) {
                        const glm::tvec3<cputex::SizeType> mipExtentSizeT = mipExtent;
                        const glm::tvec3<cputex::SizeType> blockExtentSizeT = info.blockExtent;

                        SurfaceInfo surfaceInfo;
                        surfaceInfo.offset = sizeInBytes;
                        surfaceInfo.sizeInBytes = glm::compMul((mipExtentSizeT + (blockExtentSizeT - glm::tvec3<cputex::SizeType>(1))) / blockExtentSizeT) * info.blockByteSize;;
                        surfaceInfo.sizeInBytes = std::max(surfaceInfo.sizeInBytes, static_cast<cputex::SizeType>(info.blockByteSize));

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
                                                       static_cast<cputex::CountType>(tempSurfaceInfos.size()),
                                                       sizeInBytes);
            
            cputex::span<Extent> mipExtents{ reinterpret_cast<Extent*>(storage.get() + header->mipExtentsOffset), static_cast<cputex::span<Extent>::size_type>(header->params.mips) };
            std::copy(tempMipExtents.cbegin(), tempMipExtents.cend(), mipExtents.begin());

            cputex::span<SurfaceInfo> surfaceInfos{ reinterpret_cast<SurfaceInfo*>(storage.get() + header->surfaceInfoOffset), static_cast<cputex::span<Extent>::size_type>(header->surfaceCount) };
            std::copy(tempSurfaceInfos.cbegin(), tempSurfaceInfos.cend(), surfaceInfos.begin());

            cputex::span<cputex::byte> surfaceData{ storage.get() + header->surfaceDataOffset, static_cast<cputex::span<Extent>::size_type>(header->sizeInBytes) };
            std::copy_n(initialData.begin(), std::min(static_cast<cputex::SizeType>(initialData.size_bytes()), sizeInBytes), surfaceData.begin());

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
        const Extent &extent(cputex::CountType mip) const noexcept {
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
        cputex::CountType arraySize() const noexcept {
            return (mStorage) ? getHeader()->params.arraySize : 0u;
        }

        [[nodiscard]]
        cputex::CountType faces() const noexcept {
            return (mStorage) ? getHeader()->params.faces : 0u;
        }

        [[nodiscard]]
        cputex::CountType mips() const noexcept {
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
        cputex::CountType surfaceByteAligment() const noexcept {
            return (mStorage) ? getHeader()->params.surfaceByteAlignment : 0u;
        }

        [[nodiscard]]
        cputex::SizeType sizeInBytes() const noexcept {
            return (mStorage) ? getHeader()->sizeInBytes : 0u;
        }

        [[nodiscard]]
        cputex::SizeType sizeInBytes(cputex::CountType mip) const noexcept {
            return (mStorage) ? getSurfaceInfo(getSurfaceIndex(0u, 0u, mip)).sizeInBytes : 0u;
        }

        [[nodiscard]]
        cputex::CountType surfaceCount() const noexcept {
            return (mStorage) ? getHeader()->surfaceCount : 0u;
        }

        [[nodiscard]]
        cputex::IndexType getSurfaceIndex(cputex::CountType arraySlice, cputex::CountType face, cputex::CountType mip) const noexcept {
            const Header *header = getHeader();

            if(header == nullptr) {
                return cputex::IndexType(0);
            }

            return header->params.faces * header->params.mips * arraySlice +
                header->params.mips * face +
                mip;
        }

        [[nodiscard]]
        cputex::IndexType getSurfaceIndexUnsafe(cputex::CountType arraySlice, cputex::CountType face, cputex::CountType mip) const noexcept {
            const Header *header = getHeader();

            return header->params.faces * header->params.mips * arraySlice +
                header->params.mips * face +
                mip;
        }

        [[nodiscard]]
        cputex::span<const cputex::byte> get2DSurfaceData(cputex::CountType arraySlice, cputex::CountType face, cputex::CountType mip, cputex::CountType volumeSlice) const noexcept {
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
            cputex::SizeType volumeSliceByteSize = surfaceInfo.sizeInBytes / mipExtent.z;
            return getData().subspan(surfaceInfo.offset + (volumeSliceByteSize * volumeSlice), volumeSliceByteSize);
        }

        template<class T>
        [[nodiscard]]
        cputex::span<const T> get2DSurfaceDataAs(cputex::CountType arraySlice, cputex::CountType face, cputex::CountType mip, cputex::CountType volumeSlice) const noexcept {
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
            cputex::SizeType volumeSliceByteSize = surfaceInfo.sizeInBytes / mipExtent.z;
            auto byteSpan = getData().subspan(surfaceInfo.offset + (volumeSliceByteSize * volumeSlice), volumeSliceByteSize);

            return cputex::span<const T>{reinterpret_cast<const T*>(byteSpan.data()), byteSpan.size_bytes() / sizeof(T)};
        }

        [[nodiscard]]
        cputex::span<cputex::byte> access2DSurfaceData(cputex::CountType arraySlice, cputex::CountType face, cputex::CountType mip, cputex::CountType volumeSlice) noexcept {
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
            cputex::SizeType volumeSliceByteSize = surfaceInfo.sizeInBytes / mipExtent.z;
            return accessData().subspan(surfaceInfo.offset + (volumeSliceByteSize * volumeSlice), volumeSliceByteSize);
        }

        template<class T>
        [[nodiscard]]
        cputex::span<T> access2DSurfaceDataAs(cputex::CountType arraySlice, cputex::CountType face, cputex::CountType mip, cputex::CountType volumeSlice) noexcept {
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
            cputex::SizeType volumeSliceByteSize = surfaceInfo.sizeInBytes / mipExtent.z;
            auto byteSpan = accessData().subspan(surfaceInfo.offset + (volumeSliceByteSize * volumeSlice), volumeSliceByteSize);

            return cputex::span<T>{reinterpret_cast<T*>(byteSpan.data()), byteSpan.size_bytes() / sizeof(T)};
        }

        [[nodiscard]]
        cputex::span<const cputex::byte> getMipSurfaceData(cputex::CountType arraySlice, cputex::CountType face, cputex::CountType mip) const noexcept {
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
        cputex::span<const T> getMipSurfaceDataAs(cputex::CountType arraySlice, cputex::CountType face, cputex::CountType mip) const noexcept {
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
        cputex::span<cputex::byte> accessMipSurfaceData(cputex::CountType arraySlice, cputex::CountType face, cputex::CountType mip) noexcept {
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
        cputex::span<T> accessMipSurfaceDataAs(cputex::CountType arraySlice, cputex::CountType face, cputex::CountType mip) noexcept {
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
            return cputex::span<SurfaceInfo>(reinterpret_cast<SurfaceInfo*>(mStorage + header->surfaceInfoOffset), header->surfaceCount);
        }

        [[nodiscard]]
        cputex::span<const SurfaceInfo> getSurfaceInfos() const noexcept {
            const Header *header = getHeader();
            return cputex::span<const SurfaceInfo>(reinterpret_cast<const SurfaceInfo*>(mStorage + header->surfaceInfoOffset), header->surfaceCount);
        }

        [[nodiscard]]
        const SurfaceInfo& getSurfaceInfo(IndexType index) const noexcept {
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

        template<class T>
        [[nodiscard]]
        cputex::span<T> accessDataAs() noexcept {
            const Header *header = getHeader();
            return cputex::span<T>(reinterpret_cast<T*>(mStorage + header->surfaceDataOffset), header->sizeInBytes / sizeof(T));
        }

        [[nodiscard]]
        cputex::span<const cputex::byte> getData() const noexcept {
            const Header *header = getHeader();
            return cputex::span<const cputex::byte>(mStorage + header->surfaceDataOffset, header->sizeInBytes);
        }

        template<class T>
        [[nodiscard]]
        cputex::span<const T> getDataAs() const noexcept {
            const Header *header = getHeader();
            return cputex::span<const T>(reinterpret_cast<const T*>(mStorage + header->surfaceDataOffset, header->sizeInBytes / sizeof(T)));
        }

        cputex::byte *mStorage = nullptr;
    };
}