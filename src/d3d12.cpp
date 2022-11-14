#ifdef _WIN32

#include <cputex/d3d12.h>

#include <d3d12.h>
#include <directx/d3dx12.h>
#include <gpufmt/dxgi.h>
#include <array>

namespace cputex::d3d12
{
D3D12_RESOURCE_DIMENSION getD3d12ResourceDimension(cputex::TextureDimension dimension)
{
    switch(dimension)
    {
    case cputex::TextureDimension::Texture1D:
        return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
    case cputex::TextureDimension::Texture2D:
        [[fallthrough]];
    case cputex::TextureDimension::TextureCube:
        return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    case cputex::TextureDimension::Texture3D:
        return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
    default:
        return D3D12_RESOURCE_DIMENSION_UNKNOWN;
    }
}

tl::expected<CreateTextureResult, HRESULT> createTexture(ID3D12Device* d3d12Device, const cputex::TextureParams& params, const cputex::d3d12::TextureParams& d3d12Params)
{
    if(d3d12Device == nullptr) { return tl::make_unexpected(E_INVALIDARG); }

    if(d3d12Params.placedResource && d3d12Params.placedParams.heap == nullptr)
    {
        return tl::make_unexpected(E_INVALIDARG);
    }

    const auto formatConversion = gpufmt::dxgi::translateFormat(params.format);

    if(!formatConversion.exact) { return tl::make_unexpected(E_INVALIDARG); }

    DXGI_FORMAT dxgiFormat = formatConversion.exact.value();

    if(d3d12Params.useTypelessFormat)
    {
        dxgiFormat = gpufmt::dxgi::typelessFormat(dxgiFormat).value_or(dxgiFormat);
    }

    //-----------------------------------------------------------------------------------
    // 1.a.
    // Get the alignment and size of the resource that will actually be used on the gpu.
    // ----------------------------------------------------------------------------------
    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.Dimension = getD3d12ResourceDimension(params.dimension);
    textureDesc.Alignment = 0;
    textureDesc.Width = (UINT64)params.extent.x;
    textureDesc.Height = (UINT64)params.extent.y;
    textureDesc.DepthOrArraySize =
        (params.dimension == cputex::TextureDimension::Texture3D) ? (UINT16)params.extent.z : (UINT16)params.arraySize;
    textureDesc.MipLevels = (UINT16)params.mips;
    textureDesc.Format = dxgiFormat;
    textureDesc.SampleDesc = d3d12Params.sampleDesc;
    textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    textureDesc.Flags = d3d12Params.resourceFlags;

    // Here's and article describing in detail what this memory alignment stuff is all about:
    // https://asawicki.info/news_1726_secrets_of_direct3d_12_resource_alignment

    // https://docs.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12device-getresourceallocationinfo
    D3D12_RESOURCE_ALLOCATION_INFO allocInfo;
    allocInfo = d3d12Device->GetResourceAllocationInfo(0, 1, &textureDesc);
    textureDesc.Alignment = allocInfo.Alignment;

    const D3D12_CLEAR_VALUE* clearValue = (d3d12Params.clearValue) ? &d3d12Params.clearValue.value() : nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> resource;
    HRESULT hr;

    if(d3d12Params.placedResource)
    {
        // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12device-createplacedresource
        hr = d3d12Device->CreatePlacedResource(
                d3d12Params.placedParams.heap,
                d3d12Params.placedParams.heapOffset,
                &textureDesc, d3d12Params.initialResourceState,
                clearValue,
                IID_PPV_ARGS(&resource));
    }
    else
    {
        // https://docs.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12device-createcommittedresource
        hr = d3d12Device->CreateCommittedResource(
                &d3d12Params.committedParams.heapProperties,
                d3d12Params.committedParams.heapFlags,
                &textureDesc,
                d3d12Params.initialResourceState,
                clearValue,
                IID_PPV_ARGS(&resource));
    }

    if(FAILED(hr))
    {
        return tl::make_unexpected(hr);
    }

    const auto subresourceCount = params.arraySize * params.faces * params.mips;
    
    CreateTextureResult result;
    result.textureResource = std::move(resource);
    result.uploadBufferByteSize = GetRequiredIntermediateSize(result.textureResource.Get(), 0, (UINT)subresourceCount);

    return result;
}

tl::expected<CreateTextureResult, HRESULT> createTexture(ID3D12Device* d3d12Device, cputex::TextureView sourceTexture, const cputex::d3d12::TextureParams& d3d12Params)
{
    cputex::TextureParams params;
    params.arraySize = sourceTexture.arraySize();
    params.dimension = sourceTexture.dimension();
    params.extent = sourceTexture.extent();
    params.faces = sourceTexture.faces();
    params.format = sourceTexture.format();
    params.mips = sourceTexture.mips();
    params.surfaceByteAlignment = sourceTexture.surfaceByteAlignment();

    return createTexture(d3d12Device, params, d3d12Params);
}

tl::expected<Microsoft::WRL::ComPtr<ID3D12Resource>, HRESULT> createUploadBuffer(ID3D12Device* d3d12Device, const cputex::d3d12::UploadBufferParams& d3d12Params)
{
    if(d3d12Device == nullptr) { return tl::make_unexpected(E_INVALIDARG); }

    if(d3d12Params.placedResource && d3d12Params.placedParams.heap == nullptr) { return tl::make_unexpected(E_INVALIDARG); }

    D3D12_RESOURCE_DESC uploadDesc = {};
    uploadDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    uploadDesc.Alignment = 0;
    uploadDesc.Width = (UINT64)d3d12Params.byteSize;
    uploadDesc.Height = 1;
    uploadDesc.DepthOrArraySize = 1;
    uploadDesc.MipLevels = 1;
    uploadDesc.Format = DXGI_FORMAT_UNKNOWN;
    uploadDesc.SampleDesc.Count = 1;
    uploadDesc.SampleDesc.Quality = 0;
    uploadDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    uploadDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    // https://docs.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12device-getresourceallocationinfo
    D3D12_RESOURCE_ALLOCATION_INFO allocInfo;
    allocInfo = d3d12Device->GetResourceAllocationInfo(0, 1, &uploadDesc);
    uploadDesc.Alignment = allocInfo.Alignment;

    Microsoft::WRL::ComPtr<ID3D12Resource> uploadResource;

    HRESULT hr;

    if(d3d12Params.placedResource)
    {
        hr = d3d12Device->CreatePlacedResource(
            d3d12Params.placedParams.heap,
            d3d12Params.placedParams.heapOffset,
            &uploadDesc,
            D3D12_RESOURCE_STATE_COPY_SOURCE,
            nullptr,
            IID_PPV_ARGS(&uploadResource));
    }
    else
    {
        hr = d3d12Device->CreateCommittedResource(&d3d12Params.committedParams.heapProperties,
            d3d12Params.committedParams.heapFlags,
            &uploadDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&uploadResource));
    }

