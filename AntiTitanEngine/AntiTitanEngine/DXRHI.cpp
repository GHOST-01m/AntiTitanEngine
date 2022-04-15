#include "stdafx.h"
#include "DXRHI.h"

Microsoft::WRL::ComPtr<ID3D12Device> DXRHI::Getd3dDevice() {
	return md3dDevice;
};

Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> DXRHI::GetCommandList(){
	return mCommandList;
}

bool DXRHI::Get4xMsaaState() const{
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
	//CreateSwapChain();
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
	//BuildPSO();

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
	CreateSwapChain();
	//InitDX_CreateSwapChain();
	//InitDX_CreateRtvAndDsvDescriptorHeaps();

	//--------------------------------------------------------------------------------
	//OnResize();//这里曾经有一个OnResize

	// Reset the command list to prep for initialization commands.
	//ResetCommandList();
}

void DXRHI::OpenDebugLayer()
{
#if defined(DEBUG) || defined(_DEBUG) 
	// Enable the D3D12 debug layer.
	{
		ComPtr<ID3D12Debug> debugController;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();
	}
#endif
}

std::shared_ptr<Primitive_Heap> DXRHI::CreateDescriptorHeap(std::string heapName,int NumDescriptors, int mHeapType,int mFlag)
{
	std::shared_ptr<Primitive_Heap> heap = std::make_shared<DXPrimitive_Heap>();
	auto mHeap = std::dynamic_pointer_cast<DXPrimitive_Heap>(heap);
	mHeap->name = heapName;
	D3D12_DESCRIPTOR_HEAP_TYPE heapType = D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
	D3D12_DESCRIPTOR_HEAP_FLAGS heapFlag = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	switch (mHeapType)
	{
	case 0:  heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;	break;
	case 1:  heapType = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;	break;
	case 2:  heapType = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;	break;
	case 3:  heapType = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;	break;
	case 4:  heapType = D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;	break;
	assert(0);
	break;
	}

	switch (mFlag)
	{
	case NONE:  heapFlag = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;			break;
	case SHADER_VISIBLE:  heapFlag = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;	break;
	assert(0);
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

void DXRHI::CreateSwapChain() {
	//auto SwapChainBufferCount = std::dynamic_pointer_cast<DXRHIResource_RenderTarget>(rendertarget)->GetSwapChainBufferCount();

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
	sd.BufferCount = 2;
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

DXGI_FORMAT DXRHI::SwitchFormat(int type)
{
	switch (type)
	{
	case R8G8B8A8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM; break;
	case R16G16B16A16_FLOAT:return DXGI_FORMAT_R16G16B16A16_FLOAT; break;
	case D24_UNORM_S8_UINT: return DXGI_FORMAT_D24_UNORM_S8_UINT; break;
		assert(0);
		break;
	}
	return DXGI_FORMAT_UNKNOWN;
}

D3D12_RESOURCE_DIMENSION DXRHI::SwitchDimension(int resourceDimension)
{
	switch (resourceDimension) {
	case UNKNOWN:	return  D3D12_RESOURCE_DIMENSION_UNKNOWN; break;
	case BUFFER:	return  D3D12_RESOURCE_DIMENSION_BUFFER; break;
	case TEXTURE1D:	return D3D12_RESOURCE_DIMENSION_TEXTURE1D; break;
	case TEXTURE2D:	return D3D12_RESOURCE_DIMENSION_TEXTURE2D; break;
	case TEXTURE3D:	return D3D12_RESOURCE_DIMENSION_TEXTURE3D; break;
		assert(0);
		break;
	}
	return D3D12_RESOURCE_DIMENSION_UNKNOWN;
}

D3D12_RESOURCE_FLAGS DXRHI::SwitchFlags(int ResourceFormat)
{
	switch (ResourceFormat)
	{
	case R8G8B8A8_UNORM: return D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET; break;
	case R16G16B16A16_FLOAT:return D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET; break;
	case D24_UNORM_S8_UINT: return D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; break;
		assert(0);
		break;
	}
	return D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
}

D3D12_RESOURCE_STATES DXRHI::SwitchInitialResourceStateType(int initialResourceStateType)
{
	switch (initialResourceStateType)
	{
	case STATE_COMMON:           return D3D12_RESOURCE_STATE_COMMON; break;
	case STATE_DEPTH_WRITE:      return D3D12_RESOURCE_STATE_DEPTH_WRITE; break;
	case STATE_RENDER_TARGET:    return D3D12_RESOURCE_STATE_RENDER_TARGET; break;
	case STATE_PRESENT:          return D3D12_RESOURCE_STATE_PRESENT; break;
	case STATE_GENERIC_READ:     return D3D12_RESOURCE_STATE_GENERIC_READ; break;
		//这里还要加两个shadow要用的，这个type的注释记得加到RHI里面
		assert(0); break;
	}
	return D3D12_RESOURCE_STATE_COMMON;
}

void DXRHI::CreateResource(
	std::shared_ptr<DXPrimitive_GPUResource> gpuResource, 
	int ResourceFormat, int resourceDimension, int ResourceTnitStateType,
	int Width, int Height,bool UseClearColor)
{
	D3D12_RESOURCE_DESC ResourceDesc;
	ZeroMemory(&ResourceDesc, sizeof(D3D12_RESOURCE_DESC));

	ResourceDesc.Dimension = SwitchDimension(resourceDimension);
	ResourceDesc.Alignment = 0;
	ResourceDesc.Width = int(Width);
	ResourceDesc.Height = int(Height);
	ResourceDesc.DepthOrArraySize = 1;
	ResourceDesc.MipLevels = 1;
	ResourceDesc.Format = SwitchFormat(ResourceFormat);
	ResourceDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	ResourceDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	ResourceDesc.Flags = SwitchFlags(ResourceFormat);//这里可以重新写

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = SwitchFormat(ResourceFormat);
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;

	if (UseClearColor)
	{
		optClear.Color[0] = 0;
		optClear.Color[1] = 0;
		optClear.Color[2] = 0;
		optClear.Color[3] = 0;
	}


	//设置资源初始状态==================================================================
	D3D12_RESOURCE_STATES InitStateType;
	InitStateType = SwitchInitialResourceStateType(ResourceTnitStateType);

	ThrowIfFailed(md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&ResourceDesc,
		InitStateType,
		&optClear,
		//IID_PPV_ARGS(Rendertarget->mDepthStencilBuffer.GetAddressOf())));
		IID_PPV_ARGS(gpuResource->mResource.GetAddressOf())));

	gpuResource->currentType = InitStateType;

}

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

void DXRHI::LoadDDSTextureToResource(std::wstring Path,std::shared_ptr<Primitive_Texture>texture)
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
//
//void DXRHI::BuildShadow()
//{
//	auto mRenderPrimitiveManager = Engine::Get()->GetRenderer()->GetRenderPrimitiveManager();
//	auto shadowResource = std::dynamic_pointer_cast<DXRHIResource_ShadowMap>(mRenderPrimitiveManager->mShadowMap);
//	auto DXShader = std::dynamic_pointer_cast<DXRHIResource_Shader> (mRenderPrimitiveManager->mShader);
//
//	shadowResource->mViewport = { 0.0f, 0.0f, (float)shadowResource->mWidth, (float)shadowResource->mHeight, 0.0f, 1.0f };
//	shadowResource->mScissorRect = { 0, 0, (int)shadowResource->mWidth, (int)shadowResource->mHeight };
//
//	//ShadowMap内CpuSrv,GpuSrv指针赋值,创建一个ShadowSrvHeap
//	auto mShadowSrvDescriptorHeap = std::dynamic_pointer_cast<DXRHIResource_Heap>(mRenderPrimitiveManager->GetHeapByName("mShadowSrvDescriptorHeap"));
//	shadowResource->mhCpuSrv = CD3DX12_CPU_DESCRIPTOR_HANDLE(mShadowSrvDescriptorHeap->mDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
//	shadowResource->mhGpuSrv = CD3DX12_GPU_DESCRIPTOR_HANDLE(mShadowSrvDescriptorHeap->mDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
//
//	//ShadowMap内CpuDsv指针赋值,创建一个ShadowDsvHeap
//	auto mShadowDsvDescriptorHeap = std::dynamic_pointer_cast<DXRHIResource_Heap>(mRenderPrimitiveManager->GetHeapByName("mShadowDsvDescriptorHeap"));
//	shadowResource->mhCpuDsv = CD3DX12_CPU_DESCRIPTOR_HANDLE(mShadowDsvDescriptorHeap->mDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
//
//	//一个Resource
//	D3D12_RESOURCE_DESC texDesc;
//	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
//	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
//	texDesc.Alignment = 0;
//	texDesc.Width = shadowResource->mWidth;
//	texDesc.Height = shadowResource->mHeight;
//	texDesc.DepthOrArraySize = 1;
//	texDesc.MipLevels = 1;
//	texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
//	texDesc.SampleDesc.Count = 1;//像素的多重采样数
//	texDesc.SampleDesc.Quality = 0;
//	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
//	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
//
//	D3D12_CLEAR_VALUE optClear;
//	optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
//	optClear.DepthStencil.Depth = 1.0f;
//	optClear.DepthStencil.Stencil = 0;
//
//	ThrowIfFailed(md3dDevice->CreateCommittedResource(
//		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
//		D3D12_HEAP_FLAG_NONE,
//		&texDesc,
//		D3D12_RESOURCE_STATE_GENERIC_READ,
//		&optClear,
//		IID_PPV_ARGS(&shadowResource->mShadowResource)));
//
//	//两个View
//	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
//	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
//	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
//	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
//	srvDesc.Texture2D.MostDetailedMip = 0;
//	srvDesc.Texture2D.MipLevels = 1;
//	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
//	srvDesc.Texture2D.PlaneSlice = 0;
//	md3dDevice->CreateShaderResourceView(shadowResource->mShadowResource.Get(),&srvDesc, shadowResource->mhCpuSrv);
//
//	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
//	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
//	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
//	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
//	dsvDesc.Texture2D.MipSlice = 0;
//	md3dDevice->CreateDepthStencilView(shadowResource->mShadowResource.Get(), &dsvDesc, shadowResource->mhCpuDsv);
//}

std::shared_ptr<Primitive_Shader> DXRHI::CreateShader(std::string ShaderName, std::wstring ShaderPath)
{
	auto dxshader = std::make_shared<DXPrimitive_Shader>();
	dxshader->name = ShaderName;
	dxshader->mvsByteCode = d3dUtil::CompileShader(ShaderPath, nullptr, "VS", "vs_5_0");
	dxshader->mpsByteCode = d3dUtil::CompileShader(ShaderPath, nullptr, "PS", "ps_5_0");

	dxshader->mInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BITANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 60, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 76, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	return dxshader;
}
//
//void DXRHI::LoadMeshAndSetBuffer()
//{
//	auto mRenderPrimitiveManager = Engine::Get()->GetRenderer()->GetRenderPrimitiveManager();
//
//	std::string StaticMeshPath;
//	std::set<std::string> StaticMeshs;//用于去重
//	StaticMesh mesh;
//	Engine::Get()->GetAssetManager()->GetGeometryLibrary()->resize(Engine::Get()->GetAssetManager()->GetMapActorInfo()->Size());
//	Vertex vertice;
//	//mRHIResourceManager->VBIBBuffers.resize(Engine::Get()->GetAssetManager()->GetMapActorInfo()->Size());
//	int MeshNum=0;
//	//循环加入MeshGeometry
//	for (int i = 0; i < Engine::Get()->GetAssetManager()->Geos.size(); i++)
//	{
//		std::vector<Vertex> vertices;
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
//		std::shared_ptr<DXPrimitive_MeshBuffer> buffer=std::make_shared<DXPrimitive_MeshBuffer>();
//		buffer->indices = mesh.MeshInfo.MeshIndexInfo;
//		buffer->vertices = vertices;
//		buffer->MeshName = mesh.getMeshName();
//
//		//mRHIResourceManager->VBIBBuffers[MeshNum] = std::make_shared<DXRHIRessource_VBIBBuffer>();
//		
//		mRenderPrimitiveManager->VBIBBuffers.push_back(buffer);
//		std::string MeshName = Engine::Get()->GetAssetManager()->GetMapActorInfo()->MeshNameArray[i];
//		MeshName.erase(MeshName.length() - 1);
//
//		mRenderPrimitiveManager->MeshMap.insert(std::make_pair(MeshNum,MeshName));
//		MeshNum++;
//	}
//}
//
//void DXRHI::CreateMeshBuffers() {
//	auto mRenderPrimitiveManager = Engine::Get()->GetRenderer()->GetRenderPrimitiveManager();
//
//	for (int i = 0; i < mRenderPrimitiveManager->VBIBBuffers.size(); i++)
//	{
//		mRenderPrimitiveManager->VBIBBuffers;
//		mRenderPrimitiveManager->MeshMap;
//
//		auto VIBuffer = std::dynamic_pointer_cast<DXPrimitive_MeshBuffer>(mRenderPrimitiveManager->VBIBBuffers[i]);
//
//		UINT vbByteSize;
//		UINT ibByteSize;
//		vbByteSize = (UINT)VIBuffer->vertices.size() * sizeof(Vertex);
//		ibByteSize = (UINT)VIBuffer->indices.size() * sizeof(std::uint32_t);
//
//		VIBuffer->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), VIBuffer->vertices.data(), vbByteSize, VIBuffer->VertexBufferUploader);
//		VIBuffer->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), VIBuffer->indices.data(), ibByteSize, VIBuffer->IndexBufferUploader);
//
//		VIBuffer->VertexByteStride=sizeof(Vertex);
//		VIBuffer->VertexBufferByteSize = vbByteSize;
//		VIBuffer->IndexFormat=DXGI_FORMAT_R32_UINT;
//		VIBuffer->IndexBufferByteSize = ibByteSize;
//
//		SubmeshGeometry submesh;
//		submesh.IndexCount = (UINT)VIBuffer->indices.size();
//		submesh.StartIndexLocation = 0;
//		submesh.BaseVertexLocation = 0;
//		
//		VIBuffer->DrawArgs[VIBuffer->MeshName] = submesh;
//	}
//}

std::shared_ptr<Primitive_Pipeline> DXRHI::CreatePipeline(std::string pipelineName, std::shared_ptr<Primitive_Shader> shader,int NumRenderTargets, int RenderTargetType, bool isShadowPipeline)
{
	auto dxShader = std::dynamic_pointer_cast<DXPrimitive_Shader>(shader);
	auto pipeline = std::make_shared<DXPrimitive_Pipeline>();
	pipeline->pipelineName = pipelineName;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	psoDesc.InputLayout = { dxShader->mInputLayout.data(), (UINT)dxShader->mInputLayout.size() };
	psoDesc.pRootSignature = mRootSignature.Get();
	psoDesc.VS ={
		reinterpret_cast<BYTE*>(dxShader->mvsByteCode->GetBufferPointer()),
		dxShader->mvsByteCode->GetBufferSize()
	};
	if (NumRenderTargets>0)
	{
		psoDesc.PS = {
			reinterpret_cast<BYTE*>(dxShader->mpsByteCode->GetBufferPointer()),
			dxShader->mpsByteCode->GetBufferSize()
		};
	}

		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

	psoDesc.RasterizerState.FrontCounterClockwise = true;
	//psoDesc.RasterizerState.FrontCounterClockwise = PipDesc.rasterizeDesc.FrontCounterClockwise;

	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = NumRenderTargets;

	switch (RenderTargetType)
	{
	case RenderTargetFormat_R8G8B8A8_UNORM:	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; break;
	case RenderTargetFormat_UNKNOWN:	psoDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN; break;
	case RenderTargetFormat_R16G16B16A16_FLOAT:psoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT; break;
	case RenderTargetFormat_R32G32B32A32_FLOAT:	psoDesc.RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT; break;
		assert(0);
		break;
	}

	psoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	psoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	if (isShadowPipeline)
	{
		psoDesc.RasterizerState.DepthBias = 400000;//
		psoDesc.RasterizerState.DepthBiasClamp = 0.0f;//
		psoDesc.RasterizerState.SlopeScaledDepthBias = 1.0f;//
	}

	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&psoDesc, 
		IID_PPV_ARGS(&pipeline->mPipeline)));

	return pipeline;
}

