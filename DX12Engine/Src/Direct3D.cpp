////////////////////////////////////////////////////////////////////////////////
// Filename: d3dclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "Direct3D.h"
#include "..\Model.h"
#include "Dynamic Descriptor Heap/UploadBuffer.h"


Direct3D::Direct3D(): m_Camera(nullptr), m_Model(nullptr), m_viewport(), m_scissorRect(), constantBufferUploadHeap{},
                      m_constantBuffersData(), cbGPUAddress{},
                      m_rtvDescriptorSize(0),
                      m_frameIndex(0),
                      m_fenceEvent(nullptr),
                      m_fenceValues{},
					  m_DescriptorAllocator
						{
							D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
							D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
							D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
							D3D12_DESCRIPTOR_HEAP_TYPE_DSV
						},
                      m_LastFrameDuration(0),
					m_DynamicViewDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV),
					m_DynamicSamplerDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER),
					m_CpuLinearAllocator(kCpuWritable),
					m_GpuLinearAllocator(kGpuExclusive),
					m_CbvDescriptorCurrentOffset(0)
{
}

Direct3D::~Direct3D()
{
	m_Cpu.Shutdown();
}


bool Direct3D::Initialize(AnimationStuff::Animator* animator, Model* model, OldCamera* camera, int screenWidth, int screenHeight, HWND hwnd, bool vsync, bool fullscreen)
{
	// Fill out the Viewport
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;
	m_viewport.Width = screenWidth;
	m_viewport.Height = screenHeight;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;

	// Fill out a scissor rect
	m_scissorRect.left = 0;
	m_scissorRect.top = 0;
	m_scissorRect.right = screenWidth;
	m_scissorRect.bottom = screenHeight;

	m_Model = model;
	m_Camera = camera;
	m_Cpu.Initialize();
	LoadPipeline(screenWidth, screenHeight, hwnd);
	LoadAssets(screenWidth, screenHeight);
	m_DynamicViewDescriptorHeap.InitDynamicDescriptorHeap(this);
	m_DynamicSamplerDescriptorHeap.InitDynamicDescriptorHeap(this);
	m_Animator = animator;

	return true;
}


// Load the rendering pipeline dependencies.
void Direct3D::LoadPipeline(int width, int height, HWND hwnd)
{
	UINT dxgiFactoryFlags = 0;
	UINT d3d11DeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
	D2D1_FACTORY_OPTIONS d2dFactoryOptions = {};

#if defined(_DEBUG)
	// Enable the debug layer (requires the Graphics Tools "optional feature").
	// NOTE: Enabling the debug layer after device creation will invalidate the active device.
	{
		Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();

			// Enable additional debug layers.
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
			d3d11DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
			d2dFactoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
		}
	}
#endif

	Microsoft::WRL::ComPtr<IDXGIFactory4> factory;
	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

	Microsoft::WRL::ComPtr<IDXGIAdapter1> hardwareAdapter;
	GetHardwareAdapter(factory.Get(), &hardwareAdapter);

	ThrowIfFailed(D3D12CreateDevice(
		hardwareAdapter.Get(),
		D3D_FEATURE_LEVEL_12_1,
		IID_PPV_ARGS(&m_d3d12Device)
	));

#if defined(_DEBUG)
	// Filter a debug error coming from the 11on12 layer.
	Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue;
	if (SUCCEEDED(m_d3d12Device->QueryInterface(IID_PPV_ARGS(&infoQueue))))
	{
		// Suppress whole categories of messages.
		//D3D12_MESSAGE_CATEGORY categories[] = {};

		// Suppress messages based on their severity level.
		D3D12_MESSAGE_SEVERITY severities[] =
		{
			D3D12_MESSAGE_SEVERITY_INFO,
		};

		// Suppress individual messages by their ID.
		D3D12_MESSAGE_ID denyIds[] =
		{
			// This occurs when there are uninitialized descriptors in a descriptor table, even when a
			// shader does not access the missing descriptors.
			D3D12_MESSAGE_ID_INVALID_DESCRIPTOR_HANDLE,
		};

		D3D12_INFO_QUEUE_FILTER filter = {};
		//filter.DenyList.NumCategories = _countof(categories);
		//filter.DenyList.pCategoryList = categories;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;

		ThrowIfFailed(infoQueue->PushStorageFilter(&filter));
	}
