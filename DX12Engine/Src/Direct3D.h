////////////////////////////////////////////////////////////////////////////////
// Filename: d3dclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _D3DCLASS_H_
#define _D3DCLASS_H_



/////////////
// LINKING //
/////////////
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#pragma comment(lib, "dxguid.lib")


//////////////
// INCLUDES //
//////////////
#include <d3d12.h>

//#define BUILDWITHD3D11ON12
#if defined(BUILDWITHD3D11ON12) 

#include <d3d11on12.h>

#endif

#include <directxmath.h>

#include "d3dx12.h"
#include <dxgi1_4.h>
#include <dxgi1_6.h>

#include <D3Dcompiler.h>
#include <stdexcept>

#include "AnimatedMesh.h"
#include "Direct2D.h"
#include "OldCamera.h"
#include "CPUPerformance.h"
#include "FPSTimer.h"
#include "OldCamera.h"
#include "GameTime.h"
#include "Dynamic Descriptor Heap/RootSignature.h"
#include "Dynamic Descriptor Heap/DynamicDescriptorHeap.h"
#include "Dynamic Descriptor Heap/GpuBuffer.h"
#include "Dynamic Descriptor Heap/LinearAllocator.h"

#if defined(_DEBUG) || defined(DBG)
inline void SetName(ID3D12Object* pObject, LPCWSTR name)
{
    pObject->SetName(name);
}
inline void SetNameIndexed(ID3D12Object* pObject, LPCWSTR name, UINT index)
{
    WCHAR fullName[50];
    if (swprintf_s(fullName, L"%s[%u]", name, index) > 0)
    {
        pObject->SetName(fullName);
    }
}
#else
inline void SetName(ID3D12Object*, LPCWSTR)
{
}
inline void SetNameIndexed(ID3D12Object*, LPCWSTR, UINT)
{
}
#endif

// Naming helper for ComPtr<T>.
// Assigns the name of the variable as the name of the object.
// The indexed variant will include the index in the name of the object.
#define NAME_D3D12_OBJECT(x) SetName((x).Get(), L#x)
#define NAME_D3D12_OBJECT_INDEXED(x, n) SetNameIndexed((x)[n].Get(), L#x, n)

inline std::string HrToString(HRESULT hr)
{
    char s_str[64] = {};
    sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
    return std::string(s_str);
}

class HrException : public std::runtime_error
{
public:
    HrException(HRESULT hr) : std::runtime_error(HrToString(hr)), m_hr(hr) {}
    HRESULT Error() const { return m_hr; }
private:
    const HRESULT m_hr;
};

#define SAFE_RELEASE(p) if (p) (p)->Release()

inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw HrException(hr);
    }
}


struct SceneConstantBuffer
{
    DirectX::XMFLOAT4X4 worldViewProjectionMat;
    DirectX::XMFLOAT4X4 finalBonesMatrices[100];
    float padding[48]; // Padding so the constant buffer is 256-byte aligned.
};
static_assert((sizeof(SceneConstantBuffer) % 256) == 0, "Constant Buffer size must be 256-byte aligned");


class Model;
////////////////////////////////////////////////////////////////////////////////
// Class name: Direct3D
////////////////////////////////////////////////////////////////////////////////
class Direct3D
{
	enum CommandListState
	{
		CLOSED,
		OPEN
	};
public:

    enum RootBindings
    {
        CBVs,
        SRVs,
        NumRootBindings
    };

	Direct3D();
	~Direct3D();

	bool Initialize(AnimationStuff::Animator*, Model*, OldCamera*, int, int, HWND, bool, bool);
	
	bool Render();

	bool PrepareDebuginfo();

	void AddTextString(std::string Text, std::string FontName, float FontSize, float ColorR, float ColorG, float ColorB, DWRITE_FONT_WEIGHT FontWeight, DWRITE_FONT_STYLE FontStyle, DWRITE_FONT_STRETCH FontStretch, float TextBoxStartX, float TextBoxStartY, float Width, float Height);
	void AddTemporaryTextString(std::string Text, std::string FontName, float FontSize, float ColorR, float ColorG, float ColorB, DWRITE_FONT_WEIGHT FontWeight, DWRITE_FONT_STYLE FontStyle, DWRITE_FONT_STRETCH FontStretch, float TextBoxStartX, float TextBoxStartY, float Width, float Height);

	void AddTemporaryLine(float x1, float y1, float x2, float y2, float r, float g, float b);

	void SetWVPCBV(DirectX::XMMATRIX worldMatrix);

    void SetCB(D3D12_CPU_DESCRIPTOR_HANDLE cbHanle);

    void SetSRV(D3D12_CPU_DESCRIPTOR_HANDLE srvHandle);

	void RendererStart();

	void RendererEnd();

	void BeginCommandListRecording();

    void SetDefaultRootSignatureViewportAndScissorRect();

    void SetMeshDataHeaps();

	void StopCommandListRecordingAndExecute();

	void WaitForGpuToFinishWork();

    UINT GetSRVDescriptorIncrementSize();

    UINT GetSRVDescriptorCurrentOffset();

    bool IsFenceComplete(uint64_t FenceValue);