std::shared_ptr<Primitive_RenderTarget> DXRHI::CreateRenderTarget(
	std::string RenderTargetName,
	int resourceDimension, int initialResourceStateType, int ResourceFormat,
	std::shared_ptr<Primitive_Heap>rtvHeap, int rtvOffset,
	std::shared_ptr<Primitive_Heap>srvHeap, int srvOffset,
	std::shared_ptr<Primitive_Heap>dsvHeap, int dsvOffset,
	bool rtvBindToSwapChain, int SwapChainCount, float Width, float Height)
{
	auto Rendertarget = std::make_shared<DXPrimitive_RenderTarget>();
	Rendertarget->mDSVResource = std::make_shared<DXPrimitive_GPUResource>();
	Rendertarget->name = RenderTargetName;
	Rendertarget->width = Width;
	Rendertarget->height = Height;

	auto dxRendertarget = std::dynamic_pointer_cast<DXPrimitive_RenderTarget>(Rendertarget);
	dxRendertarget->mSwapChainResource.resize(SwapChainCount);
	for (auto i=0;i< SwapChainCount;i++ ){
		dxRendertarget->mSwapChainResource[i]= std::make_shared<DXPrimitive_GPUResource>();
	}

	//设置heap==================================================================
	//如果传了heap进来就把对应的handle给填上
	if (rtvHeap != nullptr)
	{
		auto rtvSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		auto mRtvHeap = std::dynamic_pointer_cast<DXPrimitive_Heap>(rtvHeap);
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mRtvHeap->mDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		rtvHeapHandle.Offset(rtvOffset, rtvSize);
		for (auto i = 0; i < SwapChainCount; i++)
		{
			if (rtvBindToSwapChain)
			{
				ThrowIfFailed(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&std::dynamic_pointer_cast<DXPrimitive_GPUResource>(Rendertarget->mSwapChainResource[i])->mResource)));
				md3dDevice->CreateRenderTargetView(std::dynamic_pointer_cast<DXPrimitive_GPUResource>(Rendertarget->mSwapChainResource[i])->mResource.Get(), nullptr, rtvHeapHandle);
				std::dynamic_pointer_cast<DXPrimitive_GPUResource>(Rendertarget->mSwapChainResource[i])->rtvHeapOffsetLocation = rtvOffset;
				std::dynamic_pointer_cast<DXPrimitive_GPUResource>(Rendertarget->mSwapChainResource[i])->rtvSize = rtvSize;
				rtvHeapHandle.Offset(1, mRtvDescriptorSize);
				rtvOffset++;
			}
			else
			{
				CreateResource(
					std::dynamic_pointer_cast<DXPrimitive_GPUResource>(dxRendertarget->mSwapChainResource[i]),
					R16G16B16A16_FLOAT, resourceDimension,
					STATE_RENDER_TARGET,
					int(Width), int(Height),true);

				D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
				rtvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
				rtvDesc.Texture2D.PlaneSlice = 0;
				rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

				md3dDevice->CreateRenderTargetView(std::dynamic_pointer_cast<DXPrimitive_GPUResource>(Rendertarget->mSwapChainResource[i])->mResource.Get(), &rtvDesc, rtvHeapHandle);
				std::dynamic_pointer_cast<DXPrimitive_GPUResource>(Rendertarget->mSwapChainResource[i])->rtvHeapOffsetLocation = rtvOffset;
				std::dynamic_pointer_cast<DXPrimitive_GPUResource>(Rendertarget->mSwapChainResource[i])->rtvSize = rtvSize;

				rtvHeapHandle.Offset(1, mRtvDescriptorSize);
				rtvOffset++;
			}
		}
		Rendertarget->rtvHeapName = mRtvHeap->name;
		Rendertarget->mRtvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	if (dsvHeap != nullptr)
	{
		CreateResource(
			std::dynamic_pointer_cast<DXPrimitive_GPUResource>(Rendertarget->GetDSVResource()),
			ResourceFormat, resourceDimension,
			initialResourceStateType,
			int(Width), int(Height),false);

		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		//dsvDesc.Format = SwitchFormat(ResourceFormat);
		dsvDesc.Texture2D.MipSlice = 0;

		auto DsvSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

		auto mDsvHeap=std::dynamic_pointer_cast<DXPrimitive_Heap>(dsvHeap);
		Rendertarget->mhCpuDsvHandle = mDsvHeap->mDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		Rendertarget->mhCpuDsvHandle.Offset(dsvOffset, DsvSize);
		//ResourceTransition(Rendertarget->GetCommonResource(),1);
		md3dDevice->CreateDepthStencilView(std::dynamic_pointer_cast<DXPrimitive_GPUResource>(Rendertarget->GetDSVResource())->mResource.Get(), &dsvDesc, Rendertarget->mhCpuDsvHandle);
		std::dynamic_pointer_cast<DXPrimitive_GPUResource>(Rendertarget->GetDSVResource())->dsvHeapOffsetLocation = dsvOffset;
		std::dynamic_pointer_cast<DXPrimitive_GPUResource>(Rendertarget->GetDSVResource())->dsvSize = DsvSize;
	}

	if (srvHeap != nullptr)
	{
		auto srvSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		if (rtvHeap == nullptr){
			srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		}
		else{
			srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		}
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		srvDesc.Texture2D.PlaneSlice = 0;

		auto mSrvHeap = std::dynamic_pointer_cast<DXPrimitive_Heap>(srvHeap);
		Rendertarget->mhCpuSrvHandle = mSrvHeap->mDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		Rendertarget->mhCpuSrvHandle.Offset(srvOffset, srvSize);
		Rendertarget->mhGpuSrvHandle = mSrvHeap->mDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
		Rendertarget->mhGpuSrvHandle.Offset(srvOffset, srvSize);
		if (rtvHeap == nullptr)
		{
			md3dDevice->CreateShaderResourceView(std::dynamic_pointer_cast<DXPrimitive_GPUResource>(Rendertarget->GetDSVResource())->mResource.Get(), &srvDesc, Rendertarget->mhCpuSrvHandle);
			std::dynamic_pointer_cast<DXPrimitive_GPUResource>(Rendertarget->GetDSVResource())->srvSize = srvSize;
		}
		else {
			md3dDevice->CreateShaderResourceView(std::dynamic_pointer_cast<DXPrimitive_GPUResource>(dxRendertarget->mSwapChainResource[0])->mResource.Get(), &srvDesc, Rendertarget->mhCpuSrvHandle);
			std::dynamic_pointer_cast<DXPrimitive_GPUResource>(dxRendertarget->mSwapChainResource[0])->srvSize = srvSize;
		}
	}

	return Rendertarget;
}