#endif

	// Describe and create the command queue.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(m_d3d12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));
	NAME_D3D12_OBJECT(m_commandQueue);

	// Describe the swap chain.
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = FrameCount;
	swapChainDesc.Width = width;
	swapChainDesc.Height = height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
	ThrowIfFailed(factory->CreateSwapChainForHwnd(
		m_commandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
		hwnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
	));

	// This sample does not support fullscreen transitions.
	ThrowIfFailed(factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));

	ThrowIfFailed(swapChain.As(&m_swapChain));
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
#if defined(BUILDWITHD3D11ON12) 
	// Create an 11 device wrapped around the 12 device and share
	// 12's command queue.
	Microsoft::WRL::ComPtr<ID3D11Device> d3d11Device;
	ThrowIfFailed(D3D11On12CreateDevice(
		m_d3d12Device.Get(),
		d3d11DeviceFlags,
		nullptr,
		0,
		reinterpret_cast<IUnknown**>(m_commandQueue.GetAddressOf()),
		1,
		0,
		&d3d11Device,
		&m_d3d11DeviceContext,
		nullptr
	));

	// Query the 11On12 device from the 11 device.
	ThrowIfFailed(d3d11Device.As(&m_d3d11On12Device));

	// Create D2D/DWrite components.
	{
		D2D1_DEVICE_CONTEXT_OPTIONS deviceOptions = D2D1_DEVICE_CONTEXT_OPTIONS_NONE;
		ThrowIfFailed(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory3), &d2dFactoryOptions, &m_d2dFactory));
		Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
		ThrowIfFailed(m_d3d11On12Device.As(&dxgiDevice));
		ThrowIfFailed(m_d2dFactory->CreateDevice(dxgiDevice.Get(), &m_d2dDevice));
		ThrowIfFailed(m_d2dDevice->CreateDeviceContext(deviceOptions, &m_d2dDeviceContext));
		ThrowIfFailed(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &m_dWriteFactory));
	}

	// Query the desktop's dpi settings, which will be used to create
	// D2D's render targets.
	float dpiX;
	float dpiY;
#pragma warning(push)
#pragma warning(disable : 4996) // GetDesktopDpi is deprecated.
	m_d2dFactory->GetDesktopDpi(&dpiX, &dpiY);
#pragma warning(pop)
D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
		D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
		dpiX,
		dpiY
	);
#endif
	{
		// Describe and create a render target view (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = FrameCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(m_d3d12Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

		// Describe and create a shader resource view (SRV) heap for the texture.
		for (int i = 0; i < FrameCount; ++i)
		{
			D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
			heapDesc.NumDescriptors = DynaminInstancingCount;
			heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			ThrowIfFailed(m_d3d12Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_CbvHeap[i])));
		}

		m_rtvDescriptorSize = m_d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		m_CbvDescriptorSize = m_d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	{
		// Create the depth/stencil buffer

  // create a depth stencil descriptor heap so we can get a pointer to the depth stencil buffer
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(m_d3d12Device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_DepthStencilDescriptorHeap)));
		m_DepthStencilDescriptorHeap->SetName(L"Depth/Stencil Resource Heap");

		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
		depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

		D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
		depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
		depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
		depthOptimizedClearValue.DepthStencil.Stencil = 0;

		CD3DX12_HEAP_PROPERTIES dsHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_RESOURCE_DESC dsResourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

		m_d3d12Device->CreateCommittedResource(
			&dsHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&dsResourceDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&depthOptimizedClearValue,
			IID_PPV_ARGS(&m_DepthStencilBuffer)
		);

		m_d3d12Device->CreateDepthStencilView(m_DepthStencilBuffer.Get(), &depthStencilDesc, m_DepthStencilDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	}

	// Create frame resources.
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

		// Create a RTV, D2D render target, and a command allocator for each frame.
		for (UINT n = 0; n < FrameCount; n++)
		{
			ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
			m_d3d12Device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);

			NAME_D3D12_OBJECT_INDEXED(m_renderTargets, n);

			rtvHandle.Offset(1, m_rtvDescriptorSize);

			ThrowIfFailed(m_d3d12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[n])));

#if defined(BUILDWITHD3D11ON12) 
			// Create a wrapped 11On12 resource of this back buffer. Since we are 
			// rendering all D3D12 content first and then all D2D content, we specify 
			// the In resource state as RENDER_TARGET - because D3D12 will have last 
			// used it in this state - and the Out resource state as PRESENT. When 
			// ReleaseWrappedResources() is called on the 11On12 device, the resource 
			// will be transitioned to the PRESENT state.
			D3D11_RESOURCE_FLAGS d3d11Flags = {D3D11_BIND_RENDER_TARGET};
			ThrowIfFailed(m_d3d11On12Device->CreateWrappedResource(
				m_renderTargets[n].Get(),
				&d3d11Flags,
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_PRESENT,
				IID_PPV_ARGS(&m_wrappedBackBuffers[n])
			));

			// Create a render target for D2D to draw directly to this back buffer.
			Microsoft::WRL::ComPtr<IDXGISurface> surface;
			ThrowIfFailed(m_wrappedBackBuffers[n].As(&surface));
			ThrowIfFailed(m_d2dDeviceContext->CreateBitmapFromDxgiSurface(
				surface.Get(),
				&bitmapProperties,
				&m_d2dRenderTargets[n]
			));
#endif
			
		}
	}


}

