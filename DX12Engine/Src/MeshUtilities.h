#ifndef _MESHUTILITIES_H_
#define _MESHUTILITIES_H_
#pragma once

#include <windows.h>
#include <wrl.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include <string>
#include <algorithm>
#include <vector>
#include <array>
#include <unordered_map>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <cassert>
#include <filesystem>

#include "d3dx12.h"
#include "Direct3D.h"
#include "DirectXTex.h"

namespace MeshUtilities
{
    struct ObjectConstants
    {
	    ObjectConstants()
	    {
            DirectX::XMStoreFloat4x4(&WorldViewProj, DirectX::XMMatrixIdentity());
	    }

	    DirectX::XMFLOAT4X4 WorldViewProj;
    };


    struct SubmeshGeometry
    {
        UINT IndexCount = 0;
        UINT StartIndexLocation = 0;
        INT BaseVertexLocation = 0;
        std::string MaterialName;
        // Bounding box of the geometry defined by this submesh. 
        // This is used in later chapters of the book.
        DirectX::BoundingBox Bounds;
    };

    struct MeshGeometry
    {
        // System memory copies.  Use Blobs because the vertex/index format can be generic.
        // It is up to the client to cast appropriately.  
        Microsoft::WRL::ComPtr<ID3DBlob> VertexBufferCPU = nullptr;
        Microsoft::WRL::ComPtr<ID3DBlob> IndexBufferCPU = nullptr;

        Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;
        Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;

        Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferUploader = nullptr;
        Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferUploader = nullptr;

        // Data about the buffers.
        UINT VertexByteStride = 0;
        UINT VertexBufferByteSize = 0;
        DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
        UINT IndexBufferByteSize = 0;

        // A MeshGeometry may store multiple geometries in one vertex/index buffer.
        // Use this container to define the Submesh geometries so we can draw
        // the Submeshes individually.
        std::unordered_map<int, SubmeshGeometry> DrawArgs;

        D3D12_VERTEX_BUFFER_VIEW VertexBufferView()const
        {
            D3D12_VERTEX_BUFFER_VIEW vbv;
            vbv.BufferLocation = VertexBufferGPU->GetGPUVirtualAddress();
            vbv.StrideInBytes = VertexByteStride;
            vbv.SizeInBytes = VertexBufferByteSize;

            return vbv;
        }

        D3D12_INDEX_BUFFER_VIEW IndexBufferView()const
        {
            D3D12_INDEX_BUFFER_VIEW ibv;
            ibv.BufferLocation = IndexBufferGPU->GetGPUVirtualAddress();
            ibv.Format = IndexFormat;
            ibv.SizeInBytes = IndexBufferByteSize;

            return ibv;
        }

        // We can free this memory after we finish upload to the GPU.
        void DisposeUploaders()
        {
            VertexBufferUploader = nullptr;
            IndexBufferUploader = nullptr;
        }
    };

    struct MeshTexture
    {
        Microsoft::WRL::ComPtr<ID3D12Resource> TextureBufferGPU = nullptr;

        Microsoft::WRL::ComPtr<ID3D12Resource> TextureBufferUploader = nullptr;

        bool LoadTextureToCpuBuffer(std::wstring filename, bool makeSRGB)
        {
            std::vector<D3D12_SUBRESOURCE_DATA> subresource_datas;
            // Load the dds texture in.
            std::filesystem::path pathToFile = filename;
            if (!std::filesystem::exists(pathToFile))
            {
                return false;
            }

            if (pathToFile.extension() == ".dds")
            {
                // Use DDS texture loader.
                ThrowIfFailed(LoadFromDDSFile(filename.c_str(), DirectX::DDS_FLAGS_NONE, &m_Metadata, m_ScratchImage));
            }
            else if (pathToFile.extension() == ".hdr")
            {
                ThrowIfFailed(LoadFromHDRFile(filename.c_str(), &m_Metadata, m_ScratchImage));
            }
            else if (pathToFile.extension() == ".tga")
            {
                ThrowIfFailed(LoadFromTGAFile(filename.c_str(), &m_Metadata, m_ScratchImage));
            }
            else
            {
                ThrowIfFailed(LoadFromWICFile(filename.c_str(), DirectX::WIC_FLAGS_NONE, &m_Metadata, m_ScratchImage));
            }

            if (makeSRGB)
            {
                m_Metadata.format = DirectX::MakeSRGB(m_Metadata.format);
            }



            switch (m_Metadata.dimension)
            {
            case DirectX::TEX_DIMENSION_TEXTURE1D:
                m_TextureDesc = CD3DX12_RESOURCE_DESC::Tex1D(m_Metadata.format, static_cast<UINT64>(m_Metadata.width), static_cast<UINT16>(m_Metadata.arraySize));
                break;
            case DirectX::TEX_DIMENSION_TEXTURE2D:
                m_TextureDesc = CD3DX12_RESOURCE_DESC::Tex2D(m_Metadata.format, static_cast<UINT64>(m_Metadata.width), static_cast<UINT>(m_Metadata.height), static_cast<UINT16>(m_Metadata.arraySize));
                break;
            case DirectX::TEX_DIMENSION_TEXTURE3D:
                m_TextureDesc = CD3DX12_RESOURCE_DESC::Tex3D(m_Metadata.format, static_cast<UINT64>(m_Metadata.width), static_cast<UINT>(m_Metadata.height), static_cast<UINT16>(m_Metadata.depth));
                break;
            default:
                throw std::exception("Invalid texture dimension.");
                break;
            }
        }


