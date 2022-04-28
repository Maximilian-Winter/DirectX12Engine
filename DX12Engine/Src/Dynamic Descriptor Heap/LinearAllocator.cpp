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
//             Alex Nankervis
//

#include "LinearAllocator.h"
#include <thread>

#include "Utility.h"
#include "Math/Common.h"
#include "../Direct3D.h"

LinearAllocatorType LinearAllocatorPageManager::sm_AutoType = kGpuExclusive;

LinearAllocatorPageManager::LinearAllocatorPageManager()
{
    m_AllocationType = sm_AutoType;
    sm_AutoType = (LinearAllocatorType)(sm_AutoType + 1);
    ASSERT(sm_AutoType <= kNumAllocatorTypes);
}

LinearAllocatorPageManager LinearAllocator::sm_PageManager[2];

LinearAllocationPage* LinearAllocatorPageManager::RequestPage(Direct3D* context)
{
	std::lock_guard<std::mutex> LockGuard(m_Mutex);

    while (!m_RetiredPages.empty() && context->IsFenceComplete(m_RetiredPages.front().first))
    {
        m_AvailablePages.push(m_RetiredPages.front().second);
        m_RetiredPages.pop();
    }

    LinearAllocationPage* PagePtr = nullptr;

    if (!m_AvailablePages.empty())
    {
        PagePtr = m_AvailablePages.front();
        m_AvailablePages.pop();
    }
    else
    {
        PagePtr = CreateNewPage(context);
        m_PagePool.emplace_back(PagePtr);
    }

    return PagePtr;
}

void LinearAllocatorPageManager::DiscardPages( uint64_t FenceValue, const std::vector<LinearAllocationPage*>& UsedPages )
{
	std::lock_guard<std::mutex> LockGuard(m_Mutex);
    for (auto iter = UsedPages.begin(); iter != UsedPages.end(); ++iter)
        m_RetiredPages.push(std::make_pair(FenceValue, *iter));
}

void LinearAllocatorPageManager::FreeLargePages(Direct3D* context, uint64_t FenceValue, const std::vector<LinearAllocationPage*>& LargePages )
{
	std::lock_guard<std::mutex> LockGuard(m_Mutex);

    while (!m_DeletionQueue.empty() && context->IsFenceComplete(m_DeletionQueue.front().first))
    {
        delete m_DeletionQueue.front().second;
        m_DeletionQueue.pop();
    }

    for (auto iter = LargePages.begin(); iter != LargePages.end(); ++iter)
    {
        (*iter)->Unmap();
        m_DeletionQueue.push(std::make_pair(FenceValue, *iter));
    }
}

LinearAllocationPage* LinearAllocatorPageManager::CreateNewPage(Direct3D* context, size_t PageSize  )
{
    D3D12_HEAP_PROPERTIES HeapProps;
    HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    HeapProps.CreationNodeMask = 1;
    HeapProps.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC ResourceDesc;
    ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    ResourceDesc.Alignment = 0;
    ResourceDesc.Height = 1;
    ResourceDesc.DepthOrArraySize = 1;
    ResourceDesc.MipLevels = 1;
    ResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    ResourceDesc.SampleDesc.Count = 1;
    ResourceDesc.SampleDesc.Quality = 0;
    ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    D3D12_RESOURCE_STATES DefaultUsage;

    if (m_AllocationType == kGpuExclusive)
    {
        HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
        ResourceDesc.Width = PageSize == 0 ? kGpuAllocatorPageSize : PageSize;
        ResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        DefaultUsage = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    }
    else
    {
        HeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
        ResourceDesc.Width = PageSize == 0 ? kCpuAllocatorPageSize : PageSize;
        ResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
        DefaultUsage = D3D12_RESOURCE_STATE_GENERIC_READ;
    }

    ID3D12Resource* pBuffer;
    context->GetD3D12Device()->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE,
        &ResourceDesc, DefaultUsage, nullptr, MY_IID_PPV_ARGS(&pBuffer));

    pBuffer->SetName(L"LinearAllocator Page");

    return new LinearAllocationPage(pBuffer, DefaultUsage);
}

void LinearAllocator::CleanupUsedPages(Direct3D* direct_3d, uint64_t FenceID )
{
    if (m_CurPage == nullptr)
        return;

    m_RetiredPages.push_back(m_CurPage);
    m_CurPage = nullptr;
    m_CurOffset = 0;

    sm_PageManager[m_AllocationType].DiscardPages(FenceID, m_RetiredPages);
    m_RetiredPages.clear();

    sm_PageManager[m_AllocationType].FreeLargePages(direct_3d, FenceID, m_LargePageList);
    m_LargePageList.clear();
}

DynAlloc LinearAllocator::AllocateLargePage(Direct3D* direct_3d, size_t SizeInBytes)
{
    LinearAllocationPage* OneOff = sm_PageManager[m_AllocationType].CreateNewPage(direct_3d, SizeInBytes);
    m_LargePageList.push_back(OneOff);

    DynAlloc ret(*OneOff, 0, SizeInBytes);
    ret.DataPtr = OneOff->m_CpuVirtualAddress;
    ret.GpuAddress = OneOff->m_GpuVirtualAddress;

    return ret;
}

DynAlloc LinearAllocator::Allocate(Direct3D* direct_3d, size_t SizeInBytes, size_t Alignment)
{
    const size_t AlignmentMask = Alignment - 1;

    // Assert that it's a power of two.
    ASSERT((AlignmentMask & Alignment) == 0);

    // Align the allocation
    const size_t AlignedSize = Math::AlignUpWithMask(SizeInBytes, AlignmentMask);

    if (AlignedSize > m_PageSize)
        return AllocateLargePage(direct_3d, AlignedSize);

    m_CurOffset = Math::AlignUp(m_CurOffset, Alignment);

    if (m_CurOffset + AlignedSize > m_PageSize)
    {
        ASSERT(m_CurPage != nullptr);
        m_RetiredPages.push_back(m_CurPage);
        m_CurPage = nullptr;
    }

    if (m_CurPage == nullptr)
    {
        m_CurPage = sm_PageManager[m_AllocationType].RequestPage(direct_3d);
        m_CurOffset = 0;
    }

    DynAlloc ret(*m_CurPage, m_CurOffset, AlignedSize);
    ret.DataPtr = (uint8_t*)m_CurPage->m_CpuVirtualAddress + m_CurOffset;
    ret.GpuAddress = m_CurPage->m_GpuVirtualAddress + m_CurOffset;

    m_CurOffset += AlignedSize;

    return ret;
}