std::shared_ptr<Primitive_MeshBuffer> DXRHI::CreateMeshBuffer(std::shared_ptr<StaticMesh> mesh)
{
	std::shared_ptr<Primitive_MeshBuffer> MeshBuffer = std::make_shared<DXPrimitive_MeshBuffer>();
	auto DXVIBuffer = std::dynamic_pointer_cast<DXPrimitive_MeshBuffer>(MeshBuffer);
	std::vector<Vertex> vertices;
	Vertex vertice;

	for (int meshIndex = 0; meshIndex < mesh->MeshInfo.MeshVertexInfo.size(); meshIndex++)
	{
		vertice.Pos = mesh->MeshInfo.MeshVertexInfo[meshIndex];
		vertice.Color = {
			float(meshIndex) / mesh->MeshInfo.MeshVertexInfo.size(),
			float(meshIndex) / mesh->MeshInfo.MeshVertexInfo.size(),
			float(meshIndex) / mesh->MeshInfo.MeshVertexInfo.size(),
			1 };//初始化赋值为黑白色
		vertice.Normal = mesh->MeshInfo.MeshVertexNormalInfo[meshIndex];
		vertice.Tangent= mesh->MeshInfo.MeshVertexTangentInfo[meshIndex];
		vertice.Bitangent= mesh->MeshInfo.MeshVertexBitangentInfo[meshIndex];
		vertice.TexCoord = mesh->MeshInfo.MeshTexCoord[meshIndex];

		vertices.push_back(vertice);
	}

	DXVIBuffer->MeshName = mesh->GetMeshName();
	DXVIBuffer->vertices = vertices;
	DXVIBuffer->indices = mesh->MeshInfo.MeshIndexInfo;

	//不确定有没有创建好了这个Meshbuffer，创建好之后在Render里把这个Mehsbuffer插进去

	UINT vbByteSize = (UINT)DXVIBuffer->vertices.size() * sizeof(Vertex);
	UINT ibByteSize = (UINT)DXVIBuffer->indices.size() * sizeof(std::uint32_t);

	DXVIBuffer->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), DXVIBuffer->vertices.data(), vbByteSize, DXVIBuffer->VertexBufferUploader);
	DXVIBuffer->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), DXVIBuffer->indices.data(), ibByteSize, DXVIBuffer->IndexBufferUploader);

	DXVIBuffer->VertexByteStride = sizeof(Vertex);
	DXVIBuffer->VertexBufferByteSize = vbByteSize;
	DXVIBuffer->IndexFormat = DXGI_FORMAT_R32_UINT;
	DXVIBuffer->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)DXVIBuffer->indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	DXVIBuffer->DrawArgs[DXVIBuffer->MeshName] = submesh;

	//mesh->meshBuffer = MeshBuffer;
	return MeshBuffer;
}

