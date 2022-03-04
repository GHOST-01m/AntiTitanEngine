#include "stdafx.h"

//#include <WindowsX.h>

WindowsApp* WindowsApp::mWindowsApp = nullptr;

WindowsApp* WindowsApp::GetApp() {
	return mWindowsApp;
};

WindowsApp::WindowsApp(HINSTANCE hInstance):App(hInstance)
{
	mhAppInst = hInstance;
	assert(mWindowsApp == nullptr);
	mWindowsApp = this;
}

WindowsApp::~WindowsApp() {
}

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Forward hwnd on because we can get messages (e.g., WM_CREATE)
	// before CreateWindow returns, and thus before mhMainWnd is valid.
	return WindowsApp::GetApp()->MsgProc(hwnd, msg, wParam, lParam);
}

HINSTANCE WindowsApp::AppInst()const
{
	return mhAppInst;
}

HWND WindowsApp::MainWnd()const
{
	return mhMainWnd;
}

int WindowsApp::Run() {

	MSG msg = { 0 };

	mTimer.Reset();

	while (msg.message != WM_QUIT)
	{
		// If there are Window messages then process them.
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		// Otherwise, do animation/game stuff.
		else
		{
			mTimer.Tick();

			if (!mAppPaused)
			{
				CalculateFrameStats();
				Update(mTimer);
				Draw(mTimer);
			}
			else
			{
				Sleep(100);
			}
		}
	}
	return (int)msg.wParam;

};

bool WindowsApp::Initialize() {

	if (!InitMainWindow())
		return false;

	if (!InitDirect3D())
		return false;

	OnResize();

	// Reset the command list to prep for initialization commands.
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

	mCamera.SetPosition(300.0f, -3000.0f, 1000.0f);
	mCamera.MoveSpeed = 500.0;
	//mCamera.Roll(90.0f);
	MapActor.SetSceneActorsInfoFromBat("MapActorInfo/MapActorInfo.bat");
	uploadBuffer.resize(MapActor.Size());

	BuildDescriptorHeaps();
	//BuildConstantBuffers();
	BuildRootSignature();
	BuildShadersAndInputLayout();
	BuildBoxGeometry();
	BuildPSO();

	// Execute the initialization commands.
	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until initialization is complete.
	FlushCommandQueue();

	return true;
};

void WindowsApp::OnResize() {
	App::OnResize();
	mCamera.SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 10000.0f);
};

void WindowsApp::Update(const GameTimer& gt) {

	OnKeyboardInput(gt);

	ObjectConstants objConstants;

	for (int i =0;i<MapActor.Size();i++)
	{
		//auto world = MathHelper::Identity4x4();
		auto location=XMMatrixTranslation(
			MapActor.ActorsTransformArray[i].translation.x,
			MapActor.ActorsTransformArray[i].translation.y,
			MapActor.ActorsTransformArray[i].translation.z
			);
		auto Scale = XMMatrixScaling(
			MapActor.ActorsTransformArray[i].scale3D.x,
			MapActor.ActorsTransformArray[i].scale3D.y,
			MapActor.ActorsTransformArray[i].scale3D.z
		);

		DirectX::XMVECTORF32 g_XMIdentityR3 = { { {
				MapActor.ActorsQuatArray[i].X,
				MapActor.ActorsQuatArray[i].Y,
				MapActor.ActorsQuatArray[i].Z,
				MapActor.ActorsQuatArray[i].W
			} } };
		auto mrotation = DirectX::XMMatrixRotationQuaternion(g_XMIdentityR3);

		//auto Rotation = XMMatrixRotationRollPitchYaw(
		//	MapActor.ActorsTransformArray[i].rotation.Pitch ,
		//	MapActor.ActorsTransformArray[i].rotation.Yaw ,
		//	MapActor.ActorsTransformArray[i].rotation.Roll 
		//);//直接用虚幻的角度是不对的

		/*Glm库的方法，暂时不用，改掉的工程量有点大*/
		glm::mat4 translateMat4 = glm::translate(glm::mat4(1.f), glm::vec3(
			MapActor.ActorsTransformArray[i].translation.x, 
			MapActor.ActorsTransformArray[i].translation.y,
			MapActor.ActorsTransformArray[i].translation.z
		));

		glm::mat4 scaleMat4 = glm::scale(glm::mat4(1.f), glm::vec3(
			MapActor.ActorsTransformArray[i].scale3D.x,
			MapActor.ActorsTransformArray[i].scale3D.y,
			MapActor.ActorsTransformArray[i].scale3D.z
		));

		glm::quat rotationQuat(
			MapActor.ActorsQuatArray[i].X,
			MapActor.ActorsQuatArray[i].Y,
			MapActor.ActorsQuatArray[i].Z,
			MapActor.ActorsQuatArray[i].W
		);
		glm::mat4 rotationMat4 = glm::toMat4(rotationQuat);

		auto world =   Scale * mrotation * location;
		glm::mat4 worldMat4 = scaleMat4 * rotationMat4 * translateMat4;
		
		XMMATRIX worldViewProj = world * XMLoadFloat4x4(&mCamera.GetView4x4f()) * XMLoadFloat4x4((&mCamera.GetProj4x4f()));
		glm::mat4 worldViewProjMat4 =  mCamera.GetProjMat4() *  mCamera.GetViewMat4() * worldMat4;
		
		XMStoreFloat4x4(&objConstants.WorldViewProj, XMMatrixTranspose(worldViewProj));
		objConstants.WorldViewProjMat4=glm::transpose(worldViewProjMat4);


		mObjectCB->CopyData(i, objConstants);
		//copy mat4
	}
};

