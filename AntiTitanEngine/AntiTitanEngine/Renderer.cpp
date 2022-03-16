#include "stdafx.h"
#include "Renderer.h"

Renderer::Renderer()
{

}

Renderer::~Renderer()
{
	OutputDebugStringA("Renderer::~Renderer()\n");
}

Microsoft::WRL::ComPtr<ID3D12Device>* Renderer::Getd3dDevice() {
	return &md3dDevice;
};

bool Renderer::IsDeviceValid()
{
	return true;
}

std::shared_ptr<Camera> Renderer::mCamera = nullptr;

std::shared_ptr<Camera> Renderer::GetCamera()
{
	return mCamera;
}

//Camera* Renderer::GetCamera()
//{
//	return &mCamera;
//}

bool Renderer::Get4xMsaaState() const
{
	return m4xMsaaState;
}

void Renderer::Set4xMsaaState(bool value)
{

	if (m4xMsaaState != value)
	{
		m4xMsaaState = value;

		// Recreate the swapchain and buffers with new multisample settings.
		CreateSwapChain();
		OnResize();
	}
}

bool Renderer::InitRenderer() {

	mCamera = std::make_shared<Camera>();

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
	InitDX_CreateRtvAndDsvDescriptorHeaps();
	//--------------------------------------------------------------------------------

	OnResize();

	// Reset the command list to prep for initialization commands.
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));
	
	//mAsset.LoadExternalMapActor(MapLoadPath);//要先从外部导入地图数据才能绘制//这个应该放在游戏里
	Engine::Get()->GetAssetManager()->LoadExternalMapActor(MapLoadPath);
	Engine::Get()->GetMaterialSystem()->LoadTexture();
	BuildDescriptorHeaps();
	BuildRootSignature();
	BuildShadersAndInputLayout();
	LoadAsset();
	BuildPSO();

	// Execute the initialization commands.
	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until initialization is complete.
	FlushCommandQueue();

	return true;
}

void Renderer::InitDX_CreateCommandObjects() {

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

void Renderer::InitDX_CreateSwapChain() {
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

void Renderer::InitDX_CreateRtvAndDsvDescriptorHeaps() {

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = SwapChainBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(
		&rtvHeapDesc, IID_PPV_ARGS(mRtvHeap.GetAddressOf())));

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(
		&dsvHeapDesc, IID_PPV_ARGS(mDsvHeap.GetAddressOf())));

};

void Renderer::OnResize()
{
	assert(md3dDevice);
	assert(mSwapChain);
	assert(mDirectCmdListAlloc);

	// Flush before changing any resources.
	FlushCommandQueue();

	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

	// Release the previous resources we will be recreating.
	for (int i = 0; i < SwapChainBufferCount; ++i)
		mSwapChainBuffer[i].Reset();
	mDepthStencilBuffer.Reset();

		
	// Resize the swap chain.
	ThrowIfFailed(mSwapChain->ResizeBuffers(
		SwapChainBufferCount,
		Engine::Get()->GetWindow()->mClientWidth, 
		Engine::Get()->GetWindow()->mClientHeight,
		mBackBufferFormat,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	mCurrBackBuffer = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
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
		IID_PPV_ARGS(mDepthStencilBuffer.GetAddressOf())));

	// Create descriptor to mip level 0 of entire resource using the format of the resource.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = mDepthStencilFormat;
	dsvDesc.Texture2D.MipSlice = 0;
	md3dDevice->CreateDepthStencilView(mDepthStencilBuffer.Get(), &dsvDesc, DepthStencilView());

	// Transition the resource from its initial state to be used as a depth buffer.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mDepthStencilBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));

	// Execute the resize commands.
	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until resize is complete.
	FlushCommandQueue();

	// Update the viewport transform to cover the client area.
	mScreenViewport.TopLeftX = 0;
	mScreenViewport.TopLeftY = 0;
	mScreenViewport.Width = static_cast<float>(Engine::Get()->GetWindow()->mClientWidth);
	mScreenViewport.Height = static_cast<float>(Engine::Get()->GetWindow()->mClientHeight);
	mScreenViewport.MinDepth = 0.0f;
	mScreenViewport.MaxDepth = 1.0f;

	mScissorRect = { 0, 0, Engine::Get()->GetWindow()->mClientWidth, Engine::Get()->GetWindow()->mClientHeight };

	mCamera->SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 100000.0f);

}