std::shared_ptr<Primitive_Texture> DXRHI::CreateTexture(std::string name, std::wstring Path,int currentHeapOffset)
{
	std::shared_ptr<Primitive_Texture> mTexture = std::make_shared<DXPrimitive_Texture>();

	auto texture = std::dynamic_pointer_cast<DXPrimitive_Texture>(mTexture);
	texture->Name = name;//"TestTexture";
	texture->Filename = Path;
	texture->heapOffset = currentHeapOffset;

	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(),
		texture->Filename.c_str(),
		texture->Resource,
		texture->UploadHeap));

	//=======================================================
	//Heap偏移根据上面给的来偏移
	//现在写的不确定有没有偏对
	auto mRenderPrimitiveManager = Engine::Get()->GetRenderer()->GetRenderPrimitiveManager();
	auto mCbvHeap = std::dynamic_pointer_cast<DXPrimitive_Heap>(mRenderPrimitiveManager->GetHeapByName("mCbvHeap"));
	auto heapCPUHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mCbvHeap->mDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	UINT ShaderResourceSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	heapCPUHandle.Offset(currentHeapOffset, ShaderResourceSize);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = texture->Resource->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = texture->Resource->GetDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	md3dDevice->CreateShaderResourceView(texture->Resource.Get(), &srvDesc, heapCPUHandle);
	//=======================================================
	return texture;
}