void WindowsApp::Draw(const GameTimer& gt) {

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

	for (int i=0;i<MapActor.Size();i++)
	{
		for (std::map<int, std::string>::iterator it = MapofGeosMesh.begin(); it != MapofGeosMesh.end(); it++)
		{
			//MapofGeosMesh通过映射找到MapActor的名字对应的Geos中的key
			if (it->second == MapActor.MeshNameArray[i]){
			mCommandList->IASetVertexBuffers(0, 1, &Geos[it->first]->VertexBufferView());
			mCommandList->IASetIndexBuffer(&Geos[it->first]->IndexBufferView());
			mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			auto heapHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mCbvHeap->GetGPUDescriptorHandleForHeapStart());
			heapHandle.Offset(i, md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
			mCommandList->SetGraphicsRootDescriptorTable(0, heapHandle);

			mCommandList->DrawIndexedInstanced(Geos[it->first]->DrawArgs[MapActor.MeshNameArray[i]].IndexCount, 1, 0, 0, 0);
			break;
			}
		}
	}
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
};

/*下面注释掉的是以前绘制单个几何体的代码*/
//void WindowsApp::Draw(const GameTimer& gt) {
//
//	// Reuse the memory associated with command recording.
//	// We can only reset when the associated command lists have finished execution on the GPU.
//	ThrowIfFailed(mDirectCmdListAlloc->Reset());
//
//	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
//	// Reusing the command list reuses memory.
//	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), mPSO.Get()));
//
//	mCommandList->RSSetViewports(1, &mScreenViewport);
//	mCommandList->RSSetScissorRects(1, &mScissorRect);
//
//	// Indicate a state transition on the resource usage.
//	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
//		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
//
//	// Clear the back buffer and depth buffer.
//	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::Black, 0, nullptr);
//	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
//
//	// Specify the buffers we are going to render to.
//	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());
//
//	ID3D12DescriptorHeap* descriptorHeaps[] = { mCbvHeap.Get() };
//	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
//
//	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());
//
//	mCommandList->IASetVertexBuffers(0, 1, &mBoxGeo->VertexBufferView());
//	mCommandList->IASetIndexBuffer(&mBoxGeo->IndexBufferView());
//	mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//
//	mCommandList->SetGraphicsRootDescriptorTable(0, mCbvHeap->GetGPUDescriptorHandleForHeapStart());
//
//	mCommandList->DrawIndexedInstanced(mBoxGeo->DrawArgs["box"].IndexCount, 1, 0, 0, 0);
//
//	// Indicate a state transition on the resource usage.
//	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
//		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
//
//	// Done recording commands.
//	ThrowIfFailed(mCommandList->Close());
//
//	// Add the command list to the queue for execution.
//	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
//	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
//
//	// swap the back and front buffers
//	ThrowIfFailed(mSwapChain->Present(0, 0));
//	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;
//
//	// Wait until frame commands are complete.  This waiting is inefficient and is
//	// done for simplicity.  Later we will show how to organize our rendering code
//	// so we do not have to wait per frame.
//	FlushCommandQueue();
//
//};