float Renderer::AspectRatio(){
	return static_cast<float>(mClientWidth) / mClientHeight;
};

void Renderer::Update()
{
	//OnKeyboardInput(*(Engine::Get()->GetGameTimer()));

	ObjectConstants objConstants;

	for (int i = 0; i < Engine::Get()->GetAssetManager()->GetMapActorInfo()->Size(); i++)
	{
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

		XMMATRIX worldViewProj = world * XMLoadFloat4x4(&mCamera->GetView4x4f()) * XMLoadFloat4x4((&mCamera->GetProj4x4f()));
		glm::mat4 worldViewProjMat4 = mCamera->GetProjMat4() * mCamera->GetViewMat4() * worldMat4;
		
		//XMMATRIX worldViewProj = world * XMLoadFloat4x4(&mCamera.GetView4x4f()) * XMLoadFloat4x4((&mCamera.GetProj4x4f()));
		//glm::mat4 worldViewProjMat4 = mCamera.GetProjMat4() * mCamera.GetViewMat4() * worldMat4;

		XMStoreFloat4x4(&objConstants.WorldViewProj, XMMatrixTranspose(worldViewProj));
		objConstants.WorldViewProjMat4 = glm::transpose(worldViewProjMat4);
		objConstants.mTime = Engine::Get()->gt.TotalTime();
		XMStoreFloat4x4(&objConstants.rotation, XMMatrixTranspose(mrotation));
		//objConstants.rotation = rotationMat4;

		//objConstants.CanMove = 1;
		// 
		//设定只有指定名字的mesh可以动
		//auto name = Engine::Get()->GetAsset()->GetMapActorInfo()->MeshNameArray[i];
		//std::string MoveMeshName = "Shape_Cube";
		// MoveMeshName = name;

		//if (name == MoveMeshName)
		//{
		//	objConstants.CanMove = true;
		//}
		//else {
		//	objConstants.CanMove = false;
		//}

		mObjectCB->CopyData(i, objConstants);
	}
}