void DXRHI::ResourceTransition(std::shared_ptr<Primitive_GPUResource> myResource, int AfterStateType)
{
	auto dxGpuResource = std::dynamic_pointer_cast<DXPrimitive_GPUResource>(myResource);

	D3D12_RESOURCE_STATES afterType;
	switch (AfterStateType)
	{
	case 0:afterType = D3D12_RESOURCE_STATE_COMMON; break;
	case 1:afterType = D3D12_RESOURCE_STATE_DEPTH_WRITE; break;
	case 2:afterType = D3D12_RESOURCE_STATE_RENDER_TARGET; break;
	case 3:afterType = D3D12_RESOURCE_STATE_PRESENT; break;
	case 4:afterType = D3D12_RESOURCE_STATE_GENERIC_READ; break;

	assert(0);break;
	}

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		dxGpuResource->mResource.Get(),
		dxGpuResource->currentType,
		afterType));

	//转换之后把之前的状态设置为刚才设置好的状态
	std::dynamic_pointer_cast<DXPrimitive_GPUResource>(myResource)->currentType = afterType;
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
	ResizeSwapChain();

	//BuildRenderTarget();

	// Execute the resize commands.
	ExecuteCommandList();

	WaitCommandComplete();

	// Update the viewport transform to cover the client area.
	//SetScreenSetViewPort(0,0, static_cast<float>(Engine::Get()->GetWindow()->mClientWidth), static_cast<float>(Engine::Get()->GetWindow()->mClientHeight),0.0f,1.0f);

	//SetScissorRect(0, 0, Engine::Get()->GetWindow()->mClientWidth, Engine::Get()->GetWindow()->mClientHeight);
}

void DXRHI::resetRenderTarget()
{
	auto mRendertarget = std::dynamic_pointer_cast<DXPrimitive_RenderTarget>(Engine::Get()->GetRenderer()->GetRenderPrimitiveManager()->GetRenderTargetByName("baseRenderTarget"));
	auto mSwapChainBuffer = mRendertarget->mSwapChainResource;
	auto SwapChainBufferCount = mRendertarget->GetSwapChainBufferCount();

	for (int i = 0; i < SwapChainBufferCount; ++i) {
		std::dynamic_pointer_cast<DXPrimitive_GPUResource>(mSwapChainBuffer[i])->mResource.Reset();
	}
	std::dynamic_pointer_cast<DXPrimitive_GPUResource>(mRendertarget->GetDSVResource())->mResource.Reset();
	//mRendertarget->GetDepthStencilBuffer().Reset();
}