// Load the sample assets.
void Direct3D::LoadAssets(int width, int height)
{
	// Create an empty root signature.
	{
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

		// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

		if (FAILED(m_d3d12Device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}

		FLOAT test[4];
		test[0] = 0.0f;
		test[1] = 0.0f;
		test[2] = 0.0f;
		test[3] = 0.0f;
		D3D12_SAMPLER_DESC SamplerLinearWrapDesc;
		SamplerLinearWrapDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		SamplerLinearWrapDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		SamplerLinearWrapDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		SamplerLinearWrapDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		SamplerLinearWrapDesc.MipLODBias = 0;
		SamplerLinearWrapDesc.MaxAnisotropy = 0;
		SamplerLinearWrapDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		SamplerLinearWrapDesc.MinLOD = 0.0f;
		SamplerLinearWrapDesc.MaxLOD = D3D12_FLOAT32_MAX;


		m_RootSig.Reset(2, 1);
		m_RootSig.InitStaticSampler(0, SamplerLinearWrapDesc, D3D12_SHADER_VISIBILITY_PIXEL);
		m_RootSig[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_VERTEX, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
		m_RootSig[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
		m_RootSig.Finalize(this, L"RootSig", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	}

	// Create the pipeline state, which includes compiling and loading shaders.
	{
		Microsoft::WRL::ComPtr<ID3DBlob> vertexShader;
		Microsoft::WRL::ComPtr<ID3DBlob> pixelShader;
		Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

#if defined(_DEBUG)
		// Enable better shader debugging with the graphics debugging tools.
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0;
#endif


		HRESULT hr = D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, &errorBlob);
		if (FAILED(hr))
		{
			if (errorBlob)
			{
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());
				return;
			}
		}
		hr = D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, &errorBlob);
		if (FAILED(hr))
		{
			if (errorBlob)
			{
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());
				return;
			}
		}
		// Define the vertex input layout.
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "BONEIDS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "BONEWEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		// Describe and create the graphics pipeline state object (PSO).
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
		psoDesc.pRootSignature = m_RootSig.GetSignature();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT); // a default depth stencil state
		psoDesc.DepthStencilState.DepthEnable = TRUE;
		psoDesc.DepthStencilState.StencilEnable = FALSE;
		psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count = 1;

		ThrowIfFailed(m_d3d12Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
		NAME_D3D12_OBJECT(m_pipelineState);
	}

	ThrowIfFailed(m_d3d12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[m_frameIndex].Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList)));
	NAME_D3D12_OBJECT(m_commandList);
#if defined(BUILDWITHD3D11ON12) 
	// Create D2D/DWrite objects for rendering text.
	{
		ThrowIfFailed(m_d2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_textBrush));
		ThrowIfFailed(m_dWriteFactory->CreateTextFormat(
			L"Verdana",
			NULL,
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			50,
			L"en-us",
			&m_textFormat
		));
		ThrowIfFailed(m_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
		ThrowIfFailed(m_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER));
	}
#endif

	CD3DX12_RESOURCE_DESC constantBufferResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(DynaminInstancingCount * sizeof(SceneConstantBuffer));
	CD3DX12_HEAP_PROPERTIES heap_properties_Upload = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

	// Close the command list and execute it to begin the vertex buffer copy into
	// the default heap.
	ThrowIfFailed(m_commandList->Close());
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	for (int i = 0; i < FrameCount; ++i)
	{
		ThrowIfFailed(m_d3d12Device->CreateCommittedResource(
			&heap_properties_Upload, // this heap will be used to upload the constant buffer data
			D3D12_HEAP_FLAG_NONE, // no flags
			&constantBufferResourceDesc, // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
			D3D12_RESOURCE_STATE_GENERIC_READ, // will be data that is read from so we keep it in the generic read state
			nullptr, // we do not have use an optimized clear value for constant buffers
			IID_PPV_ARGS(&constantBufferUploadHeap[i])));
		constantBufferUploadHeap[i]->SetName(L"Constant Buffer Upload Resource Heap");

		ZeroMemory(&m_constantBuffersData, sizeof(m_constantBuffersData));


		CD3DX12_RANGE readRange(0, 0);    // We do not intend to read from this resource on the CPU. (End is less than or equal to begin)
		constantBufferUploadHeap[i]->Map(0, &readRange, reinterpret_cast<void**>(&cbGPUAddress[i]));
		memcpy(cbGPUAddress[i], &m_constantBuffersData, sizeof(m_constantBuffersData));

		CD3DX12_CPU_DESCRIPTOR_HANDLE cbvHandle(m_CbvHeap[i]->GetCPUDescriptorHandleForHeapStart(), 0, m_CbvDescriptorSize);
		UINT64 cbOffset = 0;

		for (UINT k = 0; k < DynaminInstancingCount; k++)
		{
			// Describe and create a constant buffer view (CBV).
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
			cbvDesc.BufferLocation = constantBufferUploadHeap[i]->GetGPUVirtualAddress() + cbOffset;
			cbvDesc.SizeInBytes = sizeof(SceneConstantBuffer);
			cbOffset += cbvDesc.SizeInBytes;
			m_d3d12Device->CreateConstantBufferView(&cbvDesc, cbvHandle);
			cbvHandle.Offset(m_CbvDescriptorSize);
		}

		
	}
	

	// Create synchronization objects and wait until assets have been uploaded to the GPU.
	{
		ThrowIfFailed(m_d3d12Device->CreateFence(m_fenceValues[m_frameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.GetAddressOf())));
		m_fenceValues[m_frameIndex]++;

		// Create an event handle to use for frame synchronization.
		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (m_fenceEvent == nullptr)
		{
			ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
		}

		// Wait for the command list to execute; we are reusing the same command 
		// list in our main loop but for now, we just want to wait for setup to 
		// complete before continuing.
		WaitForGpu();
	}
}

