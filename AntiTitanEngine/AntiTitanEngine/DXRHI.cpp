#include "stdafx.h"
#include "DXRHI.h"

Microsoft::WRL::ComPtr<ID3D12Device> DXRHI::Getd3dDevice() {
	return md3dDevice;
};

Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> DXRHI::GetCommandList()
{
	return mCommandList;
}

bool DXRHI::Get4xMsaaState() const
{
	return m4xMsaaState;
}

bool DXRHI::Init() {

#if defined(DEBUG) || defined(_DEBUG) 
	//Enable the D3D12 debug layer.
	{
		ComPtr<ID3D12Debug> debugController;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();
	}
#endif

	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&mdxgiFactory)));

	// Try to create hardware device.
	HRESULT hardwareResult = D3D12CreateDevice(
		nullptr,             // default adapter
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&md3dDevice));

	// Fallback to WARP device.
	if (FAILED(hardwareResult))
	{
		ComPtr<IDXGIAdapter> pWarpAdapter;
		ThrowIfFailed(mdxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));

		ThrowIfFailed(D3D12CreateDevice(
			pWarpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&md3dDevice)));
	}

	ThrowIfFailed(md3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&mFence)));

	mRtvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	mDsvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	mCbvSrvUavDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// Check 4X MSAA quality support for our back buffer format.
	// All Direct3D 11 capable devices support 4X MSAA for all render 
	// target formats, so we only need to check quality support.

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = mBackBufferFormat;
	msQualityLevels.SampleCount = 4 ;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 0;
	ThrowIfFailed(md3dDevice->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&msQualityLevels,
		sizeof(msQualityLevels)));

	m4xMsaaQuality = msQualityLevels.NumQualityLevels;
	assert(m4xMsaaQuality > 0 && "Unexpected MSAA quality level.");

	InitDX_CreateCommandObjects();
	InitDX_CreateSwapChain();
	//InitDX_CreateRtvAndDsvDescriptorHeaps();
	//--------------------------------------------------------------------------------

	OnResize();

	// Reset the command list to prep for initialization commands.
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

	//mAsset.LoadExternalMapActor(MapLoadPath);//要先从外部导入地图数据才能绘制//这个应该放在游戏里
	//Engine::Get()->GetAssetManager()->LoadExternalMapActor(MapLoadPath);
	Engine::Get()->GetMaterialSystem()->LoadTexture();
	//BuildDescriptorHeaps();
	BuildRootSignature();
	BuildShadersAndInputLayout();
	//LoadAsset();
	BuildPSO();

	// Execute the initialization commands.
	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until initialization is complete.
	FlushCommandQueue();

	return true;
}

void DXRHI::InitPrimitiveManagerMember()
{
	auto mRenderPrimitiveManager = Engine::Get()->GetRenderer()->GetRenderPrimitiveManager();
	mRenderPrimitiveManager->mShader = std::make_shared<DXRHIResource_Shader>();
	mRenderPrimitiveManager->mShadowMap = std::make_shared<DXRHIResource_ShadowMap>();
	mRenderPrimitiveManager->mRenderTarget = std::make_shared<DXRHIResource_RenderTarget>();

#if defined(DEBUG) || defined(_DEBUG) 
	// Enable the D3D12 debug layer.
	{
		ComPtr<ID3D12Debug> debugController;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();
	}
#endif

	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&mdxgiFactory)));

	// Try to create hardware device.
	HRESULT hardwareResult = D3D12CreateDevice(
		nullptr,             // default adapter
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&md3dDevice));

	// Fallback to WARP device.
	if (FAILED(hardwareResult))
	{
		ComPtr<IDXGIAdapter> pWarpAdapter;
		ThrowIfFailed(mdxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));

		ThrowIfFailed(D3D12CreateDevice(
			pWarpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&md3dDevice)));
	}

	ThrowIfFailed(md3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&mFence)));

	mRtvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	mDsvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	mCbvSrvUavDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// Check 4X MSAA quality support for our back buffer format.
	// All Direct3D 11 capable devices support 4X MSAA for all render 
	// target formats, so we only need to check quality support.

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = mBackBufferFormat;
	msQualityLevels.SampleCount = 4;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 0;
	ThrowIfFailed(md3dDevice->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&msQualityLevels,
		sizeof(msQualityLevels)));

	m4xMsaaQuality = msQualityLevels.NumQualityLevels;
	assert(m4xMsaaQuality > 0 && "Unexpected MSAA quality level.");

	InitDX_CreateCommandObjects();
	InitDX_CreateSwapChain();
	//InitDX_CreateRtvAndDsvDescriptorHeaps();

	//Heap创建
	std::string rtvHeapName = "mRtvHeap";
	auto mRendertarget = Engine::Get()->GetRenderer()->GetRenderPrimitiveManager()->mRenderTarget;
	auto SwapChainBufferCount = std::dynamic_pointer_cast<DXRHIResource_RenderTarget>(mRendertarget)->GetSwapChainBufferCount();
	auto rtvHeap = CreateDescriptorHeap(rtvHeapName, SwapChainBufferCount, 2,0);
	mRenderPrimitiveManager->mHeapsLib.insert(std::pair<std::string, std::shared_ptr<RHIResource_Heap>>(rtvHeapName, rtvHeap));

	std::string dsvHeapName = "mDsvHeap";
	auto dsvHeap = CreateDescriptorHeap(dsvHeapName, 100, 3, 0);
	mRenderPrimitiveManager->mHeapsLib.insert(std::pair<std::string, std::shared_ptr<RHIResource_Heap>>(dsvHeapName, dsvHeap));

	std::string cbvHeapName = "mCbvHeap";
	auto cbvHeap = CreateDescriptorHeap(cbvHeapName, 10000, 0, 1);
	mRenderPrimitiveManager->mHeapsLib.insert(std::pair<std::string, std::shared_ptr<RHIResource_Heap>>(cbvHeapName, cbvHeap));

	std::string TextureHeapName = "mTextureHeap";
	auto TextureHeap = CreateDescriptorHeap(TextureHeapName, 1, 0,0);
	mRenderPrimitiveManager->mHeapsLib.insert(std::pair<std::string, std::shared_ptr<RHIResource_Heap>>(TextureHeapName, TextureHeap));

	std::string ShadowSrvDescriptorHeapName = "mShadowSrvDescriptorHeap";
	auto ShadowSrvDescriptorHeap = CreateDescriptorHeap(ShadowSrvDescriptorHeapName, 10000, 0,1);
	mRenderPrimitiveManager->mHeapsLib.insert(std::pair<std::string, std::shared_ptr<RHIResource_Heap>>(ShadowSrvDescriptorHeapName,ShadowSrvDescriptorHeap));

	std::string ShadowDsvDescriptorHeapName = "mShadowDsvDescriptorHeap";
	auto ShadowDsvDescriptorHeap = CreateDescriptorHeap(ShadowDsvDescriptorHeapName, 10000, 3, 0);
	mRenderPrimitiveManager->mHeapsLib.insert(std::pair<std::string, std::shared_ptr<RHIResource_Heap>>(ShadowDsvDescriptorHeapName, ShadowDsvDescriptorHeap));

	//--------------------------------------------------------------------------------
	OnResize();

	// Reset the command list to prep for initialization commands.
	ResetCommandList();
}

std::shared_ptr<RHIResource_Heap> DXRHI::CreateDescriptorHeap(std::string heapName,int NumDescriptors, int mHeapType,int mFlag)
{
	std::shared_ptr<RHIResource_Heap> heap = std::make_shared<DXRHIResource_Heap>();
	auto mHeap = std::dynamic_pointer_cast<DXRHIResource_Heap>(heap);
	
	D3D12_DESCRIPTOR_HEAP_TYPE heapType = D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
	D3D12_DESCRIPTOR_HEAP_FLAGS heapFlag = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	switch (mHeapType)
	{
	case 0:  heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;	break;
	case 1:  heapType = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;	break;
	case 2:  heapType = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;	break;
	case 3:  heapType = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;	break;
	case 4:  heapType = D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;	break;
	break;
	}

	switch (mFlag)
	{
	case 0:  heapFlag = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;			break;
	case 1:  heapFlag = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;	break;
	break;
	}


	D3D12_DESCRIPTOR_HEAP_DESC HeapDesc;
	HeapDesc.NumDescriptors = NumDescriptors;
	HeapDesc.Type = heapType;
	HeapDesc.Flags = heapFlag;
	HeapDesc.NodeMask = 0;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(
		&HeapDesc, IID_PPV_ARGS(mHeap->mDescriptorHeap.GetAddressOf())));

	return heap;
}