void Renderer::Draw()
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
	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	ID3D12DescriptorHeap* descriptorHeaps[] = { mCbvHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

	//for (int i = 0; i < Engine::Get()->GetAsset()->GetMapActorInfo()->Size(); i++)//绘制每一个Actor
	//{
	//	for (auto it = Engine::Get()->GetAsset()->GetMapofGeosMesh().begin(); it != Engine::Get()->GetAsset()->GetMapofGeosMesh().end(); it++)
	//	{
	//		//MapofGeosMesh通过映射找到MapActor的名字对应的Geos中的key
	//		if (it->second == Engine::Get()->GetAsset()->GetMapActorInfo()->MeshNameArray[i]) {

	//			auto a = Engine::Get()->GetAsset()->GetGeometryLibrary();
	//			mCommandList->IASetVertexBuffers(0, 1, &Engine::Get()->GetAsset()->Geos[it->first]->VertexBufferView());
	//			mCommandList->IASetIndexBuffer(&Engine::Get()->GetAsset()->Geos[it->first]->IndexBufferView());
	//			mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//			auto heapHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mCbvHeap->GetGPUDescriptorHandleForHeapStart());
	//			heapHandle.Offset(i, md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	//			mCommandList->SetGraphicsRootDescriptorTable(0, heapHandle);

	//			mCommandList->DrawIndexedInstanced(Engine::Get()->GetAsset()->Geos[it->first]->DrawArgs[Engine::Get()->GetAsset()->mMapActor.MeshNameArray[i]].IndexCount, 1, 0, 0, 0);
	//			break;
	//		}
	//	}
	//}

	for (int i = 0; i < Engine::Get()->GetAssetManager()->GetMapActorInfo()->Size(); i++)//绘制每一个Actor
	{
		auto GeoIndex = Engine::Get()->GetAssetManager()->GetGeoKeyByName(Engine::Get()->GetAssetManager()->GetMapActorInfo()->MeshNameArray[i]);

		mCommandList->IASetVertexBuffers(0, 1, &Engine::Get()->GetAssetManager()->Geos[GeoIndex]->VertexBufferView());
		mCommandList->IASetIndexBuffer(&Engine::Get()->GetAssetManager()->Geos[GeoIndex]->IndexBufferView());
		mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		auto heapHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mCbvHeap->GetGPUDescriptorHandleForHeapStart());
		heapHandle.Offset(i, md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
		mCommandList->SetGraphicsRootDescriptorTable(0, heapHandle);


		//贴图的Size要记得改下面的Offset
		auto GPUHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mCbvHeap->GetGPUDescriptorHandleForHeapStart());
		GPUHandle.Offset(Engine::Get()->GetAssetManager()->GetMapActorInfo()->Size(), md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
		mCommandList->SetGraphicsRootDescriptorTable(1, GPUHandle);

		mCommandList->DrawIndexedInstanced(Engine::Get()->GetAssetManager()->Geos[GeoIndex]->DrawArgs[Engine::Get()->GetAssetManager()->GetMapActorInfo()->MeshNameArray[i]].IndexCount, 1, 0, 0, 0);
	}

	mCommandList->SetGraphicsRoot32BitConstants(2, 3, &mCamera->GetPosition(), 0);

	// Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	// Done recording commands.
	ThrowIfFailed(mCommandList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// swap the back and front buffers
	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

	// Wait until frame commands are complete.  This waiting is inefficient and is
	// done for simplicity.  Later we will show how to organize our rendering code
	// so we do not have to wait per frame.
	FlushCommandQueue();
}

void Renderer::FlushCommandQueue()
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

void Renderer::CreateSwapChain()
{
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

}

void Renderer::LoadAsset()//将外部导入的Actor信息赋给VS
{
	std::string StaticMeshPath;
	std::set<std::string> StaticMeshs;//用于去重

	StaticMesh mesh;

	Engine::Get()->GetAssetManager()->GetGeometryLibrary()->resize(Engine::Get()->GetAssetManager()->GetMapActorInfo()->Size());

	Vertex vertice;

	UINT vbByteSize;
	UINT ibByteSize;

	//循环加入MeshGeometry
	for (int i = 0; i < Engine::Get()->GetAssetManager()->Geos.size(); i++)
	{
		std::vector<Vertex> vertices;
		std::vector<int32_t> indices;
		//相同的StaticMesh不用重复建立
		auto check = StaticMeshs.find(Engine::Get()->GetAssetManager()->GetMapActorInfo()->MeshNameArray[i]);
		if (check == StaticMeshs.end()) {
			StaticMeshs.insert(Engine::Get()->GetAssetManager()->GetMapActorInfo()->MeshNameArray[i]);
			Engine::Get()->GetAssetManager()->GetMapofGeosMesh()->insert(std::pair<int, std::string>(i, Engine::Get()->GetAssetManager()->GetMapActorInfo()->MeshNameArray[i]));
		}
		else { continue; }

		//读取mesh信息
		StaticMeshPath = "SplitMesh/" + Engine::Get()->GetAssetManager()->GetMapActorInfo()->MeshNameArray[i];
		//StaticMeshPath = "SplitMesh/SM_MatPreviewMesh_02";
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

		indices = mesh.MeshInfo.MeshIndexInfo;

		vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
		ibByteSize = (UINT)indices.size() * sizeof(std::uint32_t);

		Engine::Get()->GetAssetManager()->Geos[i] = std::make_unique<MeshGeometry>();
		Engine::Get()->GetAssetManager()->Geos[i]->Name = mesh.getMeshName();

		ThrowIfFailed(D3DCreateBlob(vbByteSize, &Engine::Get()->GetAssetManager()->Geos[i]->VertexBufferCPU));
		CopyMemory(Engine::Get()->GetAssetManager()->Geos[i]->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

		ThrowIfFailed(D3DCreateBlob(ibByteSize, &Engine::Get()->GetAssetManager()->Geos[i]->IndexBufferCPU));
		CopyMemory(Engine::Get()->GetAssetManager()->Geos[i]->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

		Engine::Get()->GetAssetManager()->Geos[i]->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
			mCommandList.Get(), vertices.data(), vbByteSize, Engine::Get()->GetAssetManager()->Geos[i]->VertexBufferUploader);

		Engine::Get()->GetAssetManager()->Geos[i]->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
			mCommandList.Get(), indices.data(), ibByteSize, Engine::Get()->GetAssetManager()->Geos[i]->IndexBufferUploader);

		Engine::Get()->GetAssetManager()->Geos[i]->VertexByteStride = sizeof(Vertex);
		Engine::Get()->GetAssetManager()->Geos[i]->VertexBufferByteSize = vbByteSize;
		Engine::Get()->GetAssetManager()->Geos[i]->IndexFormat = DXGI_FORMAT_R32_UINT;
		Engine::Get()->GetAssetManager()->Geos[i]->IndexBufferByteSize = ibByteSize;

		SubmeshGeometry submesh;
		submesh.IndexCount = (UINT)indices.size();
		submesh.StartIndexLocation = 0;
		submesh.BaseVertexLocation = 0;

		//Geos[i]->DrawArgs["box"] = submesh;
		Engine::Get()->GetAssetManager()->Geos[i]->DrawArgs[Engine::Get()->GetAssetManager()->GetMapActorInfo()->MeshNameArray[i]] = submesh;
		//--------------------------------------------------------------------------------
	}
}

void Renderer::CalculateFrameStats()
{

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

void Renderer::BuildDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	if (!Engine::Get()->GetAssetManager()->GetMapActorInfo()->Size())
	{
		cbvHeapDesc.NumDescriptors = 1;
	}
	else {
		cbvHeapDesc.NumDescriptors = 
			Engine::Get()->GetAssetManager()->GetMapActorInfo()->Size() + 
			Engine::Get()->GetMaterialSystem()->GetTextureNum();//Actor数量加材质数量
	}

	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&cbvHeapDesc,
		IID_PPV_ARGS(&mCbvHeap)));

	SetDescriptorHeaps();
}