    if(FAILED(hr)) { return tl::make_unexpected(hr); }

    return uploadResource;
}

HRESULT uploadTexture(ID3D12GraphicsCommandList* commandList, ID3D12Resource* destResource, ID3D12Resource* intermediateResource, TextureView sourceTexture)
{
    // Using 15 because its the maximum number of mips a 16384x16384 texture can have plus 1.
    std::array<D3D12_SUBRESOURCE_DATA, 16> subresources;
    UINT subresourceCount = 0;
    UINT firstSubresource = 0;
    
    const gpufmt::FormatInfo& surfaceFormatInfo = gpufmt::formatInfo(sourceTexture.format());

    for(int32_t arraySlice = 0; arraySlice < sourceTexture.arraySize(); ++arraySlice)
    {
        for(int32_t face = 0; face < sourceTexture.faces(); ++face)
        {
            for(int32_t mip = 0; mip < sourceTexture.mips(); ++mip)
            {
                // putting this assert here to check that surfaces are being access in order of incrementing d3d12
                // subresource index
                assert(arraySlice * sourceTexture.faces() * sourceTexture.mips() +
                       face * sourceTexture.mips() + mip ==
                       (int64_t)D3D12CalcSubresource(mip, arraySlice * sourceTexture.faces() + face, 0,
                       sourceTexture.mips(), sourceTexture.arraySize() * sourceTexture.faces()));

                TextureSurfaceView surface = sourceTexture.getMipSurface(arraySlice, face, mip);

                const auto rowPitch = surfaceFormatInfo.blockByteSize * (surface.extent().x / surfaceFormatInfo.blockExtent.x);

                D3D12_SUBRESOURCE_DATA& subresourceData = subresources[subresourceCount++];
                subresourceData.pData = surface.getData().data();
                subresourceData.RowPitch = (LONG_PTR)rowPitch;
                subresourceData.SlicePitch = (LONG_PTR)surface.sizeInBytes();

                if(subresourceCount == subresources.size())
                {
                    UpdateSubresources<subresources.size()>(commandList, destResource, intermediateResource, 0,
                                                            firstSubresource, subresourceCount, subresources.data());

                    firstSubresource = subresourceCount;
                    subresourceCount = 0;
                }
            }
        }
    }

    if(subresourceCount != 0)
    {
        UpdateSubresources<subresources.size()>(commandList, destResource, intermediateResource, 0, firstSubresource,
                                                subresourceCount, subresources.data());
    }

    return S_OK;
}