void DXRHI::ResetCommandList()
{
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));
}

void DXRHI::InitDX_CreateCommandObjects() {

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(md3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCommandQueue)));

	ThrowIfFailed(md3dDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(mDirectCmdListAlloc.GetAddressOf())));

	ThrowIfFailed(md3dDevice->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		mDirectCmdListAlloc.Get(), // Associated command allocator
		nullptr,                   // Initial PipelineStateObject
		IID_PPV_ARGS(mCommandList.GetAddressOf())));

	// Start off in a closed state.  This is because the first time we refer 
	// to the command list we will Reset it, and it needs to be closed before
	// calling Reset.
	mCommandList->Close();
};

void DXRHI::InitDX_CreateSwapChain() {
	auto mRendertarget = Engine::Get()->GetRenderer()->GetRenderPrimitiveManager()->mRenderTarget;
	auto SwapChainBufferCount = std::dynamic_pointer_cast<DXRHIResource_RenderTarget>(mRendertarget)->GetSwapChainBufferCount();

	// Release the previous swapchain we will be recreating.
	mSwapChain.Reset();

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = mClientWidth;
	sd.BufferDesc.Height = mClientHeight;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = mBackBufferFormat;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	sd.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = SwapChainBufferCount;
	sd.OutputWindow = std::dynamic_pointer_cast<Win32Window>(Engine::Get()->GetWindow())->GetHWND();
	//sd.OutputWindow = dynamic_cast<Win32Window*>(Engine::Get()->GetWindow())->GetHWND();
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// Note: Swap chain uses queue to perform flush.
	ThrowIfFailed(mdxgiFactory->CreateSwapChain(
		mCommandQueue.Get(),
		&sd,
		mSwapChain.GetAddressOf()));
};

//void DXRHI::LoadExternalMapActor(std::string MapActorLoadPath)
//{
//	Engine::Get()->GetAssetManager()->LoadExternalMapActor(MapActorLoadPath);
//}

//void DXRHI::LoadLightInfo(std::string MapLightLoadPath)
//{
//	Engine::Get()->GetAssetManager()->mLight = std::make_shared<FLight>();
//	Engine::Get()->GetAssetManager()->mLight->LoadLightFromBat(MapLightLoadPath);
//	Engine::Get()->GetAssetManager()->mLight->InitView();
//	Engine::Get()->GetAssetManager()->mLight->InitProj();
//}

void DXRHI::LoadDDSTextureToResource(std::wstring Path, int TextureIndex)
{
	Engine::Get()->GetMaterialSystem()->mTexture = std::make_shared<Texture>();
	Engine::Get()->GetMaterialSystem()->mTexture->Name = "TestTexture";
	Engine::Get()->GetMaterialSystem()->mTexture->Filename = Path;

	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(),
		Engine::Get()->GetMaterialSystem()->mTexture->Filename.c_str(),
		Engine::Get()->GetMaterialSystem()->mTexture->Resource,
		Engine::Get()->GetMaterialSystem()->mTexture->UploadHeap));
	
	//------------------------------------------------------
	//手动给一个测试
	Engine::Get()->GetMaterialSystem()->mTextureNormal = std::make_shared<Texture>();
	Engine::Get()->GetMaterialSystem()->mTextureNormal->Name = "TestNormal";
	Engine::Get()->GetMaterialSystem()->mTextureNormal->Filename = L"Texture/Stone_Normal.dds";

	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(),
		Engine::Get()->GetMaterialSystem()->mTextureNormal->Filename.c_str(),
		Engine::Get()->GetMaterialSystem()->mTextureNormal->Resource,
		Engine::Get()->GetMaterialSystem()->mTextureNormal->UploadHeap));
}

void DXRHI::BuildShadow()
{
	auto mRenderPrimitiveManager = Engine::Get()->GetRenderer()->GetRenderPrimitiveManager();
	auto shadowResource = std::dynamic_pointer_cast<DXRHIResource_ShadowMap>(mRenderPrimitiveManager->mShadowMap);
	auto DXShader = std::dynamic_pointer_cast<DXRHIResource_Shader> (mRenderPrimitiveManager->mShader);

	shadowResource->mViewport = { 0.0f, 0.0f, (float)shadowResource->mWidth, (float)shadowResource->mHeight, 0.0f, 1.0f };
	shadowResource->mScissorRect = { 0, 0, (int)shadowResource->mWidth, (int)shadowResource->mHeight };

	//ShadowMap内CpuSrv,GpuSrv指针赋值,创建一个ShadowSrvHeap
	auto mShadowSrvDescriptorHeap = std::dynamic_pointer_cast<DXRHIResource_Heap>(mRenderPrimitiveManager->GetHeapByName("mShadowSrvDescriptorHeap"));
	shadowResource->mhCpuSrv = CD3DX12_CPU_DESCRIPTOR_HANDLE(mShadowSrvDescriptorHeap->mDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	shadowResource->mhGpuSrv = CD3DX12_GPU_DESCRIPTOR_HANDLE(mShadowSrvDescriptorHeap->mDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

	//ShadowMap内CpuDsv指针赋值,创建一个ShadowDsvHeap
	auto mShadowDsvDescriptorHeap = std::dynamic_pointer_cast<DXRHIResource_Heap>(mRenderPrimitiveManager->GetHeapByName("mShadowDsvDescriptorHeap"));
	shadowResource->mhCpuDsv = CD3DX12_CPU_DESCRIPTOR_HANDLE(mShadowDsvDescriptorHeap->mDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	//一个Resource
	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = shadowResource->mWidth;
	texDesc.Height = shadowResource->mHeight;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	texDesc.SampleDesc.Count = 1;//像素的多重采样数
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;

	ThrowIfFailed(md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		&optClear,
		IID_PPV_ARGS(&shadowResource->mShadowResource)));

	//两个View
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	srvDesc.Texture2D.PlaneSlice = 0;
	md3dDevice->CreateShaderResourceView(shadowResource->mShadowResource.Get(),&srvDesc, shadowResource->mhCpuSrv);

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.Texture2D.MipSlice = 0;
	md3dDevice->CreateDepthStencilView(shadowResource->mShadowResource.Get(), &dsvDesc, shadowResource->mhCpuDsv);

	//一根shadowMap的管线
		//ShadowMapPSO有自己的Shader
		DXShader->mvsShadowMapCode = d3dUtil::CompileShader(L"Shaders\\Shadows.hlsl", nullptr, "VS", "vs_5_1");
		DXShader->mpsShadowMapCode = d3dUtil::CompileShader(L"Shaders\\Shadows.hlsl", nullptr, "PS", "ps_5_1");
		DXShader->mInputLayout =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC shadowMapPsoDesc;
	ZeroMemory(&shadowMapPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	shadowMapPsoDesc.InputLayout = { DXShader->mInputLayout.data(), (UINT)DXShader->mInputLayout.size() };
	shadowMapPsoDesc.pRootSignature = mRootSignature.Get();
	shadowMapPsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(DXShader->mvsShadowMapCode->GetBufferPointer()),
		DXShader->mvsShadowMapCode->GetBufferSize()
	};
	shadowMapPsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(DXShader->mpsShadowMapCode->GetBufferPointer()),
		DXShader->mpsShadowMapCode->GetBufferSize()
	};

	shadowMapPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	shadowMapPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	shadowMapPsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	shadowMapPsoDesc.RasterizerState.FrontCounterClockwise = true;
	shadowMapPsoDesc.SampleMask = UINT_MAX;
	shadowMapPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	shadowMapPsoDesc.NumRenderTargets = 0;//
	shadowMapPsoDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;//
	shadowMapPsoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	shadowMapPsoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	shadowMapPsoDesc.DSVFormat = mDepthStencilFormat;

	shadowMapPsoDesc.RasterizerState.DepthBias = 400000;//
	shadowMapPsoDesc.RasterizerState.DepthBiasClamp = 0.0f;//
	shadowMapPsoDesc.RasterizerState.SlopeScaledDepthBias = 1.0f;//
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&shadowMapPsoDesc, IID_PPV_ARGS(&mShadowMapPSO)));
}

//void DXRHI::BuildTexture(std::string Name ,std::wstring Path)
//{
//	mRHIResourceManager->mTextures.resize(100);
//	mRHIResourceManager->mTextures[Engine::Get()->GetMaterialSystem()->mTextureNum] = std::make_shared<DXRHIResource_Texture>();
//	auto Textures=std::dynamic_pointer_cast<DXRHIResource_Texture>(mRHIResourceManager->mTextures[Engine::Get()->GetMaterialSystem()->mTextureNum]);
//
//	Textures->Name = Name;
//	Textures->Filename = Path;
//
//	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
//		mCommandList.Get(), Textures->Filename.c_str(),
//		Textures->Resource, Textures->UploadHeap));
//	mRHIResourceManager->TextureMap.insert(std::make_pair(Engine::Get()->GetMaterialSystem()->mTextureNum, Name));
//
//	Engine::Get()->GetMaterialSystem()->mTextureNum++;
//}