void WindowsApp::OnKeyboardInput(const GameTimer& gt) {

	const float dt = gt.DeltaTime();

	if (GetAsyncKeyState('W') & 0x8000) {
		mCamera.Walk(mCamera.MoveSpeed * dt);
	}

	if (GetAsyncKeyState('S') & 0x8000) {
		mCamera.Walk(-(mCamera.MoveSpeed) * dt);
	}

	if (GetAsyncKeyState('A') & 0x8000) {
		mCamera.Strafe(-(mCamera.MoveSpeed) * dt);
	}

	if (GetAsyncKeyState('D') & 0x8000) {
		mCamera.Strafe(mCamera.MoveSpeed * dt);
	}

	if (GetAsyncKeyState('Q') & 0x8000) {
		mCamera.Fly(-(mCamera.MoveSpeed) * dt);
	}

	if (GetAsyncKeyState('E') & 0x8000) {
		mCamera.Fly(mCamera.MoveSpeed * dt);
	}

	mCamera.UpdateViewMatrix();
}

LRESULT WindowsApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	switch (msg)
	{
		// WM_ACTIVATE is sent when the window is activated or deactivated.  
		// We pause the game when the window is deactivated and unpause it 
		// when it becomes active.  
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			mAppPaused = true;
			mTimer.Stop();
		}
		else
		{
			mAppPaused = false;
			mTimer.Start();
		}
		return 0;

		// WM_SIZE is sent when the user resizes the window.  
	case WM_SIZE:
		// Save the new client area dimensions.
		mClientWidth = LOWORD(lParam);
		mClientHeight = HIWORD(lParam);
		if (md3dDevice)
		{
			if (wParam == SIZE_MINIMIZED)
			{
				mAppPaused = true;
				mMinimized = true;
				mMaximized = false;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				mAppPaused = false;
				mMinimized = false;
				mMaximized = true;
				OnResize();
			}
			else if (wParam == SIZE_RESTORED)
			{

				// Restoring from minimized state?
				if (mMinimized)
				{
					mAppPaused = false;
					mMinimized = false;
					OnResize();
				}

				// Restoring from maximized state?
				else if (mMaximized)
				{
					mAppPaused = false;
					mMaximized = false;
					OnResize();
				}
				else if (mResizing)
				{
					// If user is dragging the resize bars, we do not resize 
					// the buffers here because as the user continuously 
					// drags the resize bars, a stream of WM_SIZE messages are
					// sent to the window, and it would be pointless (and slow)
					// to resize for each WM_SIZE message received from dragging
					// the resize bars.  So instead, we reset after the user is 
					// done resizing the window and releases the resize bars, which 
					// sends a WM_EXITSIZEMOVE message.
				}
				else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
				{
					OnResize();
				}
			}
		}
		return 0;

		// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
	case WM_ENTERSIZEMOVE:
		mAppPaused = true;
		mResizing = true;
		mTimer.Stop();
		return 0;

		// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
		// Here we reset everything based on the new window dimensions.
	case WM_EXITSIZEMOVE:
		mAppPaused = false;
		mResizing = false;
		mTimer.Start();
		OnResize();
		return 0;

		// WM_DESTROY is sent when the window is being destroyed.
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

		// The WM_MENUCHAR message is sent when a menu is active and the user presses 
		// a key that does not correspond to any mnemonic or accelerator key. 
	case WM_MENUCHAR:
		// Don't beep when we alt-enter.
		return MAKELRESULT(0, MNC_CLOSE);

		// Catch this message so to prevent the window from becoming too small.
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_KEYUP:
		if (wParam == VK_ESCAPE)
		{
			PostQuitMessage(0);
		}
		else if ((int)wParam == VK_F2)
			Set4xMsaaState(!m4xMsaaState);


		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);

};

