//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author(s):  James Stanard
//

#pragma once

#include "GpuResource.h"

class Direct3D;
class Texture : public GpuResource
{
    friend class Direct3D;

public:

    Texture() { m_hCpuDescriptorHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN; }
    Texture(D3D12_CPU_DESCRIPTOR_HANDLE Handle) : m_hCpuDescriptorHandle(Handle) {}

    // Create a 1-level textures
    void Create2D(Direct3D* direct_3d, size_t RowPitchBytes, size_t Width, size_t Height, DXGI_FORMAT Format, const void* InitData );
    void CreateCube(Direct3D* direct_3d, size_t RowPitchBytes, size_t Width, size_t Height, DXGI_FORMAT Format, const void* InitialData);

    void CreateTGAFromMemory(Direct3D* direct_3d, const void* memBuffer, size_t fileSize, bool sRGB );
    bool CreateDDSFromMemory(Direct3D* direct_3d, const void* memBuffer, size_t fileSize, bool sRGB );
    void CreatePIXImageFromMemory(Direct3D* direct_3d, const void* memBuffer, size_t fileSize );

    virtual void Destroy() override
    {
        GpuResource::Destroy();
        m_hCpuDescriptorHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
    }

    const D3D12_CPU_DESCRIPTOR_HANDLE& GetSRV() const { return m_hCpuDescriptorHandle; }

    uint32_t GetWidth() const { return m_Width; }
    uint32_t GetHeight() const { return m_Height; }
    uint32_t GetDepth() const { return m_Depth; }

protected:

    uint32_t m_Width;
    uint32_t m_Height;
    uint32_t m_Depth;

    D3D12_CPU_DESCRIPTOR_HANDLE m_hCpuDescriptorHandle;
};