//void DXRHI::BuildMember()
//{
//	SetDescriptorHeaps();
//	BuildRootSignature();
//}


void DXRHI::SetShader(std::wstring ShaderPath)
{
	auto mRenderPrimitiveManager = Engine::Get()->GetRenderer()->GetRenderPrimitiveManager();

	auto DXShader = std::dynamic_pointer_cast<DXRHIResource_Shader> (mRenderPrimitiveManager->mShader);
	DXShader->mvsByteCode = d3dUtil::CompileShader(ShaderPath, nullptr, "VS", "vs_5_0");
	DXShader->mpsByteCode = d3dUtil::CompileShader(ShaderPath, nullptr, "PS", "ps_5_0");

	DXShader->mInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void DXRHI::LoadMeshAndSetBuffer()
{
	auto mRenderPrimitiveManager = Engine::Get()->GetRenderer()->GetRenderPrimitiveManager();

	std::string StaticMeshPath;
	std::set<std::string> StaticMeshs;//用于去重
	StaticMesh mesh;
	Engine::Get()->GetAssetManager()->GetGeometryLibrary()->resize(Engine::Get()->GetAssetManager()->GetMapActorInfo()->Size());
	Vertex vertice;
	//mRHIResourceManager->VBIBBuffers.resize(Engine::Get()->GetAssetManager()->GetMapActorInfo()->Size());
	int MeshNum=0;
	//循环加入MeshGeometry
	for (int i = 0; i < Engine::Get()->GetAssetManager()->Geos.size(); i++)
	{
		std::vector<Vertex> vertices;
		//相同的StaticMesh不用重复建立
		auto check = StaticMeshs.find(Engine::Get()->GetAssetManager()->GetMapActorInfo()->MeshNameArray[i]);
		if (check == StaticMeshs.end()) {
			StaticMeshs.insert(Engine::Get()->GetAssetManager()->GetMapActorInfo()->MeshNameArray[i]);
			Engine::Get()->GetAssetManager()->GetMapofGeosMesh()->insert(std::pair<int, std::string>(i, Engine::Get()->GetAssetManager()->GetMapActorInfo()->MeshNameArray[i]));
		}
		else { continue; }

		//读取mesh信息
		StaticMeshPath = "SplitMesh/" + Engine::Get()->GetAssetManager()->GetMapActorInfo()->MeshNameArray[i];
		StaticMeshPath.erase(StaticMeshPath.length() - 1);
		StaticMeshPath += ".bat";
		mesh.LoadStaticMeshFromBat(StaticMeshPath);

		if (mesh.MeshInfo.MeshVertexInfo.size() < 3) { continue; }//没有StaticMesh就不读取
		//--------------------------------------------------------------------------------

		for (int j = 0; j < mesh.MeshInfo.MeshVertexInfo.size(); j++)
		{
			vertice.Pos = mesh.MeshInfo.MeshVertexInfo[j];
			vertice.Color = {
				float(j) / mesh.MeshInfo.MeshVertexInfo.size(),
				float(j) / mesh.MeshInfo.MeshVertexInfo.size(),
				float(j) / mesh.MeshInfo.MeshVertexInfo.size(),
				1 };//初始化赋值为黑白色
			vertice.Normal = mesh.MeshInfo.MeshVertexNormalInfo[j];
			vertice.TexCoord = mesh.MeshInfo.MeshTexCoord[j];

			vertices.push_back(vertice);
		}
		std::shared_ptr<DXRHIResource_MeshBuffer> buffer=std::make_shared<DXRHIResource_MeshBuffer>();
		buffer->indices = mesh.MeshInfo.MeshIndexInfo;
		buffer->vertices = vertices;
		buffer->MeshName = mesh.getMeshName();

		//mRHIResourceManager->VBIBBuffers[MeshNum] = std::make_shared<DXRHIRessource_VBIBBuffer>();
		
		mRenderPrimitiveManager->VBIBBuffers.push_back(buffer);
		std::string MeshName = Engine::Get()->GetAssetManager()->GetMapActorInfo()->MeshNameArray[i];
		MeshName.erase(MeshName.length() - 1);

		mRenderPrimitiveManager->MeshMap.insert(std::make_pair(MeshNum,MeshName));
		MeshNum++;
	}
}

void DXRHI::CreateVBIB() {
	auto mRenderPrimitiveManager = Engine::Get()->GetRenderer()->GetRenderPrimitiveManager();

	for (int i = 0; i < mRenderPrimitiveManager->VBIBBuffers.size(); i++)
	{
		mRenderPrimitiveManager->VBIBBuffers;
		mRenderPrimitiveManager->MeshMap;
		auto test2Buffer =mRenderPrimitiveManager->VBIBBuffers;
		auto testBuffer = std::dynamic_pointer_cast<DXRHIResource_MeshBuffer>(test2Buffer[i]);
		auto VIBuffer = std::dynamic_pointer_cast<DXRHIResource_MeshBuffer>(mRenderPrimitiveManager->VBIBBuffers[i]);
		UINT vbByteSize;
		UINT ibByteSize;
		vbByteSize = (UINT)VIBuffer->vertices.size() * sizeof(Vertex);
		ibByteSize = (UINT)VIBuffer->indices.size() * sizeof(std::uint32_t);

		Engine::Get()->GetAssetManager()->Geos[i] = std::make_unique<MeshGeometry>();
		Engine::Get()->GetAssetManager()->Geos[i]->Name = VIBuffer->MeshName;

		ThrowIfFailed(D3DCreateBlob(vbByteSize, &Engine::Get()->GetAssetManager()->Geos[i]->VertexBufferCPU));
		CopyMemory(Engine::Get()->GetAssetManager()->Geos[i]->VertexBufferCPU->GetBufferPointer(), VIBuffer->vertices.data(), vbByteSize);

		ThrowIfFailed(D3DCreateBlob(ibByteSize, &Engine::Get()->GetAssetManager()->Geos[i]->IndexBufferCPU));
		CopyMemory(Engine::Get()->GetAssetManager()->Geos[i]->IndexBufferCPU->GetBufferPointer(), VIBuffer->indices.data(), ibByteSize);

		VIBuffer->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),mCommandList.Get(), VIBuffer->vertices.data(), vbByteSize, Engine::Get()->GetAssetManager()->Geos[i]->VertexBufferUploader);

		VIBuffer->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),mCommandList.Get(), VIBuffer->indices.data(), ibByteSize, Engine::Get()->GetAssetManager()->Geos[i]->IndexBufferUploader);

		VIBuffer->VertexByteStride=sizeof(Vertex);
		VIBuffer->VertexBufferByteSize = vbByteSize;
		VIBuffer->IndexFormat=DXGI_FORMAT_R32_UINT;
		VIBuffer->IndexBufferByteSize = ibByteSize;

		SubmeshGeometry submesh;
		submesh.IndexCount = (UINT)VIBuffer->indices.size();
		submesh.StartIndexLocation = 0;
		submesh.BaseVertexLocation = 0;
		
		VIBuffer->DrawArgs[VIBuffer->MeshName] = submesh;
		//Engine::Get()->GetAssetManager()->Geos[i]->DrawArgs[Engine::Get()->GetAssetManager()->GetMapActorInfo()->MeshNameArray[i]] = submesh;
	}
}

void DXRHI::InitPSO()
{
	auto mRenderPrimitiveManager = Engine::Get()->GetRenderer()->GetRenderPrimitiveManager();
	auto DXShader = std::dynamic_pointer_cast<DXRHIResource_Shader> (mRenderPrimitiveManager->mShader);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	psoDesc.InputLayout = { DXShader->mInputLayout.data(), (UINT)DXShader->mInputLayout.size() };
	psoDesc.pRootSignature = mRootSignature.Get();
	psoDesc.VS =
	{
		reinterpret_cast<BYTE*>(DXShader->mvsByteCode->GetBufferPointer()),
		DXShader->mvsByteCode->GetBufferSize()
	};
	psoDesc.PS =
	{
		reinterpret_cast<BYTE*>(DXShader->mpsByteCode->GetBufferPointer()),
		DXShader->mpsByteCode->GetBufferSize()
	};

	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.FrontCounterClockwise = true;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = mBackBufferFormat;
	psoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	psoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	psoDesc.DSVFormat = mDepthStencilFormat;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPSO)));
}