// Convenience overrides for handling mouse input.
void WindowsApp::OnMouseDown(WPARAM btnState, int x, int y) {
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void WindowsApp::OnMouseUp(WPARAM btnState, int x, int y) {
	ReleaseCapture();
 }

void WindowsApp::OnMouseMove(WPARAM btnState, int x, int y) {
	if ((btnState & MK_RBUTTON) != 0) {
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

		mCamera.Pitch(dy);
		mCamera.Yaw(dx);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

bool WindowsApp::InitMainWindow() {

	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = mhAppInst;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = L"MainWnd";

	if (!RegisterClass(&wc))
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	// Compute window rectangle dimensions based on requested client area dimensions.
	RECT R = { 0, 0, mClientWidth, mClientHeight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	mhMainWnd = CreateWindow(L"MainWnd", mMainWndCaption.c_str(),
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, mhAppInst, 0);
	if (!mhMainWnd)
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(mhMainWnd, SW_SHOW);
	UpdateWindow(mhMainWnd);

	return true;

};

void WindowsApp::CreateSwapChain() {

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
	sd.OutputWindow = mhMainWnd;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// Note: Swap chain uses queue to perform flush.
	ThrowIfFailed(mdxgiFactory->CreateSwapChain(
		mCommandQueue.Get(),
		&sd,
		mSwapChain.GetAddressOf()));

};

void WindowsApp::CalculateFrameStats() {

	// Code computes the average frames per second, and also the 
	// average time it takes to render one frame.  These stats 
	// are appended to the window caption bar.

	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	// Compute averages over one second period.
	if ((mTimer.TotalTime() - timeElapsed) >= 1.0f)
	{
		float fps = (float)frameCnt; // fps = frameCnt / 1
		float mspf = 1000.0f / fps;

		std::wstring fpsStr = std::to_wstring(fps);
		std::wstring mspfStr = std::to_wstring(mspf);

		std::wstring mCameraX = std::to_wstring(mCamera.mPosition.x);
		std::wstring mCameraY = std::to_wstring(mCamera.mPosition.y);
		std::wstring mCameraZ = std::to_wstring(mCamera.mPosition.z);

		std::wstring windowText = mMainWndCaption +
			L"    fps: " + fpsStr +
			L"   mspf: " + mspfStr+
			L"   Location: " + 
			L"   x: " + mCameraX +
			L"   y: " + mCameraY +
			L"   z: " + mCameraZ 

			;

		SetWindowText(mhMainWnd, windowText.c_str());

		// Reset for next average.
		frameCnt = 0;
		timeElapsed += 1.0f;
	}

};//窗口上面变动的fps和mspf

void WindowsApp::BuildDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = MapActor.Size();

	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&cbvHeapDesc,
		IID_PPV_ARGS(&mCbvHeap)));
	
	SetDescriptorHeaps();
}

void WindowsApp::SetDescriptorHeaps() {//给Heap开辟空间

	mObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(md3dDevice.Get(), MapActor.Size(), true);

	UINT DescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	UINT ConstantbufferSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	
	// Offset to the ith object constant buffer in the buffer.

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	//cbvDesc.BufferLocation = cbAddress;
	//cbvDesc.SizeInBytes = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	//循环开辟Heap空间
	for (int i=0;i<MapActor.Size();i++)
	{
		D3D12_GPU_VIRTUAL_ADDRESS cbAddress = mObjectCB->Resource()->GetGPUVirtualAddress();
		auto heapCPUHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mCbvHeap->GetCPUDescriptorHandleForHeapStart());
		heapCPUHandle.Offset(i, DescriptorSize);
		cbAddress += i*ConstantbufferSize;
		//cbAddress=uploadBuffer[i]->GetGPUVirtualAddress();
		cbvDesc.BufferLocation = cbAddress;
		cbvDesc.SizeInBytes = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
		md3dDevice->CreateConstantBufferView(&cbvDesc, heapCPUHandle);

		//heapCPUHandle.Offset(i, DescriptorSize);
	}
}