void DXRHI::ResizeSwapChain()
{
	auto mRendertarget = Engine::Get()->GetRenderer()->GetRenderPrimitiveManager()->GetRenderTargetByName("baseRenderTarget");
	auto SwapChainBufferCount = std::dynamic_pointer_cast<DXPrimitive_RenderTarget>(mRendertarget)->GetSwapChainBufferCount();

	ThrowIfFailed(mSwapChain->ResizeBuffers(
		SwapChainBufferCount,
		Engine::Get()->GetWindow()->mClientWidth,
		Engine::Get()->GetWindow()->mClientHeight,
		mBackBufferFormat,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	std::dynamic_pointer_cast<DXPrimitive_RenderTarget>(mRendertarget)->SetCurrBackBufferIndex(0);

}

void DXRHI::SetScreenSetViewPort(float Width, float Height)
{
	D3D12_VIEWPORT mScreenViewport;

	mScreenViewport.TopLeftX = 0;
	mScreenViewport.TopLeftY = 0;
	mScreenViewport.Width = Width;
	mScreenViewport.Height = Height;
	mScreenViewport.MinDepth = 0;
	mScreenViewport.MaxDepth = 1;

	mCommandList->RSSetViewports(1, &mScreenViewport);
};

void DXRHI::SetScissorRect(long Right, long Bottom)
{
	D3D12_RECT mScissorRect;
	mScissorRect = { 0, 0, Right, Bottom };
	mCommandList->RSSetScissorRects(1, &mScissorRect);
}
void DXRHI::CommitResourceToGPU(int elementIndex, ObjectConstants objConstants)
{
	mObjectCB->CopyData(elementIndex, objConstants);
}

void DXRHI::DrawReset()
{
	ThrowIfFailed(mDirectCmdListAlloc->Reset());
	auto mRenderPrimitiveManager = Engine::Get()->GetRenderer()->GetRenderPrimitiveManager();
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), 
		std::dynamic_pointer_cast<DXPrimitive_Pipeline>(mRenderPrimitiveManager->GetPipelineByName("basePipeline"))->mPipeline.Get()));
}