void DXRHI::ExecuteCommandList()
{
	// Execute the initialization commands.
	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
}

void DXRHI::WaitCommandComplete()
{
	FlushCommandQueue();
}

void DXRHI::OnResize()
{
	//assert(md3dDevice);
	//assert(mSwapChain);
	//assert(mDirectCmdListAlloc);

	// Flush before changing any resources.
	WaitCommandComplete();

	ResetCommandList();

	// Release the previous resources we will be recreating.
	resetRenderTarget();//Resize的时候才要用，初始化不需要用

	// Resize the swap chain.
	resizeSwapChain();

	//auto mRendertarget = Engine::Get()->GetRenderer()->GetRenderPrimitiveManager()->mRenderTarget;
	//auto mSwapChainBuffer     = std::dynamic_pointer_cast<DXRHIResource_RenderTarget>(mRendertarget)->mSwapChainBuffer;
	//auto SwapChainBufferCount = std::dynamic_pointer_cast<DXRHIResource_RenderTarget>(mRendertarget)->GetSwapChainBufferCount();
	//auto mDepthStencilBuffer  = std::dynamic_pointer_cast<DXRHIResource_RenderTarget>(mRendertarget)->mDepthStencilBuffer;
	//

	BuildRenderTarget();

	// Execute the resize commands.
	ExecuteCommandList();

	WaitCommandComplete();

	// Update the viewport transform to cover the client area.
	SetScreenSetViewPort(0,0, static_cast<float>(Engine::Get()->GetWindow()->mClientWidth), static_cast<float>(Engine::Get()->GetWindow()->mClientHeight),0.0f,1.0f);

	SetScissorRect(0, 0, Engine::Get()->GetWindow()->mClientWidth, Engine::Get()->GetWindow()->mClientHeight);
}

void DXRHI::resetRenderTarget()
{
	auto mRendertarget = std::dynamic_pointer_cast<DXRHIResource_RenderTarget>(Engine::Get()->GetRenderer()->GetRenderPrimitiveManager()->mRenderTarget);
	auto mSwapChainBuffer = mRendertarget->mSwapChainBuffer;
	auto SwapChainBufferCount = mRendertarget->GetSwapChainBufferCount();

	for (int i = 0; i < SwapChainBufferCount; ++i) {
		mSwapChainBuffer[i].Reset();
	}
	mRendertarget->GetDepthStencilBuffer().Reset();
}

void DXRHI::resizeSwapChain()
{
	auto mRendertarget = Engine::Get()->GetRenderer()->GetRenderPrimitiveManager()->mRenderTarget;
	auto SwapChainBufferCount = std::dynamic_pointer_cast<DXRHIResource_RenderTarget>(mRendertarget)->GetSwapChainBufferCount();

	ThrowIfFailed(mSwapChain->ResizeBuffers(
		SwapChainBufferCount,
		Engine::Get()->GetWindow()->mClientWidth,
		Engine::Get()->GetWindow()->mClientHeight,
		mBackBufferFormat,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	std::dynamic_pointer_cast<DXRHIResource_RenderTarget>(mRendertarget)->SetCurrBackBufferIndex(0);

}

void DXRHI::BuildRenderTarget()
{
	auto mRenderPrimitiveManager = Engine::Get()->GetRenderer()->GetRenderPrimitiveManager();
	auto mRtvHeap = std::dynamic_pointer_cast<DXRHIResource_Heap>(mRenderPrimitiveManager->GetHeapByName("mRtvHeap"));

	auto mRendertarget = std::dynamic_pointer_cast<DXRHIResource_RenderTarget>(Engine::Get()->GetRenderer()->GetRenderPrimitiveManager()->mRenderTarget);
	auto mSwapChainBuffer = mRendertarget->mSwapChainBuffer;
	auto SwapChainBufferCount = mRendertarget->GetSwapChainBufferCount();

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mRtvHeap->mDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < SwapChainBufferCount; i++)
	{
		ThrowIfFailed(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mSwapChainBuffer[i])));
		md3dDevice->CreateRenderTargetView(mSwapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
		rtvHeapHandle.Offset(1, mRtvDescriptorSize);
	}

	// Create the depth/stencil buffer and view.
	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = Engine::Get()->GetWindow()->mClientWidth;
	depthStencilDesc.Height = Engine::Get()->GetWindow()->mClientHeight;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;

	// Correction 11/12/2016: SSAO chapter requires an SRV to the depth buffer to read from 
	// the depth buffer.  Therefore, because we need to create two views to the same resource:
	//   1. SRV format: DXGI_FORMAT_R24_UNORM_X8_TYPELESS
	//   2. DSV Format: DXGI_FORMAT_D24_UNORM_S8_UINT
	// we need to create the depth buffer resource with a typeless format.  
	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	depthStencilDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	depthStencilDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = mDepthStencilFormat;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;

	ThrowIfFailed(md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&depthStencilDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&optClear,
		IID_PPV_ARGS(mRendertarget->mDepthStencilBuffer.GetAddressOf())));

	// Create descriptor to mip level 0 of entire resource using the format of the resource.

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = mDepthStencilFormat;
	dsvDesc.Texture2D.MipSlice = 0;

	auto mDsvHeap = std::dynamic_pointer_cast<DXRHIResource_Heap>(mRenderPrimitiveManager->GetHeapByName("mDsvHeap"));
	auto mDsvHeaphandle = mDsvHeap->mDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	md3dDevice->CreateDepthStencilView(mRendertarget->mDepthStencilBuffer.Get(), &dsvDesc, mDsvHeaphandle);

	// Transition the resource from its initial state to be used as a depth buffer.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mRendertarget->mDepthStencilBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));

}

void DXRHI::SetScreenSetViewPort(float TopLeftX, float TopLeftY, float Width, float Height, float MinDepth, float MaxDepth)
{
	mScreenViewport.TopLeftX = TopLeftX;
	mScreenViewport.TopLeftY = TopLeftY;
	mScreenViewport.Width = Width;
	mScreenViewport.Height = Height;
	mScreenViewport.MinDepth = MinDepth;
	mScreenViewport.MaxDepth = MaxDepth;
}

void DXRHI::SetScissorRect(long Left, long Top, long Right, long Bottom)
{
	mScissorRect = { Left, Top, Right, Bottom };
}