/*下面注释的是原来只往descriptHeap里开辟一个空间的代码*/
//void WindowsApp::BuildConstantBuffers()
//{
//	mObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(md3dDevice.Get(), 1, true);
//
//	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
//
//	D3D12_GPU_VIRTUAL_ADDRESS cbAddress = mObjectCB->Resource()->GetGPUVirtualAddress();
//	// Offset to the ith object constant buffer in the buffer.
//	int boxCBufIndex = 0;
//	cbAddress += boxCBufIndex * objCBByteSize;
//
//	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
//	cbvDesc.BufferLocation = cbAddress;
//	cbvDesc.SizeInBytes = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
//
//	md3dDevice->CreateConstantBufferView(
//		&cbvDesc,
//		mCbvHeap->GetCPUDescriptorHandleForHeapStart());
//}

void WindowsApp::BuildRootSignature()
{
	// Shader programs typically require resources as input (constant buffers,
	// textures, samplers).  The root signature defines the resources the shader
	// programs expect.  If we think of the shader programs as a function, and
	// the input resources as function parameters, then the root signature can be
	// thought of as defining the function signature.  

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[1];

	// Create a single descriptor table of CBVs.
	CD3DX12_DESCRIPTOR_RANGE cbvTable;
	cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, slotRootParameter, 0, nullptr,
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

void WindowsApp::BuildShadersAndInputLayout()
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
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void WindowsApp::BuildBoxGeometry()
{
	std::string StaticMeshPath;
	std::set<std::string> StaticMeshs;//用于去重

	StaticMesh mesh;

	Geos.resize(MapActor.Size());

	//std::vector<Vertex> vertices;
	//std::vector<int32_t> indices;

	Vertex vertice;

	UINT vbByteSize ;
	UINT ibByteSize ;

	//循环加入MeshGeometry
	for (int i = 0; i < Geos.size();i++)
	{
		std::vector<Vertex> vertices;
		std::vector<int32_t> indices;
		//相同的StaticMesh不用重复建立
		auto check = StaticMeshs.find(MapActor.MeshNameArray[i]);
		if (check == StaticMeshs.end()){
			StaticMeshs.insert(MapActor.MeshNameArray[i]);
			MapofGeosMesh.insert(std::pair<int, std::string>(i, MapActor.MeshNameArray[i]));
		}
		else{continue;}

		//读取mesh信息
		StaticMeshPath = "SplitMesh/" + MapActor.MeshNameArray[i];
		//StaticMeshPath = "SplitMesh/SM_MatPreviewMesh_02";
		StaticMeshPath.erase(StaticMeshPath.length()-1);
		StaticMeshPath += ".bat";
		mesh.SetStaticMeshFromBat(StaticMeshPath);

		if (mesh.MeshInfo.MeshVertexInfo.size() < 3) { continue; }//没有StaticMesh就不读取
		//--------------------------------------------------------------------------------

		for (int j = 0; j < mesh.MeshInfo.MeshVertexInfo.size(); j++)
		{
			vertice.Pos = mesh.MeshInfo.MeshVertexInfo[j] ;
			
			//vertice.Color = {
			//	static_cast <float> (rand()) / static_cast <float> (RAND_MAX),
			//	static_cast <float> (rand()) / static_cast <float> (RAND_MAX),
			//	static_cast <float> (rand()) / static_cast <float> (RAND_MAX),
			//	static_cast <float> (rand()) / static_cast <float> (RAND_MAX)
			//};//Random Color

			vertice.Color = {
				float(j) / mesh.MeshInfo.MeshVertexInfo.size(),
				float(j) / mesh.MeshInfo.MeshVertexInfo.size(),
				float(j) / mesh.MeshInfo.MeshVertexInfo.size(),
				1 };//黑白色

			vertice.Normal = mesh.MeshInfo.MeshVertexNormalInfo[j];

			vertices.push_back(vertice);
		}

		indices = mesh.MeshInfo.MeshIndexInfo;

		vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
		ibByteSize = (UINT)indices.size() * sizeof(std::uint32_t);

		Geos[i] = std::make_unique<MeshGeometry>();
		Geos[i]->Name = mesh.getMeshName();

		ThrowIfFailed(D3DCreateBlob(vbByteSize, &Geos[i]->VertexBufferCPU));
		CopyMemory(Geos[i]->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

		ThrowIfFailed(D3DCreateBlob(ibByteSize, &Geos[i]->IndexBufferCPU));
		CopyMemory(Geos[i]->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

		Geos[i]->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
			mCommandList.Get(), vertices.data(), vbByteSize, Geos[i]->VertexBufferUploader);

		Geos[i]->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
			mCommandList.Get(), indices.data(), ibByteSize, Geos[i]->IndexBufferUploader);

		Geos[i]->VertexByteStride = sizeof(Vertex);
		Geos[i]->VertexBufferByteSize = vbByteSize;
		Geos[i]->IndexFormat = DXGI_FORMAT_R32_UINT;
		Geos[i]->IndexBufferByteSize = ibByteSize;

		SubmeshGeometry submesh;
		submesh.IndexCount = (UINT)indices.size();
		submesh.StartIndexLocation = 0;
		submesh.BaseVertexLocation = 0;

		//Geos[i]->DrawArgs["box"] = submesh;
		Geos[i]->DrawArgs[MapActor.MeshNameArray[i]] = submesh;
		//--------------------------------------------------------------------------------
	}

}


/*下面注释掉的代码是原来build单个Geometry的代码*/
//void WindowsApp::BuildBoxGeometry()
//{
//	std::string StaticMeshPath;
//	std::set<std::string> StaticMeshs;//用于去重
//	std::set<std::string>::iterator iter;
//
//	StaticMesh mesh;
//
//	StaticMeshPath = "SplitMesh/SM_MatPreviewMesh_02.bat";
//
//	mesh.SetStaticMeshFromBat(StaticMeshPath);
//
//	std::vector<Vertex> vertices;
//	Vertex vertice;
//
//	for (int i = 0; i < mesh.MeshInfo.MeshVertexInfo.size(); i++)
//	{
//		vertice.Pos.x = mesh.MeshInfo.MeshVertexInfo[i].X;
//		vertice.Pos.y = mesh.MeshInfo.MeshVertexInfo[i].Y;
//		vertice.Pos.z = mesh.MeshInfo.MeshVertexInfo[i].Z;
//		vertice.Color = {
//			static_cast <float> (rand()) / static_cast <float> (RAND_MAX),
//			static_cast <float> (rand()) / static_cast <float> (RAND_MAX),
//			static_cast <float> (rand()) / static_cast <float> (RAND_MAX),
//			static_cast <float> (rand()) / static_cast <float> (RAND_MAX)
//		};//Random Color
//
//		vertices.push_back(vertice);
//	}
//
//	if (mesh.MeshInfo.MeshIndexInfo.size() < 3) { return; }
//	std::vector<int32_t> indices = mesh.MeshInfo.MeshIndexInfo;
//
//	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
//	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint32_t);
//
//	mBoxGeo = std::make_unique<MeshGeometry>();
//	mBoxGeo->Name = mesh.getMeshName();
//
//	ThrowIfFailed(D3DCreateBlob(vbByteSize, &mBoxGeo->VertexBufferCPU));
//	CopyMemory(mBoxGeo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);
//
//	ThrowIfFailed(D3DCreateBlob(ibByteSize, &mBoxGeo->IndexBufferCPU));
//	CopyMemory(mBoxGeo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);
//
//	mBoxGeo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
//		mCommandList.Get(), vertices.data(), vbByteSize, mBoxGeo->VertexBufferUploader);
//
//	mBoxGeo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
//		mCommandList.Get(), indices.data(), ibByteSize, mBoxGeo->IndexBufferUploader);
//
//	mBoxGeo->VertexByteStride = sizeof(Vertex);
//	mBoxGeo->VertexBufferByteSize = vbByteSize;
//	mBoxGeo->IndexFormat = DXGI_FORMAT_R32_UINT;
//	mBoxGeo->IndexBufferByteSize = ibByteSize;
//
//	SubmeshGeometry submesh;
//	submesh.IndexCount = (UINT)indices.size();
//	submesh.StartIndexLocation = 0;
//	submesh.BaseVertexLocation = 0;
//
//	mBoxGeo->DrawArgs["box"] = submesh;
//}

void WindowsApp::BuildPSO()
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
