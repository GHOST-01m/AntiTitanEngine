#pragma once
#include "RHI.h"
#include "RHIResource.h"
#include "DXRHIResource.h"

#include <wrl.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include <string>
#include <memory>
#include <algorithm>
#include <vector>
#include <array>
#include <unordered_map>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <cassert>
#include "d3dx12.h"
#include "Common/DDSTextureLoader.h"
#include "MathHelper.h"
#include "MyStruct.h"
#include "AssetManager.h"

class DXRHI :public RHI
{
public:
	std::shared_ptr<RHIResource> GetResource()override;
	std::shared_ptr<Camera> GetCamera()override;

public:
	Microsoft::WRL::ComPtr<ID3D12Device> Getd3dDevice();
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetCommandList();
	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();
	float AspectRatio();

public:
	int mCurrBackBuffer = 0;

	static const int SwapChainBufferCount = 2;//交换链数量

	Microsoft::WRL::ComPtr<IDXGIFactory4> mdxgiFactory;
	Microsoft::WRL::ComPtr<IDXGISwapChain> mSwapChain;
	Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice;

	Microsoft::WRL::ComPtr<ID3D12Fence> mFence;
	UINT64 mCurrentFence = 0;

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mDirectCmdListAlloc;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;

	Microsoft::WRL::ComPtr<ID3D12Resource> mSwapChainBuffer[SwapChainBufferCount];
	Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilBuffer;

public:
	UINT mRtvDescriptorSize = 0;
	UINT mDsvDescriptorSize = 0;
	UINT mCbvSrvUavDescriptorSize = 0;

	D3D_DRIVER_TYPE md3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
	DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDsvHeap;

	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
	ComPtr<ID3D12DescriptorHeap> mCbvHeap = nullptr;

	D3D12_VIEWPORT mScreenViewport;
	D3D12_RECT mScissorRect;

	ComPtr<ID3DBlob> mvsByteCode = nullptr;
	ComPtr<ID3DBlob> mpsByteCode = nullptr;
	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

public:
	bool Get4xMsaaState()const;
	void Set4xMsaaState(bool value) override;
	bool      m4xMsaaState = false;    // 4X MSAA enabled
	UINT      m4xMsaaQuality = 0;      // quality level of 4X MSAA

public:
	int mClientWidth = 1920;
	int mClientHeight = 1080;

public:
	bool Init() override;
	void InitDX_CreateCommandObjects();
	void InitDX_CreateSwapChain();
	void InitDX_CreateRtvAndDsvDescriptorHeaps();

public:
	void OnResize();
	void Update() override;
		 void UpdateMVP(int Index, ObjectConstants& objConstants) override;
		 void UpdateTime(ObjectConstants& objConstants) override;
		 void UploadConstant(int offset, ObjectConstants& objConstants) override;
	void Draw() override;
		void DrawReset() override;
		void ResetViewports(int NumViewport, ScreenViewport& vp) override;
		void ResetScissorRects(int NumRects,ScissorRect& sr)override;
		void ResourceBarrier()override;
		void ClearRenderTargetView(Color mClearColor, int NumRects) override;
		void ClearDepthStencilView() override;
		void OMSetRenderTargets()override;
		void SetDescriptorHeapsAndGraphicsRootSignature()override;
		void DrawActor(int ActorIndex)override;
		void DrawFinal()override;


	void FlushCommandQueue();
	void CreateSwapChain();
	void LoadAsset();
	void CalculateFrameStats();

public:
	void BuildDescriptorHeaps();
	void SetDescriptorHeaps();//往Heap里面灌数据
	void BuildRootSignature();
	void BuildShadersAndInputLayout();
	void BuildPSO();

public:
	ID3D12Resource* CurrentBackBuffer()const;
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView()const;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView();

public:
	std::unique_ptr<UploadBuffer<ObjectConstants>> mObjectCB = nullptr;
	ComPtr<ID3D12PipelineState> mPSO = nullptr;

public://这一部分应该写到Game里
	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);
	//void OnKeyboardInput(const GameTimer& gt);

public:
	POINT mLastMousePos;
	std::string MapLoadPath = "MapActorInfo/MapActorInfo.bat";
	static std::shared_ptr<Camera> mCamera;

};