void DXRHI::Update()
{
	//Engine::Get()->GetAssetManager()->mLight->Pitch(0.0002f);
	Engine::Get()->GetAssetManager()->mLight->Yaw(0.0002f);
	
	for (int i = 0; i < Engine::Get()->GetAssetManager()->GetMapActorInfo()->Size(); i++)
	{
		ObjectConstants objConstants;

		//UPDate Actor--------------------------------------------------------------------------------

		//auto world = MathHelper::Identity4x4();
		auto location = XMMatrixTranslation(
			Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].translation.x,
			Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].translation.y,
			Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].translation.z
		);
		auto Scale = XMMatrixScaling(
			Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].scale3D.x,
			Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].scale3D.y,
			Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].scale3D.z
		);
		auto Rotator = XMMatrixScaling(
			Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].rotation.Pitch,
			Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].rotation.Yaw,
			Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].rotation.Roll
		);

		DirectX::XMVECTORF32 g_XMIdentityR3 = { { {
				Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsQuatArray[i].X,
				Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsQuatArray[i].Y,
				Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsQuatArray[i].Z,
				Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsQuatArray[i].W
			} } };

		auto mrotation = DirectX::XMMatrixRotationQuaternion(g_XMIdentityR3);

		glm::mat4 translateMat4 = glm::translate(glm::identity<glm::mat4>(), glm::vec3(
			Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].translation.x,
			Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].translation.y,
			Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].translation.z
		));

		glm::mat4 scaleMat4 = glm::scale(glm::identity<glm::mat4>(), glm::vec3(
			Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].scale3D.x,
			Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].scale3D.y,
			Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].scale3D.z
		));

		glm::quat rotationQuat(
			Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsQuatArray[i].X,
			Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsQuatArray[i].Y,
			Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsQuatArray[i].Z,
			Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsQuatArray[i].W
		);
		glm::mat4 rotationMat4 = glm::toMat4(rotationQuat);

		auto world = Scale * mrotation * location;
		glm::mat4 worldMat4 = scaleMat4 * rotationMat4 * translateMat4;
		//!!!旋转矩阵好像有问题，用glm的rotator传给Shader，mul(Normal,rotator)的值不对，表现出来的颜色不是正确的。
		//!!!要看正确的颜色可以乘XMMATRIX的rotation，XMMATRIX还要转化为FLOAT4X4

		auto mCamera = Engine::Get()->GetRenderer()->GetCamera();
		XMMATRIX worldViewProj = world * XMLoadFloat4x4(&mCamera->GetView4x4f()) * XMLoadFloat4x4(&mCamera->GetProj4x4f());
		glm::mat4 worldViewProjMat4 = mCamera->GetProjMat4() * mCamera->GetViewMat4() * worldMat4;

		//XMMATRIX worldViewProj = world * XMLoadFloat4x4(&mCamera.GetView4x4f()) * XMLoadFloat4x4((&mCamera.GetProj4x4f()));
		//glm::mat4 worldViewProjMat4 = mCamera.GetProjMat4() * mCamera.GetViewMat4() * worldMat4;

		XMStoreFloat4x4(&objConstants.WorldViewProj, XMMatrixTranspose(worldViewProj));
		objConstants.WorldViewProjMat4 = glm::transpose(worldViewProjMat4);
		//objConstants.mTime = Engine::Get()->gt.TotalTime();
		XMStoreFloat4x4(&objConstants.rotation, XMMatrixTranspose(mrotation));

		//UPDate LIGHT--------------------------------------------------------------------------------
		auto TestVV = Engine::Get()->GetAssetManager()->mLight->GetView();//重新创建的V矩阵
		auto TestPP = Engine::Get()->GetAssetManager()->mLight->GetProj();//重新创建的P矩阵

		XMMATRIX T(
			0.5f, 0.0f, 0.0f, 0.0f,
			0.0f,-0.5f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.5f, 0.5f, 0.0f, 1.0f);

		////重新造的VP
		XMMATRIX VP = TestVV * TestPP;
		XMMATRIX S  =  TestVV * TestPP * T;
		XMMATRIX LightworldViewProj = world * TestVV * TestPP;
		XMMATRIX LightworldViewProjT = world * TestVV * TestPP * T;

		XMStoreFloat4x4(&objConstants.gWorld, XMMatrixTranspose(world));
		XMStoreFloat4x4(&objConstants.gLightVP, XMMatrixTranspose(VP));
		XMStoreFloat4x4(&objConstants.gShadowTransform, XMMatrixTranspose(S));
		XMStoreFloat4x4(&objConstants.gLightMVP, XMMatrixTranspose(LightworldViewProj));
		XMStoreFloat4x4(&objConstants.gLightMVPT, XMMatrixTranspose(LightworldViewProjT));

		mObjectCB->CopyData(i, objConstants);
	}
}

void DXRHI::Draw()
{
	// Reuse the memory associated with command recording.
	// We can only reset when the associated command lists have finished execution on the GPU.
	ThrowIfFailed(mDirectCmdListAlloc->Reset());

	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
	// Reusing the command list reuses memory.
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), mPSO.Get()));

	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	// Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// Clear the back buffer and depth buffer.
	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::Black, 0, nullptr);

	auto mRenderPrimitiveManager = Engine::Get()->GetRenderer()->GetRenderPrimitiveManager();
	auto mDsvHeap = std::dynamic_pointer_cast<DXRHIResource_Heap>(mRenderPrimitiveManager->GetHeapByName("mDsvHeap"));
	auto mDsvHeaphandle = mDsvHeap->mDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	mCommandList->ClearDepthStencilView(mDsvHeaphandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &mDsvHeaphandle);

	auto mCbvHeap = std::dynamic_pointer_cast<DXRHIResource_Heap>(mRenderPrimitiveManager->GetHeapByName("mCbvHeap"));

	ID3D12DescriptorHeap* descriptorHeaps[] = { mCbvHeap->mDescriptorHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

	for (int i = 0; i < Engine::Get()->GetAssetManager()->GetMapActorInfo()->Size(); i++)//绘制每一个Actor
	{
		auto GeoIndex = Engine::Get()->GetAssetManager()->GetGeoKeyByName(Engine::Get()->GetAssetManager()->GetMapActorInfo()->MeshNameArray[i]);
		auto mCbvHeap = std::dynamic_pointer_cast<DXRHIResource_Heap>(mRenderPrimitiveManager->GetHeapByName("mCbvHeap"));

		mCommandList->IASetVertexBuffers(0, 1, &Engine::Get()->GetAssetManager()->Geos[GeoIndex]->VertexBufferView());
		mCommandList->IASetIndexBuffer(&Engine::Get()->GetAssetManager()->Geos[GeoIndex]->IndexBufferView());
		mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		auto heapHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mCbvHeap->mDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		heapHandle.Offset(i, md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
		mCommandList->SetGraphicsRootDescriptorTable(0, heapHandle);

		//贴图的Size要记得改下面的Offset
		auto GPUHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mCbvHeap->mDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		GPUHandle.Offset(Engine::Get()->GetAssetManager()->GetMapActorInfo()->Size()+i, md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
		mCommandList->SetGraphicsRootDescriptorTable(1, GPUHandle);

		mCommandList->DrawIndexedInstanced(Engine::Get()->GetAssetManager()->Geos[GeoIndex]->DrawArgs[Engine::Get()->GetAssetManager()->GetMapActorInfo()->MeshNameArray[i]].IndexCount, 1, 0, 0, 0);
	
	}
	auto mCamera = Engine::Get()->GetRenderer()->GetCamera();
	mCommandList->SetGraphicsRoot32BitConstants(2, 3, &mCamera->GetPosition(), 0);

	// Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	ExecuteCommandList();

	// swap the back and front buffers
	ThrowIfFailed(mSwapChain->Present(0, 0));
	auto mRendertarget = Engine::Get()->GetRenderer()->GetRenderPrimitiveManager()->mRenderTarget;
	auto mCurrBackBufferIndex = std::dynamic_pointer_cast<DXRHIResource_RenderTarget>(mRendertarget)->GetCurrBackBufferIndex();
	auto SwapChainBufferCount = std::dynamic_pointer_cast<DXRHIResource_RenderTarget>(mRendertarget)->GetSwapChainBufferCount();
	std::dynamic_pointer_cast<DXRHIResource_RenderTarget>(mRendertarget)->SetCurrBackBufferIndex((mCurrBackBufferIndex + 1) % SwapChainBufferCount);

	WaitCommandComplete();

}

void DXRHI::DrawSceneToShadowMap()
{
	auto mRenderPrimitiveManager = Engine::Get()->GetRenderer()->GetRenderPrimitiveManager();
	auto shadowMap = std::dynamic_pointer_cast<DXRHIResource_ShadowMap>(mRenderPrimitiveManager->mShadowMap);
	auto mCbvHeap = std::dynamic_pointer_cast<DXRHIResource_Heap>(mRenderPrimitiveManager->GetHeapByName("mCbvHeap"));

	mCommandList->RSSetViewports(1, &shadowMap->mViewport);
	mCommandList->RSSetScissorRects(1, &shadowMap->mScissorRect);

	//切换到Depth-Write
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(shadowMap->mShadowResource.Get(),
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE));
	//mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(shadowMap->mShadowResource.Get(),
	//	D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE));

	mCommandList->ClearDepthStencilView(shadowMap->mhCpuDsv,
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	mCommandList->OMSetRenderTargets(0, nullptr, false, &shadowMap->mhCpuDsv);
	//mCommandList->OMSetRenderTargets(0, nullptr, false, &DepthStencilView());

	ID3D12DescriptorHeap* descriptorHeaps[] = { mCbvHeap->mDescriptorHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	mCommandList->SetPipelineState(mShadowMapPSO.Get());
	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

	for (int ActorIndex = 0; ActorIndex < Engine::Get()->GetAssetManager()->GetMapActorInfo()->Size(); ActorIndex++)//绘制每一个Actor
	{
		auto DrawMeshName = Engine::Get()->GetAssetManager()->GetMapActorInfo()->MeshNameArray[ActorIndex];
		DrawMeshName.erase(DrawMeshName.size() - 1, 1);
		auto testGeoIndex = mRenderPrimitiveManager->GetMeshKeyByName(DrawMeshName);
		auto mVBIB = std::dynamic_pointer_cast<DXRHIResource_MeshBuffer>(mRenderPrimitiveManager->VBIBBuffers[testGeoIndex]);

		mCommandList->IASetVertexBuffers(0, 1, &mVBIB->GetVertexBufferView());
		mCommandList->IASetIndexBuffer(&mVBIB->GetIndexBufferView());
		mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		auto heapHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mCbvHeap->mDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		heapHandle.Offset(ActorIndex, md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
		mCommandList->SetGraphicsRootDescriptorTable(0, heapHandle);

		mCommandList->DrawIndexedInstanced(mVBIB->DrawArgs[Engine::Get()->GetAssetManager()->GetMapActorInfo()->MeshNameArray[ActorIndex]].IndexCount, 1, 0, 0, 0);
	}

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(shadowMap->mShadowResource.Get(),
		D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ));

	//mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(shadowMap->mShadowResource.Get(),
	//	D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

}

void DXRHI::DrawReset()
{
	ThrowIfFailed(mDirectCmdListAlloc->Reset());
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), mPSO.Get()));
}