    void SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type, ID3D12DescriptorHeap* HeapPtr);

	void BindDescriptorHeaps(void);

    void InitializeBuffer(GpuBuffer& Dest, const void* Data, size_t NumBytes, size_t DestOffset = 0);
    void InitializeBuffer(GpuBuffer& Dest, const UploadBuffer& Src, size_t SrcOffset, size_t NumBytes = -1, size_t DestOffset = 0);

	void TransitionResource(GpuResource& Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate);

    D3D12_CPU_DESCRIPTOR_HANDLE AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE Type, UINT Count = 1);

    DynAlloc ReserveUploadMemory(size_t SizeInBytes);

	void InitializeTexture(GpuResource& Dest, UINT NumSubresources, D3D12_SUBRESOURCE_DATA SubData[]);

	Microsoft::WRL::ComPtr<ID3D12Device> GetD3D12Device();
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetCommandList();
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetSRVHeapDescriptor();

    void SetDynamicDescriptor(UINT RootIndex, UINT Offset, D3D12_CPU_DESCRIPTOR_HANDLE Handle);
    void SetDynamicDescriptors(UINT RootIndex, UINT Offset, UINT Count, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[]);
    void SetDynamicSampler(UINT RootIndex, UINT Offset, D3D12_CPU_DESCRIPTOR_HANDLE Handle);
    void SetDynamicSamplers(UINT RootIndex, UINT Offset, UINT Count, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[]);

private:
	
	

	
    static const UINT FrameCount = 3;

    static const UINT DynaminInstancingCount = 100;

    struct Vertex
    {
	    DirectX::XMFLOAT3 position;
	    DirectX::XMFLOAT4 color;
    };

	AnimationStuff::Animator* m_Animator;
	OldCamera* m_Camera;
	Model* m_Model;
    // Pipeline objects.
    CD3DX12_VIEWPORT m_viewport;
    CD3DX12_RECT m_scissorRect;
	Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
	Microsoft::WRL::ComPtr<ID3D12Device> m_d3d12Device;
#if defined(BUILDWITHD3D11ON12) 
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_d3d11DeviceContext;
    Microsoft::WRL::ComPtr<ID3D11On12Device> m_d3d11On12Device;
    Microsoft::WRL::ComPtr<IDWriteFactory> m_dWriteFactory;
    Microsoft::WRL::ComPtr<ID2D1Factory3> m_d2dFactory;
    Microsoft::WRL::ComPtr<ID2D1Device2> m_d2dDevice;
    Microsoft::WRL::ComPtr<ID2D1DeviceContext2> m_d2dDeviceContext;
#endif
    Microsoft::WRL::ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
    Microsoft::WRL::ComPtr<ID3D11Resource> m_wrappedBackBuffers[FrameCount];
    Microsoft::WRL::ComPtr<ID2D1Bitmap1> m_d2dRenderTargets[FrameCount];
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocators[FrameCount];
    Microsoft::WRL::ComPtr<ID3D12Resource> m_DepthStencilBuffer; // This is the memory for our depth buffer. it will also be used for a stencil buffer in a later tutorial
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DepthStencilDescriptorHeap; // This is a heap for our depth/stencil buffer descriptor
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_CbvHeap[FrameCount];
	ID3D12Resource* constantBufferUploadHeap[FrameCount];
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
    ID3D12DescriptorHeap* m_CurrentDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

	CommandListState m_CommandListState;

	SceneConstantBuffer m_constantBuffersData[DynaminInstancingCount];
	UINT8* cbGPUAddress[FrameCount];

    // App resources.
    UINT m_rtvDescriptorSize;
    UINT m_CbvDescriptorSize;
    UINT m_CbvDescriptorCurrentOffset;

    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_textBrush;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_textFormat;


    // Synchronization objects.
    UINT m_frameIndex;
    HANDLE m_fenceEvent;
    Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValues[FrameCount];

	std::vector<UI_Text> UI_TextStrings;
	std::vector<UI_Text> UI_TemporaryTextStrings;
	std::vector<UI_Line_Element> UI_TemporaryLines;
    UINT64 m_LastCompletedFenceValue = 0;

    DescriptorAllocator m_DescriptorAllocator[4];

    RootSignature m_RootSig;

	double m_LastFrameDuration;

    LinearAllocator m_CpuLinearAllocator;
    LinearAllocator m_GpuLinearAllocator;

    DynamicDescriptorHeap m_DynamicViewDescriptorHeap;		// HEAP_TYPE_CBV_SRV_UAV
    DynamicDescriptorHeap m_DynamicSamplerDescriptorHeap;	// HEAP_TYPE_SAMPLER

	CPUPerformance m_Cpu;

    void LoadPipeline(int width, int height, HWND hwnd);
    void LoadAssets(int width, int height);
    
	std::wstring StringToWString(const std::string& s);
    void WaitForGpu();
    void MoveToNextFrame();
    void RenderUI();
    void GetHardwareAdapter(
        IDXGIFactory1* pFactory,
        IDXGIAdapter1** ppAdapter,
        bool requestHighPerformanceAdapter = false);
};

#endif