void DXRHI::ResourceBarrier()
{
	auto RPM = Engine::Get()->GetRenderer()->GetRenderPrimitiveManager();
	auto CurrentBuffer=std::dynamic_pointer_cast<DXPrimitive_RenderTarget>(RPM->GetRenderTargetByName("baseRenderTarget"))->GetCurrentSwapChainBuffer();
	auto CurrentBackBufferResource = std::dynamic_pointer_cast<DXPrimitive_GPUResource>(CurrentBuffer)->mResource;
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBufferResource.Get(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
}

void DXRHI::ClearRenderTargetView(std::shared_ptr<Primitive_RenderTarget>renderTarget,Color mClearColor, int NumRects)
{
	float color[4];
	color[0] = mClearColor.r;
	color[1] = mClearColor.g;
	color[2] = mClearColor.b;
	color[3] = mClearColor.a;

	auto dxRenderTarget=std::dynamic_pointer_cast<DXPrimitive_RenderTarget>(renderTarget);

	//mCommandList->ClearRenderTargetView(mCurrentBackBufferView, color, NumRects, nullptr);
	mCommandList->ClearRenderTargetView(dxRenderTarget->GetCurrentBackBufferCpuHandle(), color, NumRects, nullptr);
}

void DXRHI::ClearDepthStencilView(std::shared_ptr<Primitive_RenderTarget> renderTarget)
{
	mCommandList->ClearDepthStencilView(
		std::dynamic_pointer_cast<DXPrimitive_RenderTarget>(renderTarget)->mhCpuDsvHandle,
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
}

void DXRHI::ClearRenderTarget(std::shared_ptr<Primitive_RenderTarget> renderTarget,std::string heapName)
{
	float color[4];
	color[0] = 0;
	color[1] = 0;
	color[2] = 0;
	color[3] = 0;
	auto dxRenderTarget = std::dynamic_pointer_cast<DXPrimitive_RenderTarget>(renderTarget);
	auto resource = dxRenderTarget->GetCurrentSwapChainBuffer();
	auto rtvSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	auto dxResource = std::dynamic_pointer_cast<DXPrimitive_GPUResource>(resource);
	auto rtvHeap=Engine::Get()->GetRenderer()->GetRenderPrimitiveManager()->GetHeapByName(heapName);
	auto mRtvHeap = std::dynamic_pointer_cast<DXPrimitive_Heap>(rtvHeap);
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mRtvHeap->mDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	rtvHeapHandle.Offset(dxResource->rtvHeapOffsetLocation, rtvSize);

	//mCommandList->ClearRenderTargetView(mCurrentBackBufferView, color, NumRects, nullptr);
	mCommandList->ClearRenderTargetView(rtvHeapHandle, color, 0, nullptr);
}

void DXRHI::OMSetRenderTargets(std::shared_ptr<Primitive_RenderTarget>renderTarget)
{

	auto dxRenderTarget = std::dynamic_pointer_cast<DXPrimitive_RenderTarget>(renderTarget);
	if (dxRenderTarget->rtvHeapName != ""){
		mCommandList->OMSetRenderTargets(1, &dxRenderTarget->GetCurrentBackBufferCpuHandle(), true, &dxRenderTarget->mhCpuDsvHandle);
	}
	else{
		mCommandList->OMSetRenderTargets(0, nullptr, false, &dxRenderTarget->mhCpuDsvHandle);
	}
}

void DXRHI::SetDescriptorHeap(std::shared_ptr<Primitive_Heap> heap)
{
	//auto mRenderPrimitiveManager = Engine::Get()->GetRenderer()->GetRenderPrimitiveManager();
	//auto mCbvHeap = std::dynamic_pointer_cast<DXRHIResource_Heap>(mRenderPrimitiveManager->GetHeapByName("mCbvHeap"));

	ID3D12DescriptorHeap* descriptorHeaps[] = { std::dynamic_pointer_cast<DXPrimitive_Heap>(heap)->mDescriptorHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
}

void DXRHI::SetPipelineState(std::shared_ptr<Primitive_Pipeline> pipeline)
{
	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());
	mCommandList->SetPipelineState(
		std::dynamic_pointer_cast<DXPrimitive_Pipeline>(pipeline)->mPipeline.Get());
}

void DXRHI::CommitShaderParameters()
{
	//-------------------------------------------------------------------------------
	auto mRenderPrimitiveManager = Engine::Get()->GetRenderer()->GetRenderPrimitiveManager();

	SetDescriptorHeap(mRenderPrimitiveManager->GetHeapByName("mShadowSrvDescriptorHeap"));
	SetPipelineState(mRenderPrimitiveManager->GetPipelineByName("shadowPipeline"));

	auto dxshadowRT=std::dynamic_pointer_cast<DXPrimitive_RenderTarget>(mRenderPrimitiveManager->GetRenderTargetByName("shadowRenderTarget"));
	mCommandList->SetGraphicsRootDescriptorTable(4, dxshadowRT->mhGpuSrvHandle);
	//-------------------------------------------------------------------------------
}

void DXRHI::DrawMesh(int ActorIndex,int TextureIndex)
{
	auto mRenderPrimitiveManager = Engine::Get()->GetRenderer()->GetRenderPrimitiveManager();
	auto mCbvHeap = std::dynamic_pointer_cast<DXPrimitive_Heap>(mRenderPrimitiveManager->GetHeapByName("mCbvHeap"));
	auto DrawMeshName = Engine::Get()->GetAssetManager()->GetMapActorInfo()->MeshNameArray[ActorIndex];
	auto mesh = Engine::Get()->GetAssetManager()->GetStaticMeshByName(DrawMeshName);

	mCommandList->IASetVertexBuffers(0, 1, &std::dynamic_pointer_cast<DXPrimitive_MeshBuffer>(mesh->meshBuffer)->GetVertexBufferView());
	mCommandList->IASetIndexBuffer(&std::dynamic_pointer_cast<DXPrimitive_MeshBuffer>(mesh->meshBuffer)->GetIndexBufferView());
	mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	auto heapHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mCbvHeap->mDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	heapHandle.Offset(ActorIndex, md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	mCommandList->SetGraphicsRootDescriptorTable(0, heapHandle);

	//贴图
	auto GPUHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mCbvHeap->mDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	GPUHandle.Offset(mesh->material->GetTextureByName("TestTexture")->heapOffset, md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	mCommandList->SetGraphicsRootDescriptorTable(1, GPUHandle);

	//单独的Nromal图
	auto TextureHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mCbvHeap->mDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	TextureHandle.Offset(mesh->material->GetTextureByName("NormalTexture")->heapOffset, md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	mCommandList->SetGraphicsRootDescriptorTable(3, TextureHandle);

	mCommandList->DrawIndexedInstanced(UINT(mesh->MeshInfo.MeshIndexInfo.size()), 1, 0, 0, 0);
}

void DXRHI::DrawFinal()
{
	auto mRenderPrimitiveManager = Engine::Get()->GetRenderer()->GetRenderPrimitiveManager();
	auto baseRenderRarget = mRenderPrimitiveManager->GetRenderTargetByName("baseRenderTarget");
	auto currentBackBuffer = std::dynamic_pointer_cast<DXPrimitive_RenderTarget>(baseRenderRarget)->GetCurrentSwapChainBuffer();
	auto CurrentBackBufferResource = std::dynamic_pointer_cast<DXPrimitive_GPUResource>(currentBackBuffer)->mResource;

	auto mCamera = Engine::Get()->GetRenderer()->GetCamera();
	mCommandList->SetGraphicsRoot32BitConstants(2, 4, &mCamera->GetPosition(), 0);

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBufferResource.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	ExecuteCommandList();
	//WaitCommandComplete();
	// swap the back and front buffers
	ThrowIfFailed(mSwapChain->Present(0, 0));

	auto SwapChainBufferCount = std::dynamic_pointer_cast<DXPrimitive_RenderTarget>(baseRenderRarget)->GetSwapChainBufferCount();
	auto mCurrBackBufferIndex = std::dynamic_pointer_cast<DXPrimitive_RenderTarget>(baseRenderRarget)->GetCurrBackBufferIndex();
	std::dynamic_pointer_cast<DXPrimitive_RenderTarget>(baseRenderRarget)->SetCurrBackBufferIndex(mCurrBackBufferIndex = (mCurrBackBufferIndex + 1) % SwapChainBufferCount);

	WaitCommandComplete();
}

std::shared_ptr<Primitive_MeshBuffer> DXRHI::CreateTriangleMeshBuffer()
{
	std::shared_ptr<Primitive_MeshBuffer> MeshBuffer = std::make_shared<DXPrimitive_MeshBuffer>();
	auto DXVIBuffer = std::dynamic_pointer_cast<DXPrimitive_MeshBuffer>(MeshBuffer);
	std::vector<Vertex> vertices;
	Vertex vertice1;
	Vertex vertice2;
	Vertex vertice3;

	vertice1.Pos = { -1.0f, 1.0f,0.0f };
	vertice2.Pos = { -1.0f,-3.0f,0.0f };
	vertice3.Pos = {  3.0f, 1.0f,0.0f };

	vertices.push_back(vertice1);
	vertices.push_back(vertice2);
	vertices.push_back(vertice3);

	DXVIBuffer->MeshName = "Triangle";
	DXVIBuffer->vertices = vertices;
	DXVIBuffer->indices = { 0,1,2 };

	UINT vbByteSize = (UINT)DXVIBuffer->vertices.size() * sizeof(Vertex);
	UINT ibByteSize = (UINT)DXVIBuffer->indices.size() * sizeof(std::uint32_t);

	DXVIBuffer->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), DXVIBuffer->vertices.data(), vbByteSize, DXVIBuffer->VertexBufferUploader);
	DXVIBuffer->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), DXVIBuffer->indices.data(), ibByteSize, DXVIBuffer->IndexBufferUploader);

	DXVIBuffer->VertexByteStride = sizeof(Vertex);
	DXVIBuffer->VertexBufferByteSize = vbByteSize;
	DXVIBuffer->IndexFormat = DXGI_FORMAT_R32_UINT;
	DXVIBuffer->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)DXVIBuffer->indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	DXVIBuffer->DrawArgs[DXVIBuffer->MeshName] = submesh;

	return MeshBuffer;
}