tl::expected<CreateAndUploadResult, HRESULT>
createTextureAndUpload(ID3D12Device* d3d12Device,
    ID3D12GraphicsCommandList* commandList,
    TextureView sourceTexture,
    const cputex::d3d12::TextureParams& d3d12TextureParams,
    ID3D12Resource* uploadBufferResource)
{
    tl::expected<CreateTextureResult, HRESULT> createTextureResult = createTexture(d3d12Device, sourceTexture, d3d12TextureParams);

    if(!createTextureResult) { return tl::make_unexpected(createTextureResult.error()); }

    D3D12_RESOURCE_DESC uploadBufferDesc = uploadBufferResource->GetDesc();

    if(uploadBufferDesc.Width < createTextureResult->uploadBufferByteSize) { return tl::make_unexpected(E_INVALIDARG); }

    HRESULT uploadResult = uploadTexture(commandList, createTextureResult->textureResource.Get(), uploadBufferResource, sourceTexture);

    if(FAILED(uploadResult)) { return tl::make_unexpected(uploadResult); }

    CreateAndUploadResult result;
    result.textureResource = std::move(createTextureResult->textureResource);
    result.uploadResource = uploadBufferResource;

    return result;
}

tl::expected<CreateAndUploadResult, HRESULT>
createTextureAndUpload(ID3D12Device* d3d12Device,
    ID3D12GraphicsCommandList* commandList,
    TextureView sourceTexture,
    const cputex::d3d12::TextureParams& d3d12TextureParams,
    const cputex::d3d12::UploadBufferParams& d3d12UploadBufferParams)
{
    tl::expected<CreateTextureResult, HRESULT> createTextureResult = createTexture(d3d12Device, sourceTexture, d3d12TextureParams);

    if(!createTextureResult) { return tl::make_unexpected(createTextureResult.error()); }

    cputex::d3d12::UploadBufferParams newUploadParams = d3d12UploadBufferParams;
    newUploadParams.byteSize = std::max(createTextureResult->uploadBufferByteSize, d3d12UploadBufferParams.byteSize);

    tl::expected<Microsoft::WRL::ComPtr<ID3D12Resource>, HRESULT> createUploadBufferResult = createUploadBuffer(d3d12Device, newUploadParams);

    if(!createUploadBufferResult) { return tl::make_unexpected(createUploadBufferResult.error()); }

    HRESULT uploadResult = uploadTexture(commandList, createTextureResult->textureResource.Get(), createUploadBufferResult->Get(), sourceTexture);

    if(FAILED(uploadResult)) { return tl::make_unexpected(uploadResult); }

    CreateAndUploadResult result;
    result.textureResource = std::move(createTextureResult->textureResource);
    result.uploadResource = std::move(createUploadBufferResult.value());

    return result;
}

}

#endif // _WIN32