void Direct3D::GetHardwareAdapter(
	IDXGIFactory1* pFactory,
	IDXGIAdapter1** ppAdapter,
	bool requestHighPerformanceAdapter)
{
	*ppAdapter = nullptr;

	Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;

	Microsoft::WRL::ComPtr<IDXGIFactory6> factory6;
	if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
	{
		for (
			UINT adapterIndex = 0;
			SUCCEEDED(factory6->EnumAdapterByGpuPreference(
				adapterIndex,
				requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
				IID_PPV_ARGS(&adapter)));
			++adapterIndex)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				// Don't select the Basic Update Driver adapter.
				// If you want a software adapter, pass in "/warp" on the command line.
				continue;
			}

			// Check to see whether the adapter supports Direct3D 12, but don't create the
			// actual device yet.
			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
			{
				break;
			}
		}
	}

	if (adapter.Get() == nullptr)
	{
		for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				// Don't select the Basic Update Driver adapter.
				// If you want a software adapter, pass in "/warp" on the command line.
				continue;
			}

			// Check to see whether the adapter supports Direct3D 12, but don't create the
			// actual device yet.
			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
			{
				break;
			}
		}
	}

	*ppAdapter = adapter.Detach();
}


void Direct3D::RendererStart()
{
	
	// Command list allocators can only be reset when the associated 
	// command lists have finished execution on the GPU; apps should use 
	// fences to determine GPU execution progress.
	ThrowIfFailed(m_commandAllocators[m_frameIndex]->Reset());

	// However, when ExecuteCommandList() is called on a particular command 
	// list, that command list can then be reset at any time and must be before 
	// re-recording.
	ThrowIfFailed(m_commandList->Reset(m_commandAllocators[m_frameIndex].Get(), m_pipelineState.Get()));

	// Set necessary state.
	m_commandList->SetGraphicsRootSignature(m_RootSig.GetSignature());
	m_DynamicViewDescriptorHeap.ParseGraphicsRootSignature(m_RootSig);
	m_DynamicSamplerDescriptorHeap.ParseGraphicsRootSignature(m_RootSig);

	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	CD3DX12_RESOURCE_BARRIER resource_barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	// Indicate that the back buffer will be used as a render target.
	m_commandList->ResourceBarrier(1, &resource_barrier);

	// get a handle to the depth/stencil buffer
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_DepthStencilDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	// Record commands.
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	// clear the depth/stencil buffer
	m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	m_CommandListState = CommandListState::OPEN;
}

void Direct3D::RendererEnd()
{
#if !defined(BUILDWITHD3D11ON12)
	CD3DX12_RESOURCE_BARRIER resource_barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_commandList->ResourceBarrier(1, &resource_barrier);
#endif
	// Note: do not transition the render target to present here.
	// the transition will occur when the wrapped 11On12 render
	// target resource is released.

	ThrowIfFailed(m_commandList->Close());
	m_CommandListState = CommandListState::CLOSED;
}

void Direct3D::BeginCommandListRecording()
{
	if(m_CommandListState == CommandListState::CLOSED)
	{
		// Command list allocators can only be reset when the associated 
		// command lists have finished execution on the GPU; apps should use 
		// fences to determine GPU execution progress.
		ThrowIfFailed(m_commandAllocators[m_frameIndex]->Reset());

		// However, when ExecuteCommandList() is called on a particular command 
		// list, that command list can then be reset at any time and must be before 
		// re-recording.
		ThrowIfFailed(m_commandList->Reset(m_commandAllocators[m_frameIndex].Get(), m_pipelineState.Get()));

		m_CommandListState = CommandListState::OPEN;
	}

}

void Direct3D::SetDefaultRootSignatureViewportAndScissorRect()
{
	// Set necessary state.
	m_commandList->SetGraphicsRootSignature(m_RootSig.GetSignature());
	m_DynamicViewDescriptorHeap.ParseGraphicsRootSignature(m_RootSig);
	m_DynamicSamplerDescriptorHeap.ParseGraphicsRootSignature(m_RootSig);

	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_DepthStencilDescriptorHeap->GetCPUDescriptorHandleForHeapStart());


	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
}

void Direct3D::StopCommandListRecordingAndExecute()
{
	if(m_CommandListState == CommandListState::OPEN)
	{
		// Execute the initialization commands.
		ThrowIfFailed(m_commandList->Close());
		ID3D12CommandList* cmdsLists[] = { m_commandList.Get() };
		m_commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
		m_CommandListState = CommandListState::CLOSED;
		
	}
}

void Direct3D::WaitForGpuToFinishWork()
{
	WaitForGpu();
}

UINT Direct3D::GetSRVDescriptorIncrementSize()
{
	return m_CbvDescriptorSize;
}