void Renderer::SetDescriptorHeaps()
{
	mObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(md3dDevice.Get(),Engine::Get()->GetAssetManager()->GetMapActorInfo()->Size(),true);


	UINT DescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	UINT ConstantbufferSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	UINT ShaderResourceSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// Offset to the ith object constant buffer in the buffer.

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;


	//循环开辟Heap空间
	for (int i = 0; i < Engine::Get()->GetAssetManager()->GetMapActorInfo()->Size(); i++)
	{
		auto heapCPUHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mCbvHeap->GetCPUDescriptorHandleForHeapStart());

		D3D12_GPU_VIRTUAL_ADDRESS cbAddress = mObjectCB->Resource()->GetGPUVirtualAddress();
		heapCPUHandle.Offset(i, DescriptorSize);
		cbAddress += i * ConstantbufferSize;
		cbvDesc.BufferLocation = cbAddress;
		cbvDesc.SizeInBytes = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
		md3dDevice->CreateConstantBufferView(&cbvDesc, heapCPUHandle);
	}


	for (int srvIndex=0; srvIndex<Engine::Get()->GetMaterialSystem()->GetTextureNum(); srvIndex++)
	{
		auto heapCPUHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mCbvHeap->GetCPUDescriptorHandleForHeapStart());

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

	}

}