void DXRHI::BuildCBVHeapForTirangle()
{
	mObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(md3dDevice.Get(), Engine::Get()->GetAssetManager()->GetMapActorInfo()->Size(), true);
	UINT DescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	UINT ConstantbufferSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	UINT ShaderResourceSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;

	auto mRenderPrimitiveManager = Engine::Get()->GetRenderer()->GetRenderPrimitiveManager();
	auto mCbvHeap = std::dynamic_pointer_cast<DXPrimitive_Heap>(mRenderPrimitiveManager->GetHeapByName("mCbvHeap"));
	auto heapCPUHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mCbvHeap->mDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	D3D12_GPU_VIRTUAL_ADDRESS cbAddress = mObjectCB->Resource()->GetGPUVirtualAddress();
	heapCPUHandle.Offset(40, DescriptorSize);
	cbAddress += 40 * ConstantbufferSize;
	cbvDesc.BufferLocation = cbAddress;
	cbvDesc.SizeInBytes = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	md3dDevice->CreateConstantBufferView(&cbvDesc, heapCPUHandle);
}

void DXRHI::BuildTriangleAndDraw(std::shared_ptr<Primitive_MeshBuffer> Triangle )
{
	//======================================================
	auto DXVIBuffer = std::dynamic_pointer_cast<DXPrimitive_MeshBuffer>(Triangle);
	//======================================================
	mCommandList->IASetVertexBuffers(0, 1, &DXVIBuffer->GetVertexBufferView());
	mCommandList->IASetIndexBuffer(&DXVIBuffer->GetIndexBufferView());
	mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	mCommandList->DrawIndexedInstanced(3, 1, 0, 0, 0);
}

void DXRHI::CommitShaderParameter_Table(int rootParameterIndex,std::shared_ptr<Primitive_RenderTarget> rendertarget)
{
	auto dxshadowRT = std::dynamic_pointer_cast<DXPrimitive_RenderTarget>(rendertarget);
	mCommandList->SetGraphicsRootDescriptorTable(rootParameterIndex, dxshadowRT->mhGpuSrvHandle);
}

void DXRHI::CommitShaderParameter_Constant(int rootParameterIndex, int numValue, int4 value)
{
	mCommandList->SetGraphicsRoot32BitConstants(rootParameterIndex, numValue, &value, 0);
}

void DXRHI::CommitShaderParameter_ConstantBuffer(int offset, std::shared_ptr<Primitive_Heap>heap)
{
	auto mCbvHeap = std::dynamic_pointer_cast<DXPrimitive_Heap>(heap);
	auto heapHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mCbvHeap->mDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	heapHandle.Offset(offset, md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	mCommandList->SetGraphicsRootDescriptorTable(0, heapHandle);
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
	auto mRendertarget = Engine::Get()->GetRenderer()->GetRenderPrimitiveManager()->GetRenderTargetByName("baseRenderTarget");
	auto SwapChainBufferCount = std::dynamic_pointer_cast<DXPrimitive_RenderTarget>(mRendertarget)->GetSwapChainBufferCount();

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

		std::wstring mCameraX = std::to_wstring(mCamera->mPosition.x);
		std::wstring mCameraY = std::to_wstring(mCamera->mPosition.y);
		std::wstring mCameraZ = std::to_wstring(mCamera->mPosition.z);
		std::wstring totaltime = std::to_wstring(Engine::Get()->gt.TotalTime());
		std::wstring CameraSpeed = std::to_wstring(Engine::Get()->GetRenderer()->GetCamera()->MoveSpeed);


		std::wstring windowText = Engine::Get()->GetWindow()->mMainWndCaption +
			L"    fps: " + fpsStr +
			L"   mspf: " + mspfStr +
			L"   Location: " +
			L"   x: " + mCameraX +
			L"   y: " + mCameraY +
			L"   z: " + mCameraZ +
			L"   -TotalTime:" + totaltime +
			L"   CameraSpeed: " + CameraSpeed
			;

		SetWindowText(std::dynamic_pointer_cast<Win32Window>(Engine::Get()->GetWindow())->mhMainWnd, windowText.c_str());

		// Reset for next average.
		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}

void DXRHI::RenderDocBeginEvent(std::string a)
{
	PIXBeginEvent(mCommandList.Get(), 0, a.c_str());
}

void DXRHI::RenderDocEndEvent()
{
	PIXEndEvent(mCommandList.Get());
}


void DXRHI::BuildDescriptorHeaps()
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
		auto mCbvHeap = std::dynamic_pointer_cast<DXPrimitive_Heap>(mRenderPrimitiveManager->GetHeapByName("mCbvHeap"));
		auto heapCPUHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mCbvHeap->mDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

		D3D12_GPU_VIRTUAL_ADDRESS cbAddress = mObjectCB->Resource()->GetGPUVirtualAddress();
		heapCPUHandle.Offset(i, DescriptorSize);
		cbAddress += i * ConstantbufferSize;
		cbvDesc.BufferLocation = cbAddress;
		cbvDesc.SizeInBytes = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
		md3dDevice->CreateConstantBufferView(&cbvDesc, heapCPUHandle);
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

	CD3DX12_ROOT_PARAMETER slotRootParameter[6];

	// Create a single descriptor table of CBVs.
	CD3DX12_DESCRIPTOR_RANGE cbvTable;
	CD3DX12_DESCRIPTOR_RANGE srvTable;
	CD3DX12_DESCRIPTOR_RANGE srvTable2;
	CD3DX12_DESCRIPTOR_RANGE srvTable3;
	cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

	srvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	slotRootParameter[1].InitAsDescriptorTable(1, &srvTable);

	slotRootParameter[2].InitAsConstants(4, 1);

	srvTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
	slotRootParameter[3].InitAsDescriptorTable(1, &srvTable2);

	srvTable3.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);
	slotRootParameter[4].InitAsDescriptorTable(1, &srvTable3);

	slotRootParameter[5].InitAsConstants(4, 2);


	auto staticSamplers = GetStaticSamplers();	//获得静态采样器集合

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(6, slotRootParameter,
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
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BITANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 60, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 76, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

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