// Update text over D3D12 using D2D via the 11On12 device.
void Direct3D::RenderUI()
{
#if defined(BUILDWITHD3D11ON12) 
	D2D1_SIZE_F rtSize = m_d2dRenderTargets[m_frameIndex]->GetSize();
	D2D1_RECT_F textRect = D2D1::RectF(0, 0, rtSize.width, rtSize.height);
	static const WCHAR text[] = L"11On12";

	// Acquire our wrapped render target resource for the current back buffer.
	m_d3d11On12Device->AcquireWrappedResources(m_wrappedBackBuffers[m_frameIndex].GetAddressOf(), 1);

	// Update text directly to the back buffer.
	m_d2dDeviceContext->SetTarget(m_d2dRenderTargets[m_frameIndex].Get());
	m_d2dDeviceContext->BeginDraw();
	m_d2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());

	for (int i = 0; i < UI_TextStrings.size(); i++)
	{
		std::wstring FontName = StringToWString(UI_TextStrings[i].FontName);
		std::wstring Text = StringToWString(UI_TextStrings[i].Text);

		IDWriteTextFormat* TextFormat;
		ThrowIfFailed(m_dWriteFactory->CreateTextFormat(
			FontName.c_str(),
			NULL,
			UI_TextStrings[i].FontWeight,
			UI_TextStrings[i].FontStyle,
			UI_TextStrings[i].FontStretch,
			UI_TextStrings[i].FontSize,
			L"",
			&TextFormat));

		// create a brush
		ID2D1SolidColorBrush* ColoredBrush;
		ThrowIfFailed(m_d2dDeviceContext->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF(UI_TextStrings[i].ColorR, UI_TextStrings[i].ColorG, UI_TextStrings[i].ColorB, 1.0f)),
			&ColoredBrush
		));

		//Update text strings to display
		m_d2dDeviceContext->DrawText(Text.c_str(), wcslen(Text.c_str()), TextFormat, UI_TextStrings[i].Rect.UI_LayoutRect, ColoredBrush);

		TextFormat->Release();
		TextFormat = 0;
		ColoredBrush->Release();
		ColoredBrush = 0;
	}

	for (int i = 0; i < UI_TemporaryTextStrings.size(); i++)
	{
		std::wstring FontName = StringToWString(UI_TemporaryTextStrings[i].FontName);
		std::wstring Text = StringToWString(UI_TemporaryTextStrings[i].Text);

		IDWriteTextFormat* TextFormat;
		ThrowIfFailed(m_dWriteFactory->CreateTextFormat(
			FontName.c_str(),
			NULL,
			UI_TemporaryTextStrings[i].FontWeight,
			UI_TemporaryTextStrings[i].FontStyle,
			UI_TemporaryTextStrings[i].FontStretch,
			UI_TemporaryTextStrings[i].FontSize,
			L"",
			&TextFormat));

		ID2D1SolidColorBrush* ColoredBrush;
		ThrowIfFailed(m_d2dDeviceContext->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF(UI_TemporaryTextStrings[i].ColorR, UI_TemporaryTextStrings[i].ColorG, UI_TemporaryTextStrings[i].ColorB, 1.0f)),
			&ColoredBrush
		));

		//Update text strings to display
		m_d2dDeviceContext->DrawText(Text.c_str(), wcslen(Text.c_str()), TextFormat, UI_TemporaryTextStrings[i].Rect.UI_LayoutRect, ColoredBrush);

		TextFormat->Release();
		TextFormat = 0;
		ColoredBrush->Release();
		ColoredBrush = 0;
	}

	for (int i = 0; i < UI_TemporaryLines.size(); i++)
	{
		// Create a brush with the given color
		ID2D1SolidColorBrush* ColoredBrush;
		ThrowIfFailed(m_d2dDeviceContext->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF(UI_TemporaryLines[i].ColorR, UI_TemporaryLines[i].ColorG, UI_TemporaryLines[i].ColorB, 1.0f)),
			&ColoredBrush
		));

		// Update lines.
		m_d2dDeviceContext->DrawLine(UI_TemporaryLines[i].point0, UI_TemporaryLines[i].point1, ColoredBrush);

		ColoredBrush->Release();
		ColoredBrush = 0;
	}

	m_d2dDeviceContext->EndDraw();
	UI_TemporaryLines.clear();
	UI_TemporaryTextStrings.clear();

	// Release our wrapped render target resource. Releasing 
	// transitions the back buffer resource to the state specified
	// as the OutState when the wrapped resource was created.
	m_d3d11On12Device->ReleaseWrappedResources(m_wrappedBackBuffers[m_frameIndex].GetAddressOf(), 1);

	// Flush to submit the 11 command list to the shared command queue.
	m_d3d11DeviceContext->Flush();

#endif

}

// Wait for pending GPU work to complete.
void Direct3D::WaitForGpu()
{
	// Schedule a Signal command in the queue.
	ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_fenceValues[m_frameIndex]));

	// Wait until the fence has been processed.
	ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent));
	WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

	// Increment the fence value for the current frame.
	m_fenceValues[m_frameIndex]++;
}

