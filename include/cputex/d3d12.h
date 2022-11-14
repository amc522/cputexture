#ifdef _WIN32

#pragma once

#include <cputex/texture_view.h>
#include <d3d12.h>
#include <sdkddkver.h>
#include <tl/expected.hpp>
#include <wrl/client.h>
#include <winsdkver.h>
#include <optional>

namespace cputex::d3d12
{
struct CommittedResourceParams
{
    D3D12_HEAP_PROPERTIES heapProperties;
    D3D12_HEAP_FLAGS heapFlags = D3D12_HEAP_FLAG_NONE;
};

struct PlacedResourceParams
{
    ID3D12Heap* heap = nullptr;
    uint64_t heapOffset = 0;
};

struct TextureParams
{
    DXGI_SAMPLE_DESC sampleDesc = {1, 0};
    D3D12_RESOURCE_FLAGS resourceFlags = D3D12_RESOURCE_FLAG_NONE;
    D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_COMMON;
    std::optional<D3D12_CLEAR_VALUE> clearValue;

    union
    {
        CommittedResourceParams committedParams;
        PlacedResourceParams placedParams;
    };
    
    bool placedResource = true;
    bool useTypelessFormat = true;
};

struct UploadBufferParams
{
    union
    {
        CommittedResourceParams committedParams;
        PlacedResourceParams placedParams;
    };

    uint64_t byteSize = 0;

    bool placedResource = true;
};

struct CreateTextureResult
{
    Microsoft::WRL::ComPtr<ID3D12Resource> textureResource;
    uint64_t uploadBufferByteSize = 0;
};

struct CreateAndUploadResult
{
    Microsoft::WRL::ComPtr<ID3D12Resource> textureResource;
    Microsoft::WRL::ComPtr<ID3D12Resource> uploadResource;
};

[[nodiscard]] tl::expected<CreateTextureResult, HRESULT>
createTexture(ID3D12Device* d3d12Device,
              const cputex::TextureParams& params,
              const cputex::d3d12::TextureParams& d3d12Params);

[[nodiscard]] tl::expected<CreateTextureResult, HRESULT> 
createTexture(ID3D12Device* d3d12Device,
              TextureView sourceTexture,
              const cputex::d3d12::TextureParams& d3d12Params);

[[nodiscard]] tl::expected<Microsoft::WRL::ComPtr<ID3D12Resource>, HRESULT>
createUploadBuffer(ID3D12Device* d3d12Device, const cputex::d3d12::UploadBufferParams& d3d12Params);

HRESULT uploadTexture(ID3D12GraphicsCommandList* commandList, 
                      ID3D12Resource* destResource,
                      ID3D12Resource* intermediateResource,
                      TextureView sourceTexture);

[[nodiscard]] tl::expected<CreateAndUploadResult, HRESULT>
createTextureAndUpload(ID3D12Device* d3d12Device,
                       ID3D12GraphicsCommandList* commandList,
                       TextureView sourceTexture,
                       const cputex::d3d12::TextureParams& d3d12TextureParams,
                       ID3D12Resource* uploadBufferResource);

[[nodiscard]] tl::expected<CreateAndUploadResult, HRESULT>
createTextureAndUpload(ID3D12Device* d3d12Device, 
                       ID3D12GraphicsCommandList* commandList,
                       TextureView sourceTexture,
                       const cputex::d3d12::TextureParams& d3d12TextureParams,
                       const cputex::d3d12::UploadBufferParams& d3d12UploadBufferParams);

[[nodiscard]] uint64_t calcRequiredUploadBufferByteSize(ID3D12Resource* textureResource);
}

#endif // _WIN32