void DXRHI::ResetViewports(int NumViewPort, ScreenViewport& vp)
{
	mScreenViewport.Height=vp.Height;
	mScreenViewport.MaxDepth=vp.MaxDepth;
	mScreenViewport.MinDepth = vp.MinDepth ;
	mScreenViewport.TopLeftX = vp.TopLeftX ;
	mScreenViewport.TopLeftY = vp.TopLeftY;
	mScreenViewport.Width = vp.Width;

	mCommandList->RSSetViewports(1, &mScreenViewport);
}

void DXRHI::ResetScissorRects(int NumRects, ScissorRect& sr)
{
	mScissorRect.bottom = sr.bottom;
	mScissorRect.top = sr.top;
	mScissorRect.left = sr.left;
	mScissorRect.right = sr.right;

	mCommandList->RSSetScissorRects(NumRects, &mScissorRect);

}

void DXRHI::ResourceBarrier()
{
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
}

void DXRHI::ClearRenderTargetView(Color mClearColor, int NumRects)
{
	float color[4];
	color[0] = mClearColor.r;
	color[1] = mClearColor.g;
	color[2] = mClearColor.b;
	color[3] = mClearColor.a;

	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), color, NumRects, nullptr);
}

void DXRHI::ClearDepthStencilView()
{
	auto mRenderPrimitiveManager = Engine::Get()->GetRenderer()->GetRenderPrimitiveManager();
	auto mDsvHeap = std::dynamic_pointer_cast<DXRHIResource_Heap>(mRenderPrimitiveManager->GetHeapByName("mDsvHeap"));
	auto mDsvHeaphandle = mDsvHeap->mDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	mCommandList->ClearDepthStencilView(mDsvHeaphandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
}

void DXRHI::OMSetRenderTargets()
{
	CommitShadowMap();

	auto mRenderPrimitiveManager = Engine::Get()->GetRenderer()->GetRenderPrimitiveManager();
	auto mCbvHeap = std::dynamic_pointer_cast<DXRHIResource_Heap>(mRenderPrimitiveManager->GetHeapByName("mCbvHeap"));
	auto mDsvHeap = std::dynamic_pointer_cast<DXRHIResource_Heap>(mRenderPrimitiveManager->GetHeapByName("mDsvHeap"));
	auto mDsvHeapHandle = mDsvHeap->mDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &mDsvHeapHandle);
	ID3D12DescriptorHeap* descriptorHeaps[] = { mCbvHeap->mDescriptorHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());
	mCommandList->SetPipelineState(mPSO.Get());
}

void DXRHI::CommitShadowMap()
{
	//-------------------------------------------------------------------------------
	auto mRenderPrimitiveManager = Engine::Get()->GetRenderer()->GetRenderPrimitiveManager();
	auto shadowResource = std::dynamic_pointer_cast<DXRHIResource_ShadowMap>(mRenderPrimitiveManager->mShadowMap);
	auto ShadowHandle = shadowResource->mhGpuSrv;
	auto mShadowSrvDescriptorHeap = std::dynamic_pointer_cast<DXRHIResource_Heap>(mRenderPrimitiveManager->GetHeapByName("mShadowSrvDescriptorHeap"));


	ID3D12DescriptorHeap* descriptorHeaps[] = { mShadowSrvDescriptorHeap->mDescriptorHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	mCommandList->SetPipelineState(mShadowMapPSO.Get());
	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());
	//Shader Texture gShadowMap
	//TextureHandle.Offset(1000, md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	mCommandList->SetGraphicsRootDescriptorTable(4, ShadowHandle);
	//-------------------------------------------------------------------------------
}