// Prepare to render the next frame.
void Direct3D::MoveToNextFrame()
{
	// Schedule a Signal command in the queue.
	const UINT64 currentFenceValue = m_fenceValues[m_frameIndex];
	ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), currentFenceValue));

	// Update the frame index.
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
	m_CbvDescriptorCurrentOffset = 0;
	

	// If the next frame is not ready to be rendered yet, wait until it is ready.
	if (m_fence->GetCompletedValue() < m_fenceValues[m_frameIndex])
	{
		ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent));
		WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
	}

	// Set the fence value for the next frame.
	m_fenceValues[m_frameIndex] = currentFenceValue + 1;

	m_CpuLinearAllocator.CleanupUsedPages(this, m_fenceValues[m_frameIndex]);
	m_GpuLinearAllocator.CleanupUsedPages(this, m_fenceValues[m_frameIndex]);
	m_DynamicViewDescriptorHeap.CleanupUsedHeaps(m_fenceValues[m_frameIndex]);
	m_DynamicSamplerDescriptorHeap.CleanupUsedHeaps(m_fenceValues[m_frameIndex]);
}

bool Direct3D::Render()
{
	m_Cpu.Frame();

	PrepareDebuginfo();

	// Record all the commands we need to render the scene into the command list.
	RendererEnd();

	
	// Execute the command list.
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	
	RenderUI();


	ThrowIfFailed(m_swapChain->Present(0, 0));

	MoveToNextFrame();

	return true;
}

void Direct3D::AddTextString(std::string Text, std::string FontName, float FontSize, float ColorR, float ColorG, float ColorB, DWRITE_FONT_WEIGHT FontWeight, DWRITE_FONT_STYLE FontStyle, DWRITE_FONT_STRETCH FontStretch, float TextBoxStartX, float TextBoxStartY, float Width, float Height)
{
	UI_TextStrings.push_back(UI_Text(Text, FontName, FontSize, ColorR, ColorG, ColorB, FontWeight, FontStyle, FontStretch, TextBoxStartX, TextBoxStartY, Width, Height));
}

void Direct3D::AddTemporaryTextString(std::string Text, std::string FontName, float FontSize, float ColorR, float ColorG, float ColorB, DWRITE_FONT_WEIGHT FontWeight, DWRITE_FONT_STYLE FontStyle, DWRITE_FONT_STRETCH FontStretch, float TextBoxStartX, float TextBoxStartY, float Width, float Height)
{
	UI_TemporaryTextStrings.push_back(UI_Text(Text, FontName, FontSize, ColorR, ColorG, ColorB, FontWeight, FontStyle, FontStretch, TextBoxStartX, TextBoxStartY, Width, Height));
}

void Direct3D::AddTemporaryLine(float x1, float y1, float x2, float y2, float r, float g, float b)
{
	UI_TemporaryLines.push_back(UI_Line_Element(x1, y1, x2, y2, r, g, b));
}

void Direct3D::SetWVPCBV(DirectX::XMMATRIX worldMatrix)
{
	/*DirectX::XMVECTOR scale;
	DirectX::XMVECTOR rotation;
	DirectX::XMVECTOR position;
	DirectX::XMMatrixDecompose(&scale, &rotation, &position, worldMatrix);*/
	DirectX::XMMATRIX View = m_Camera->GetViewMatrix();
	DirectX::XMMATRIX Projection = m_Camera->GetProjectionMatrix();

	DirectX::XMMATRIX WorldViewProjection = worldMatrix * View * Projection;
	if(m_CbvDescriptorCurrentOffset < DynaminInstancingCount)
	{
		DirectX::XMStoreFloat4x4(&m_constantBuffersData[m_CbvDescriptorCurrentOffset].worldViewProjectionMat, DirectX::XMMatrixTranspose(WorldViewProjection));
		std::vector<DirectX::XMFLOAT4X4>boneMatrices = m_Animator->GetFinalBoneMatrices();
		int boneMatrixIndex = 0;
		for (const DirectX::XMFLOAT4X4& bone_matrix : boneMatrices)
		{
			DirectX::XMStoreFloat4x4(&m_constantBuffersData[m_CbvDescriptorCurrentOffset].finalBonesMatrices[boneMatrixIndex], DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&bone_matrix)));
			boneMatrixIndex++;
		}

		CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.

		memcpy(cbGPUAddress[m_frameIndex] + (m_CbvDescriptorCurrentOffset * sizeof(SceneConstantBuffer)), &m_constantBuffersData[m_CbvDescriptorCurrentOffset], sizeof(m_constantBuffersData[m_CbvDescriptorCurrentOffset]));
		CD3DX12_CPU_DESCRIPTOR_HANDLE cbvHandle(m_CbvHeap[m_frameIndex]->GetCPUDescriptorHandleForHeapStart(), m_CbvDescriptorCurrentOffset, m_CbvDescriptorSize);
		SetDynamicDescriptor(0, 0, cbvHandle);
		m_CbvDescriptorCurrentOffset++;
	}
	else
	{
		StopCommandListRecordingAndExecute();
		WaitForGpuToFinishWork();
		BeginCommandListRecording();
		SetDefaultRootSignatureViewportAndScissorRect();
		m_CbvDescriptorCurrentOffset = 0;
		SetWVPCBV(worldMatrix);
	}

}

void Direct3D::SetCB(D3D12_CPU_DESCRIPTOR_HANDLE cbHanle)
{
	SetDynamicDescriptor(0, 0, cbHanle);
}