void Renderer::BuildRootSignature()
{
	// Shader programs typically require resources as input (constant buffers,
	// textures, samplers).  The root signature defines the resources the shader
	// programs expect.  If we think of the shader programs as a function, and
	// the input resources as function parameters, then the root signature can be
	// thought of as defining the function signature.  

	// Root parameter can be a table, root descriptor or root constants.

	CD3DX12_ROOT_PARAMETER slotRootParameter[3];

	// Create a single descriptor table of CBVs.
	CD3DX12_DESCRIPTOR_RANGE cbvTable;
	CD3DX12_DESCRIPTOR_RANGE srvTable;
	cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

	srvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	slotRootParameter[1].InitAsDescriptorTable(1, &srvTable);

	slotRootParameter[2].InitAsConstants(1,1);

	auto staticSamplers = GetStaticSamplers();	//获得静态采样器集合

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(3, slotRootParameter, 
		(UINT)staticSamplers.size(),staticSamplers.data(),
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

void Renderer::BuildShadersAndInputLayout()
{
	HRESULT hr = S_OK;

	//mvsByteCode = d3dUtil::CompileShader(L"E:\\DX12Homework\\AntiTitanEngine\\AntiTitanEngine\\AntiTitanEngine\\AntiTitanEngine\\Shaders\\color.hlsl", nullptr, "VS", "vs_5_0");
	//mpsByteCode = d3dUtil::CompileShader(L"E:\\DX12Homework\\AntiTitanEngine\\AntiTitanEngine\\AntiTitanEngine\\AntiTitanEngine\\Shaders\\color.hlsl", nullptr, "PS", "ps_5_0");

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

void Renderer::BuildPSO()
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

ID3D12Resource* Renderer::CurrentBackBuffer() const
{
	return mSwapChainBuffer[mCurrBackBuffer].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE Renderer::CurrentBackBufferView() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		mRtvHeap->GetCPUDescriptorHandleForHeapStart(),
		mCurrBackBuffer,
		mRtvDescriptorSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE Renderer::DepthStencilView()
{
	return mDsvHeap->GetCPUDescriptorHandleForHeapStart();
}

void Renderer::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(std::dynamic_pointer_cast<Win32Window>(Engine::Get()->GetWindow())->GetHWND());
	//SetCapture(dynamic_cast<Win32Window*>(Engine::Get()->GetWindow())->GetHWND());
}

void Renderer::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void Renderer::OnMouseMove(WPARAM btnState, int x, int y)
{

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

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> Renderer::GetStaticSamplers()
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

	return {
		pointWrap, pointClamp,
		linearWrap, linearClamp,
		anisotropicWrap, anisotropicClamp };
}
//void Renderer::OnKeyboardInput(const GameTimer& gt)
//{
//	//const float dt = gt.DeltaTime();
//	const float dt = 5;
//
//	//if (GetAsyncKeyState('W') & 0x8000) {
//	//	mCamera.Walk(mCamera.MoveSpeed * dt);
//	//}
//	//if (GetAsyncKeyState('S') & 0x8000) {
//	//	mCamera.Walk(-(mCamera.MoveSpeed) * dt);
//	//}
//	//if (GetAsyncKeyState('A') & 0x8000) {
//	//	mCamera.Strafe(-(mCamera.MoveSpeed) * dt);
//	//}
//	//if (GetAsyncKeyState('D') & 0x8000) {
//	//	mCamera.Strafe(mCamera.MoveSpeed * dt);
//	//}
//	//if (GetAsyncKeyState('Q') & 0x8000) {
//	//	mCamera.Fly(-(mCamera.MoveSpeed) * dt);
//	//}
//	//if (GetAsyncKeyState('E') & 0x8000) {
//	//	mCamera.Fly(mCamera.MoveSpeed * dt);
//	//}
//	//mCamera.UpdateViewMatrix();
//	if (GetAsyncKeyState('W') & 0x8000) {
//		mCamera->Walk(mCamera->MoveSpeed * dt);
//	}
//	if (GetAsyncKeyState('S') & 0x8000) {
//		mCamera->Walk(-(mCamera->MoveSpeed) * dt);
//	}
//	if (GetAsyncKeyState('A') & 0x8000) {
//		mCamera->Strafe(-(mCamera->MoveSpeed) * dt);
//	}
//	if (GetAsyncKeyState('D') & 0x8000) {
//		mCamera->Strafe(mCamera->MoveSpeed * dt);
//	}
//	if (GetAsyncKeyState('Q') & 0x8000) {
//		mCamera->Fly(-(mCamera->MoveSpeed) * dt);
//	}
//	if (GetAsyncKeyState('E') & 0x8000) {
//		mCamera->Fly(mCamera->MoveSpeed * dt);
//	}
//	mCamera->UpdateViewMatrix();
//}