void DXRHI::DrawActor(int ActorIndex,int TextureIndex)
{
	auto mRenderPrimitiveManager = Engine::Get()->GetRenderer()->GetRenderPrimitiveManager();
	
	auto mCbvHeap = std::dynamic_pointer_cast<DXRHIResource_Heap>(mRenderPrimitiveManager->GetHeapByName("mCbvHeap"));

	auto DrawMeshName = Engine::Get()->GetAssetManager()->GetMapActorInfo()->MeshNameArray[ActorIndex];
	DrawMeshName.erase(DrawMeshName.size() - 1, 1);
	auto testGeoIndex = mRenderPrimitiveManager->GetMeshKeyByName(DrawMeshName);
	auto mVBIB = std::dynamic_pointer_cast<DXRHIResource_MeshBuffer>(mRenderPrimitiveManager->VBIBBuffers[testGeoIndex]);
	
	mCommandList->IASetVertexBuffers(0, 1, &mVBIB->GetVertexBufferView());
	mCommandList->IASetIndexBuffer(&mVBIB->GetIndexBufferView());
	mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	auto heapHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mCbvHeap->mDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	heapHandle.Offset(ActorIndex, md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	mCommandList->SetGraphicsRootDescriptorTable(0, heapHandle);

	//贴图的Size要记得改下面的Offset
	auto GPUHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mCbvHeap->mDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	GPUHandle.Offset(Engine::Get()->GetAssetManager()->GetMapActorInfo()->Size()+ TextureIndex, md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	mCommandList->SetGraphicsRootDescriptorTable(1, GPUHandle);

	//单独的Nromal图
	auto TextureHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mCbvHeap->mDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	TextureHandle.Offset(1000, md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	mCommandList->SetGraphicsRootDescriptorTable(3, TextureHandle);



	////Shader Texture gShadowMap
	//auto shadowResource = std::dynamic_pointer_cast<DXRHIResource_ShadowMap>(mRHIResourceManager->mShadowMap);
	//auto ShadowHandle = shadowResource->mhGpuSrv;
	////TextureHandle.Offset(1000, md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	//mCommandList->SetGraphicsRootDescriptorTable(4, ShadowHandle);

	mCommandList->DrawIndexedInstanced(mVBIB->DrawArgs[Engine::Get()->GetAssetManager()->GetMapActorInfo()->MeshNameArray[ActorIndex]].IndexCount, 1, 0, 0, 0);
	
}

void DXRHI::DrawFinal()
{
	auto mCamera = Engine::Get()->GetRenderer()->GetCamera();
	mCommandList->SetGraphicsRoot32BitConstants(2, 3, &mCamera->GetPosition(), 0);

	// Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	ExecuteCommandList();
	WaitCommandComplete();
	// swap the back and front buffers
	ThrowIfFailed(mSwapChain->Present(0, 0));
	auto mRendertarget = Engine::Get()->GetRenderer()->GetRenderPrimitiveManager()->mRenderTarget;
	auto SwapChainBufferCount = std::dynamic_pointer_cast<DXRHIResource_RenderTarget>(mRendertarget)->GetSwapChainBufferCount();
	auto mCurrBackBufferIndex = std::dynamic_pointer_cast<DXRHIResource_RenderTarget>(mRendertarget)->GetCurrBackBufferIndex();
	std::dynamic_pointer_cast<DXRHIResource_RenderTarget>(mRendertarget)->SetCurrBackBufferIndex(mCurrBackBufferIndex = (mCurrBackBufferIndex + 1) % SwapChainBufferCount);


	WaitCommandComplete();
}

void DXRHI::FlushCommandQueue()
{
	// Advance the fence value to mark commands up to this fence point.
	mCurrentFence++;

	// Add an instruction to the command queue to set a new fence point.  Because we 
	// are on the GPU timeline, the new fence point won't be set until the GPU finishes
	// processing all the commands prior to this Signal().
	ThrowIfFailed(mCommandQueue->Signal(mFence.Get(), mCurrentFence));

	// Wait until the GPU has completed commands up to this fence point.
	if (mFence->GetCompletedValue() < mCurrentFence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);

		// Fire event when GPU hits current fence.  
		ThrowIfFailed(mFence->SetEventOnCompletion(mCurrentFence, eventHandle));

		// Wait until the GPU hits current fence event is fired.
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

void DXRHI::SetSwapChain()
{
	auto mRendertarget = Engine::Get()->GetRenderer()->GetRenderPrimitiveManager()->mRenderTarget;
	auto SwapChainBufferCount = std::dynamic_pointer_cast<DXRHIResource_RenderTarget>(mRendertarget)->GetSwapChainBufferCount();

	// Release the previous swapchain we will be recreating.
	mSwapChain.Reset();

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = mClientWidth;
	sd.BufferDesc.Height = mClientHeight;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = mBackBufferFormat;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	sd.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = SwapChainBufferCount;
	sd.OutputWindow = std::dynamic_pointer_cast<Win32Window>(Engine::Get()->GetWindow())->GetHWND();
	//sd.OutputWindow = dynamic_cast<Win32Window*>(Engine::Get()->GetWindow())->GetHWND();

	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	//auto mDXRHIFactory = std::dynamic_pointer_cast<DXRHIFactory>(mRHIResourceManager->mRHIFactory);
	// Note: Swap chain uses queue to perform flush.
	ThrowIfFailed(mdxgiFactory ->CreateSwapChain(
		mCommandQueue.Get(),
		&sd,
		mSwapChain.GetAddressOf()));
}
//
//void DXRHI::SetRtvAndDsvDescriptorHeaps()
//{
//	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
//	rtvHeapDesc.NumDescriptors = SwapChainBufferCount;
//	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
//	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
//	rtvHeapDesc.NodeMask = 0;
//
//	auto dxDevice = std::dynamic_pointer_cast<DXRHIDevice>(mRHIResourceManager->mRHIDevice);
//	ThrowIfFailed(dxDevice->md3dDevice->CreateDescriptorHeap(
//		&rtvHeapDesc, IID_PPV_ARGS(mRtvHeap.GetAddressOf())));
//
//	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
//	dsvHeapDesc.NumDescriptors = 1;
//	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
//	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
//	dsvHeapDesc.NodeMask = 0;
//	ThrowIfFailed(dxDevice->md3dDevice->CreateDescriptorHeap(
//		&dsvHeapDesc, IID_PPV_ARGS(mDsvHeap.GetAddressOf())));
//}

//void DXRHI::LoadAsset()//将外部导入的Actor信息赋给VS
//{
//	std::string StaticMeshPath;
//	std::set<std::string> StaticMeshs;//用于去重
//
//	StaticMesh mesh;
//
//	Engine::Get()->GetAssetManager()->GetGeometryLibrary()->resize(Engine::Get()->GetAssetManager()->GetMapActorInfo()->Size());
//
//	Vertex vertice;
//	UINT vbByteSize;
//	UINT ibByteSize;
//
//	//循环加入MeshGeometry
//	for (int i = 0; i < Engine::Get()->GetAssetManager()->Geos.size(); i++)
//	{
//		std::vector<Vertex> vertices;
//		std::vector<int32_t> indices;
//		//相同的StaticMesh不用重复建立
//		auto check = StaticMeshs.find(Engine::Get()->GetAssetManager()->GetMapActorInfo()->MeshNameArray[i]);
//		if (check == StaticMeshs.end()) {
//			StaticMeshs.insert(Engine::Get()->GetAssetManager()->GetMapActorInfo()->MeshNameArray[i]);
//			Engine::Get()->GetAssetManager()->GetMapofGeosMesh()->insert(std::pair<int, std::string>(i, Engine::Get()->GetAssetManager()->GetMapActorInfo()->MeshNameArray[i]));
//		}
//		else { continue; }
//
//		//读取mesh信息
//		StaticMeshPath = "SplitMesh/" + Engine::Get()->GetAssetManager()->GetMapActorInfo()->MeshNameArray[i];
//		StaticMeshPath.erase(StaticMeshPath.length() - 1);
//		StaticMeshPath += ".bat";
//		mesh.LoadStaticMeshFromBat(StaticMeshPath);
//
//		if (mesh.MeshInfo.MeshVertexInfo.size() < 3) { continue; }//没有StaticMesh就不读取
//		//--------------------------------------------------------------------------------
//
//		for (int j = 0; j < mesh.MeshInfo.MeshVertexInfo.size(); j++)
//		{
//			vertice.Pos = mesh.MeshInfo.MeshVertexInfo[j];
//			vertice.Color = {
//				float(j) / mesh.MeshInfo.MeshVertexInfo.size(),
//				float(j) / mesh.MeshInfo.MeshVertexInfo.size(),
//				float(j) / mesh.MeshInfo.MeshVertexInfo.size(),
//				1 };//初始化赋值为黑白色
//			vertice.Normal = mesh.MeshInfo.MeshVertexNormalInfo[j];
//			vertice.TexCoord = mesh.MeshInfo.MeshTexCoord[j];
//
//			vertices.push_back(vertice);
//		}
//		indices = mesh.MeshInfo.MeshIndexInfo;
//
//		vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
//		ibByteSize = (UINT)indices.size() * sizeof(std::uint32_t);
//
//		Engine::Get()->GetAssetManager()->Geos[i] = std::make_unique<MeshGeometry>();
//		Engine::Get()->GetAssetManager()->Geos[i]->Name = mesh.getMeshName();
//
//		ThrowIfFailed(D3DCreateBlob(vbByteSize, &Engine::Get()->GetAssetManager()->Geos[i]->VertexBufferCPU));
//		CopyMemory(Engine::Get()->GetAssetManager()->Geos[i]->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);
//
//		ThrowIfFailed(D3DCreateBlob(ibByteSize, &Engine::Get()->GetAssetManager()->Geos[i]->IndexBufferCPU));
//		CopyMemory(Engine::Get()->GetAssetManager()->Geos[i]->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);
//
//		Engine::Get()->GetAssetManager()->Geos[i]->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
//			mCommandList.Get(), vertices.data(), vbByteSize, Engine::Get()->GetAssetManager()->Geos[i]->VertexBufferUploader);
//
//		Engine::Get()->GetAssetManager()->Geos[i]->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
//			mCommandList.Get(), indices.data(), ibByteSize, Engine::Get()->GetAssetManager()->Geos[i]->IndexBufferUploader);
//
//		Engine::Get()->GetAssetManager()->Geos[i]->VertexByteStride = sizeof(Vertex);
//		Engine::Get()->GetAssetManager()->Geos[i]->VertexBufferByteSize = vbByteSize;
//		Engine::Get()->GetAssetManager()->Geos[i]->IndexFormat = DXGI_FORMAT_R32_UINT;
//		Engine::Get()->GetAssetManager()->Geos[i]->IndexBufferByteSize = ibByteSize;
//
//		SubmeshGeometry submesh;
//		submesh.IndexCount = (UINT)indices.size();
//		submesh.StartIndexLocation = 0;
//		submesh.BaseVertexLocation = 0;
//
//		Engine::Get()->GetAssetManager()->Geos[i]->DrawArgs[Engine::Get()->GetAssetManager()->GetMapActorInfo()->MeshNameArray[i]] = submesh;
//	}
//}

void DXRHI::CalculateFrameStats()
{
	auto mCamera = Engine::Get()->GetRenderer()->GetCamera();
	// Code computes the average frames per second, and also the 
	// average time it takes to render one frame.  These stats 
	// are appended to the window caption bar.

	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	// Compute averages over one second period.
	if ((Engine::Get()->gt.TotalTime() - timeElapsed) >= 1.0f)
	{
		float fps = (float)frameCnt; // fps = frameCnt / 1
		float mspf = 1000.0f / fps;

		std::wstring fpsStr = std::to_wstring(fps);
		std::wstring mspfStr = std::to_wstring(mspf);

		//std::wstring mCameraX = std::to_wstring(mCamera.mPosition.x);
		//std::wstring mCameraY = std::to_wstring(mCamera.mPosition.y);
		//std::wstring mCameraZ = std::to_wstring(mCamera.mPosition.z);
		//std::wstring totaltime = std::to_wstring(Engine::Get()->GetGameTimer()->TotalTime());

		std::wstring mCameraX = std::to_wstring(mCamera->mPosition.x);
		std::wstring mCameraY = std::to_wstring(mCamera->mPosition.y);
		std::wstring mCameraZ = std::to_wstring(mCamera->mPosition.z);
		std::wstring totaltime = std::to_wstring(Engine::Get()->gt.TotalTime());

		std::wstring windowText = Engine::Get()->GetWindow()->mMainWndCaption +
			L"    fps: " + fpsStr +
			L"   mspf: " + mspfStr +
			L"   Location: " +
			L"   x: " + mCameraX +
			L"   y: " + mCameraY +
			L"   z: " + mCameraZ +
			L"   -TotalTime:" + totaltime
			;

		SetWindowText(std::dynamic_pointer_cast<Win32Window>(Engine::Get()->GetWindow())->mhMainWnd, windowText.c_str());

		// Reset for next average.
		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}

//void DXRHI::BuildDescriptorHeaps()
//{
//	SetDescriptorHeaps();
//}

void DXRHI::SetDescriptorHeaps()
{
	mObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(md3dDevice.Get(), Engine::Get()->GetAssetManager()->GetMapActorInfo()->Size(), true);

	UINT DescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	UINT ConstantbufferSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	UINT ShaderResourceSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// Offset to the ith object constant buffer in the buffer.

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;

	//循环开辟Heap空间
	auto mRenderPrimitiveManager = Engine::Get()->GetRenderer()->GetRenderPrimitiveManager();
	for (int i = 0; i < Engine::Get()->GetAssetManager()->GetMapActorInfo()->Size(); i++)
	{
		auto mCbvHeap = std::dynamic_pointer_cast<DXRHIResource_Heap>(mRenderPrimitiveManager->GetHeapByName("mCbvHeap"));
		auto heapCPUHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mCbvHeap->mDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

		D3D12_GPU_VIRTUAL_ADDRESS cbAddress = mObjectCB->Resource()->GetGPUVirtualAddress();
		heapCPUHandle.Offset(i, DescriptorSize);
		cbAddress += i * ConstantbufferSize;
		cbvDesc.BufferLocation = cbAddress;
		cbvDesc.SizeInBytes = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
		md3dDevice->CreateConstantBufferView(&cbvDesc, heapCPUHandle);
	}

	for (int srvIndex = 0; srvIndex < Engine::Get()->GetMaterialSystem()->GetTextureNum(); srvIndex++)
	{
		auto mCbvHeap = std::dynamic_pointer_cast<DXRHIResource_Heap>(mRenderPrimitiveManager->GetHeapByName("mCbvHeap"));
		auto heapCPUHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mCbvHeap->mDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

		UINT HandleOffsetNum = srvIndex + Engine::Get()->GetAssetManager()->GetMapActorInfo()->Size();//因为是在偏移了上面的CBV地址之后再做的偏移，所以这里要加上之前CBV已经偏移过的数量
		heapCPUHandle.Offset(HandleOffsetNum, ShaderResourceSize);

		auto testTextureResource = Engine::Get()->GetMaterialSystem()->GetTexture()->Resource;

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = testTextureResource->GetDesc().Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = testTextureResource->GetDesc().MipLevels;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		md3dDevice->CreateShaderResourceView(testTextureResource.Get(), &srvDesc, heapCPUHandle);

		auto TextureHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mCbvHeap->mDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		auto testNromalTexture = Engine::Get()->GetMaterialSystem()->mTextureNormal->Resource;
		TextureHandle.Offset(1000, ShaderResourceSize);

		D3D12_SHADER_RESOURCE_VIEW_DESC srv2Desc = {};
		srv2Desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srv2Desc.Format = testNromalTexture->GetDesc().Format;
		srv2Desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srv2Desc.Texture2D.MostDetailedMip = 0;
		srv2Desc.Texture2D.MipLevels = testNromalTexture->GetDesc().MipLevels;
		srv2Desc.Texture2D.ResourceMinLODClamp = 0.0f;
		md3dDevice->CreateShaderResourceView(testNromalTexture.Get(), &srv2Desc, TextureHandle);
	}
}

void DXRHI::BuildRootSignature()
{
	// Shader programs typically require resources as input (constant buffers,
	// textures, samplers).  The root signature defines the resources the shader
	// programs expect.  If we think of the shader programs as a function, and
	// the input resources as function parameters, then the root signature can be
	// thought of as defining the function signature.  

	// Root parameter can be a table, root descriptor or root constants.

	CD3DX12_ROOT_PARAMETER slotRootParameter[5];

	// Create a single descriptor table of CBVs.
	CD3DX12_DESCRIPTOR_RANGE cbvTable;
	CD3DX12_DESCRIPTOR_RANGE srvTable;
	CD3DX12_DESCRIPTOR_RANGE srvTable2;
	CD3DX12_DESCRIPTOR_RANGE srvTable3;
	cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

	srvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	slotRootParameter[1].InitAsDescriptorTable(1, &srvTable);

	slotRootParameter[2].InitAsConstants(3, 1);

	srvTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
	slotRootParameter[3].InitAsDescriptorTable(1, &srvTable2);

	srvTable3.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);
	slotRootParameter[4].InitAsDescriptorTable(1, &srvTable3);

	auto staticSamplers = GetStaticSamplers();	//获得静态采样器集合

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(5, slotRootParameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(md3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&mRootSignature)));
}

void DXRHI::BuildShadersAndInputLayout()
{
	HRESULT hr = S_OK;

	mvsByteCode = d3dUtil::CompileShader(L"Shaders\\color.hlsl", nullptr, "VS", "vs_5_0");
	mpsByteCode = d3dUtil::CompileShader(L"Shaders\\color.hlsl", nullptr, "PS", "ps_5_0");

	mInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void DXRHI::BuildPSO()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	psoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
	psoDesc.pRootSignature = mRootSignature.Get();
	psoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mvsByteCode->GetBufferPointer()),
		mvsByteCode->GetBufferSize()
	};
	psoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mpsByteCode->GetBufferPointer()),
		mpsByteCode->GetBufferSize()
	};

	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.FrontCounterClockwise = true;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = mBackBufferFormat;
	psoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	psoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	psoDesc.DSVFormat = mDepthStencilFormat;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPSO)));
}