void Direct3D::SetSRV(D3D12_CPU_DESCRIPTOR_HANDLE srvHandle)
{
	SetDynamicDescriptor(1, 0, srvHandle);
}

void Direct3D::SetDynamicDescriptor(UINT RootIndex, UINT Offset, D3D12_CPU_DESCRIPTOR_HANDLE Handle)
{
	SetDynamicDescriptors(RootIndex, Offset, 1, &Handle);
}

void Direct3D::SetDynamicDescriptors(UINT RootIndex, UINT Offset, UINT Count, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[])
{
	m_DynamicViewDescriptorHeap.SetGraphicsDescriptorHandles(RootIndex, Offset, Count, Handles);
}

void Direct3D::SetDynamicSampler(UINT RootIndex, UINT Offset, D3D12_CPU_DESCRIPTOR_HANDLE Handle)
{
	SetDynamicSamplers(RootIndex, Offset, 1, &Handle);
}

void Direct3D::SetDynamicSamplers(UINT RootIndex, UINT Offset, UINT Count, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[])
{
	m_DynamicSamplerDescriptorHeap.SetGraphicsDescriptorHandles(RootIndex, Offset, Count, Handles);
}

UINT Direct3D::GetSRVDescriptorCurrentOffset()
{
	return m_CbvDescriptorCurrentOffset;
}

bool Direct3D::IsFenceComplete(uint64_t FenceValue)
{
	UINT64 fenceValue = m_fence->GetCompletedValue();
	if(fenceValue >= FenceValue)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void Direct3D::SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type, ID3D12DescriptorHeap* HeapPtr)
{
	if (m_CurrentDescriptorHeaps[Type] != HeapPtr)
	{
		m_CurrentDescriptorHeaps[Type] = HeapPtr;
	}
	BindDescriptorHeaps();
}

void Direct3D::InitializeBuffer(GpuBuffer& Dest, const UploadBuffer& Src, size_t SrcOffset, size_t NumBytes,
	size_t DestOffset)
{
	BeginCommandListRecording();

	size_t MaxBytes = std::min<size_t>(Dest.GetBufferSize() - DestOffset, Src.GetBufferSize() - SrcOffset);
	NumBytes = std::min<size_t>(MaxBytes, NumBytes);

	// copy data to the intermediate upload heap and then schedule a copy from the upload heap to the default texture
	TransitionResource(Dest, D3D12_RESOURCE_STATE_COPY_DEST, true);
	m_commandList->CopyBufferRegion(Dest.GetResource(), DestOffset, (ID3D12Resource*)Src.GetResource(), SrcOffset, NumBytes);
	TransitionResource(Dest, D3D12_RESOURCE_STATE_GENERIC_READ, true);

	// Execute the command list and wait for it to finish so we can release the upload buffer
	StopCommandListRecordingAndExecute();
	WaitForGpuToFinishWork();
}

void Direct3D::TransitionResource(GpuResource& Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate)
{
	D3D12_RESOURCE_STATES OldState = Resource.m_UsageState;

	if (OldState != NewState)
	{
		D3D12_RESOURCE_BARRIER BarrierDesc;

		BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		BarrierDesc.Transition.pResource = Resource.GetResource();
		BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		BarrierDesc.Transition.StateBefore = OldState;
		BarrierDesc.Transition.StateAfter = NewState;

		// Check to see if we already started the transition
		if (NewState == Resource.m_TransitioningState)
		{
			BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
			Resource.m_TransitioningState = (D3D12_RESOURCE_STATES)-1;
		}
		else
			BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

		Resource.m_UsageState = NewState;
	}
}

D3D12_CPU_DESCRIPTOR_HANDLE Direct3D::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE Type, UINT Count)
{
	return m_DescriptorAllocator[Type].Allocate(GetD3D12Device().Get(), Count);
}

DynAlloc Direct3D::ReserveUploadMemory(size_t SizeInBytes)
{
	return m_CpuLinearAllocator.Allocate(this, SizeInBytes);
}

void Direct3D::InitializeTexture(GpuResource& Dest, UINT NumSubresources, D3D12_SUBRESOURCE_DATA SubData[])
{
	UINT64 uploadBufferSize = GetRequiredIntermediateSize(Dest.GetResource(), 0, NumSubresources);
	BeginCommandListRecording();

	// copy data to the intermediate upload heap and then schedule a copy from the upload heap to the default texture
	DynAlloc mem = ReserveUploadMemory(uploadBufferSize);
	UpdateSubresources(m_commandList.Get(), Dest.GetResource(), mem.Buffer.GetResource(), 0, 0, NumSubresources, SubData);
	TransitionResource(Dest, D3D12_RESOURCE_STATE_GENERIC_READ, false);

	// Execute the command list and wait for it to finish so we can release the upload buffer
	StopCommandListRecordingAndExecute();
	WaitForGpuToFinishWork();
}