    private:
        DirectX::TexMetadata m_Metadata;
        DirectX::ScratchImage m_ScratchImage;
        D3D12_RESOURCE_DESC m_TextureDesc;
    };
    


}

class DefaultBufferCreator
{
public:
   static Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultBuffer(
        Direct3D* direct_3d,
        const void* initData,
        UINT64 byteSize,
        Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer)
    {
        Microsoft::WRL::ComPtr<ID3D12Resource> defaultBuffer;
        CD3DX12_HEAP_PROPERTIES defaulHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        CD3DX12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
        // Create the actual default buffer resource.
        ThrowIfFailed(direct_3d->GetD3D12Device()->CreateCommittedResource(
            &defaulHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            IID_PPV_ARGS(defaultBuffer.GetAddressOf())));

        // In order to copy CPU memory data into our default buffer, we need to create
        // an intermediate upload heap. 
        ThrowIfFailed(direct_3d->GetD3D12Device()->CreateCommittedResource(
            &uploadHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(uploadBuffer.GetAddressOf())));


        // Describe the data we want to copy into the default buffer.
        D3D12_SUBRESOURCE_DATA subResourceData = {};
        subResourceData.pData = initData;
        subResourceData.RowPitch = byteSize;
        subResourceData.SlicePitch = subResourceData.RowPitch;

        // Schedule to copy the data to the default buffer resource.  At a high level, the helper function UpdateSubresources
        // will copy the CPU memory into the intermediate upload heap.  Then, using ID3D12CommandList::CopySubresourceRegion,
        // the intermediate upload heap data will be copied to mBuffer.
        CD3DX12_RESOURCE_BARRIER rsb = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
            D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
        direct_3d->GetCommandList()->ResourceBarrier(1, &rsb);
        UpdateSubresources<1>(direct_3d->GetCommandList().Get(), defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResourceData);
        CD3DX12_RESOURCE_BARRIER rsb2 = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
            D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
        direct_3d->GetCommandList()->ResourceBarrier(1, &rsb2);

        // Note: uploadBuffer has to be kept alive after the above function calls because
        // the command list has not been executed yet that performs the actual copy.
        // The caller can Release the uploadBuffer after it knows the copy has been executed.


        return defaultBuffer;
    }
};

template<typename T>
class EasyUploadBuffer
{
public:
    EasyUploadBuffer(ID3D12Device* device, UINT elementCount, bool isConstantBuffer) :
        mIsConstantBuffer(isConstantBuffer)
    {
        mElementByteSize = sizeof(T);

        // Constant buffer elements need to be multiples of 256 bytes.
        // This is because the hardware can only view constant data 
        // at m*256 byte offsets and of n*256 byte lengths. 
        // typedef struct D3D12_CONSTANT_BUFFER_VIEW_DESC {
        // UINT64 OffsetInBytes; // multiple of 256
        // UINT   SizeInBytes;   // multiple of 256
        // } D3D12_CONSTANT_BUFFER_VIEW_DESC;
        if (isConstantBuffer)
            mElementByteSize = CalcConstantBufferByteSize(sizeof(T));

        ThrowIfFailed(device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(mElementByteSize * elementCount),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&mUploadBuffer)));

        ThrowIfFailed(mUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mMappedData)));

        // We do not need to unmap until we are done with the resource.  However, we must not write to
        // the resource while it is in use by the GPU (so we must use synchronization techniques).
    }

    EasyUploadBuffer(const UploadBuffer& rhs) = delete;
    EasyUploadBuffer& operator=(const UploadBuffer& rhs) = delete;
    ~EasyUploadBuffer()
    {
        if (mUploadBuffer != nullptr)
            mUploadBuffer->Unmap(0, nullptr);

        mMappedData = nullptr;
    }

    ID3D12Resource* Resource()const
    {
        return mUploadBuffer.Get();
    }

    void CopyData(int elementIndex, const T& data)
    {
        memcpy(&mMappedData[elementIndex * mElementByteSize], &data, sizeof(T));
    }


    static UINT CalcConstantBufferByteSize(UINT byteSize)
    {
        // Constant buffers must be a multiple of the minimum hardware
        // allocation size (usually 256 bytes).  So round up to nearest
        // multiple of 256.  We do this by adding 255 and then masking off
        // the lower 2 bytes which store all bits < 256.
        // Example: Suppose byteSize = 300.
        // (300 + 255) & ~255
        // 555 & ~255
        // 0x022B & ~0x00ff
        // 0x022B & 0xff00
        // 0x0200
        // 512
        return (byteSize + 255) & ~255;
    }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> mUploadBuffer;
    BYTE* mMappedData = nullptr;

    UINT mElementByteSize = 0;
    bool mIsConstantBuffer = false;
};

#endif