ID3D12Resource* DXRHI::CurrentBackBuffer() const
{
	auto mRendertarget = Engine::Get()->GetRenderer()->GetRenderPrimitiveManager()->mRenderTarget;
	auto mSwapChainBuffer = std::dynamic_pointer_cast<DXRHIResource_RenderTarget>(mRendertarget)->mSwapChainBuffer;
	auto mCurrBackBufferIndex = std::dynamic_pointer_cast<DXRHIResource_RenderTarget>(mRendertarget)->GetCurrBackBufferIndex();

	return mSwapChainBuffer[mCurrBackBufferIndex].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE DXRHI::CurrentBackBufferView() const
{
	auto mRenderPrimitiveManager = Engine::Get()->GetRenderer()->GetRenderPrimitiveManager();
	auto mRtvHeap = std::dynamic_pointer_cast<DXRHIResource_Heap>(mRenderPrimitiveManager->GetHeapByName("mRtvHeap"));
	auto mRendertarget = Engine::Get()->GetRenderer()->GetRenderPrimitiveManager()->mRenderTarget;
	auto mCurrBackBufferIndex = std::dynamic_pointer_cast<DXRHIResource_RenderTarget>(mRendertarget)->GetCurrBackBufferIndex();

	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		mRtvHeap->mDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		mCurrBackBufferIndex,
		mRtvDescriptorSize);
}

//D3D12_CPU_DESCRIPTOR_HANDLE DXRHI::DepthStencilView()
//{
//	auto mRenderPrimitiveManager = Engine::Get()->GetRenderer()->GetRenderPrimitiveManager();
//	auto mDsvHeap = std::dynamic_pointer_cast<DXRHIResource_Heap>(mRenderPrimitiveManager->GetHeapByName("mDsvHeap"));
//	return mDsvHeap->mDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
//}

void DXRHI::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(std::dynamic_pointer_cast<Win32Window>(Engine::Get()->GetWindow())->GetHWND());
	//SetCapture(dynamic_cast<Win32Window*>(Engine::Get()->GetWindow())->GetHWND());
}

void DXRHI::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void DXRHI::OnMouseMove(WPARAM btnState, int x, int y)
{
	auto mCamera = Engine::Get()->GetRenderer()->GetCamera();
	if ((btnState & MK_RBUTTON) != 0) {
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

		//mCamera.Pitch(dy);
		//mCamera.Yaw(dx);
		mCamera->Pitch(dy);
		mCamera->Yaw(dx);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> DXRHI::GetStaticSamplers()
{
	// Applications usually only need a handful of samplers.  So just define them all up front
	// and keep them available as part of the root signature.  

	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
		0.0f,                             // mipLODBias
		8);                               // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
		0.0f,                              // mipLODBias
		8);                                // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC shadow(
		6, // shaderRegister
		D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
		0.0f,                               // mipLODBias
		16,                                 // maxAnisotropy
		D3D12_COMPARISON_FUNC_LESS_EQUAL,
		D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK);

	return {
		pointWrap, pointClamp,
		linearWrap, linearClamp,
		anisotropicWrap, anisotropicClamp,
		shadow };
}