void Direct3D::BindDescriptorHeaps()
{
	UINT NonNullHeaps = 0;
	ID3D12DescriptorHeap* HeapsToBind[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
	for (UINT i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
	{
		ID3D12DescriptorHeap* HeapIter = m_CurrentDescriptorHeaps[i];
		if (HeapIter != nullptr)
			HeapsToBind[NonNullHeaps++] = HeapIter;
	}

	if (NonNullHeaps > 0)
		m_commandList->SetDescriptorHeaps(NonNullHeaps, HeapsToBind);
}

void Direct3D::InitializeBuffer(GpuBuffer& Dest, const void* Data, size_t NumBytes, size_t DestOffset)
{
	BeginCommandListRecording();

	DynAlloc mem = ReserveUploadMemory(NumBytes);
	SIMDMemCopy(mem.DataPtr, Data, Math::DivideByMultiple(NumBytes, 16));

	// copy data to the intermediate upload heap and then schedule a copy from the upload heap to the default texture
	TransitionResource(Dest, D3D12_RESOURCE_STATE_COPY_DEST, true);
	m_commandList->CopyBufferRegion(Dest.GetResource(), DestOffset, mem.Buffer.GetResource(), 0, NumBytes);
	TransitionResource(Dest, D3D12_RESOURCE_STATE_GENERIC_READ, true);

	// Execute the command list and wait for it to finish so we can release the upload buffer

	// Execute the command list and wait for it to finish so we can release the upload buffer
	StopCommandListRecordingAndExecute();
	WaitForGpuToFinishWork();
}

void  Direct3D::SetMeshDataHeaps()
{
	/*ID3D12DescriptorHeap* ppHeaps[] = {m_CbvHeap[m_frameIndex].Get()};
	m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	m_commandList->SetGraphicsRootDescriptorTable(0, m_CbvHeap[m_frameIndex]->GetGPUDescriptorHandleForHeapStart());
	CD3DX12_GPU_DESCRIPTOR_HANDLE hDescriptor(m_CbvHeap[m_frameIndex]->GetGPUDescriptorHandleForHeapStart(), m_srvCbvDescriptorCurrentOffset, m_srvCbvDescriptorSize);
	m_commandList->SetGraphicsRootDescriptorTable(1, m_CbvHeap[m_frameIndex]->GetGPUDescriptorHandleForHeapStart());*/
	m_DynamicViewDescriptorHeap.CommitGraphicsRootDescriptorTables(m_commandList.Get());
	m_DynamicSamplerDescriptorHeap.CommitGraphicsRootDescriptorTables(m_commandList.Get());
}

Microsoft::WRL::ComPtr<ID3D12Device> Direct3D::GetD3D12Device()
{
	return m_d3d12Device;
}

Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> Direct3D::GetCommandList()
{
	return m_commandList;
}

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> Direct3D::GetSRVHeapDescriptor()
{
	return m_CbvHeap[m_frameIndex];
}

std::wstring Direct3D::StringToWString(const std::string& s)
{
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}

bool Direct3D::PrepareDebuginfo()
{
#if defined(BUILDWITHD3D11ON12) 
	//Get frames per second
	m_LastFrameDuration = GameTime::Get()->DeltaTime();


	//Create fps string
	std::string framedurationString = "Frameduration in ms: ";
	std::string tempString = std::to_string(m_LastFrameDuration);
	framedurationString += tempString;
	AddTemporaryTextString(framedurationString, "Seogoe UI", 20.0f, 0.0f, 1.0f, 0.0f, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 10, 10, 400, 40);

	//Create fps string
	std::string fpsString = "Frames per seconds: ";
	tempString = std::to_string(GameTime::Get()->FramesPerSecond());
	fpsString += tempString;
	AddTemporaryTextString(fpsString, "Seogoe UI", 20.0f, 0.0f, 1.0f, 0.0f, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 10, 40, 350, 40);

	//Create cpu string
	std::string cpuString = "CPU: ";
	tempString = std::to_string(m_Cpu.GetCpuPercentage());
	cpuString += tempString;
	cpuString += " %";
	AddTemporaryTextString(cpuString, "Seogoe UI", 20.0f, 1.0f, 1.0f, 1.0f, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 10, 70, 200, 40);

	std::string cameraString = "Camera: ";
	tempString = std::to_string(DirectX::XMVectorGetX(m_Camera->GetPosition())) + " " + std::to_string(DirectX::XMVectorGetY(m_Camera->GetPosition())) + " " + std::to_string(DirectX::XMVectorGetZ(m_Camera->GetPosition()));
	cameraString += tempString;
	AddTemporaryTextString(cameraString, "Seogoe UI", 20.0f, 1.0f, 1.0f, 1.0f, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 10, 110, 400, 400);


	std::string cubePos = "CubePos: ";
	tempString = std::to_string(DirectX::XMVectorGetX(m_Model->GetModelRootSceneGraphNode()->GetWorldPosition())) + " " + std::to_string(DirectX::XMVectorGetY(m_Model->GetModelRootSceneGraphNode()->GetWorldPosition())) + " " + std::to_string(DirectX::XMVectorGetZ(m_Model->GetModelRootSceneGraphNode()->GetWorldPosition()));
	cubePos += tempString;
	AddTemporaryTextString(cubePos, "Seogoe UI", 20.0f, 1.0f, 1.0f, 1.0f, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 10, 180, 400, 400);
#endif

	return true;
}