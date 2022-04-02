#pragma once
#include "RHI.h"
#include "RenderPrimitiveManager.h"

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
#include "RHI.h"
#include "DXRHIResource_Heap.h"
#include "DXRHIResource_Shader.h"
#include "DXRHIResource_Texture.h"
#include "DXRHIResource_MeshBuffer.h"
#include "DXRHIResource_RenderTarget.h"
#include "DXRHIResource_Pipeline.h"
#include "DXRHIResource_GPUResource.h"

class DXRHI :public RHI
{
public:
	Microsoft::WRL::ComPtr<ID3D12Device> Getd3dDevice();
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetCommandList();
	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> GetStaticSamplers();

public:
	Microsoft::WRL::ComPtr<IDXGIFactory4> mdxgiFactory;
	Microsoft::WRL::ComPtr<IDXGISwapChain> mSwapChain;
	Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice;

	Microsoft::WRL::ComPtr<ID3D12Fence> mFence;
	UINT64 mCurrentFence = 0;

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mDirectCmdListAlloc;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;

public:
	UINT mRtvDescriptorSize = 0;
	UINT mDsvDescriptorSize = 0;
	UINT mCbvSrvUavDescriptorSize = 0;

	D3D_DRIVER_TYPE md3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
	DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	//DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;

	D3D12_VIEWPORT mScreenViewport;
	D3D12_RECT mScissorRect;

	ComPtr<ID3DBlob> mvsByteCode = nullptr;
	ComPtr<ID3DBlob> mpsByteCode = nullptr;
	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

public:
	bool Get4xMsaaState()const;
	bool      m4xMsaaState = false;    // 4X MSAA enabled
	UINT      m4xMsaaQuality = 0;      // quality level of 4X MSAA

public:
	int mClientWidth = 1920;
	int mClientHeight = 1080;

public:
	bool Init() override;
		void InitPrimitiveManagerMember() override;
		void OpenDebugLayer()override;
		std::shared_ptr<RHIResource_Heap> CreateDescriptorHeap(std::string heapName, int NumDescriptors, int HeapType, int Flag) override;//Type:0-CBVSRVUAV  1-SAMPLE  2-RTV  3-DSV  4-NUMTYPE
		void ResetCommandList() override;
		//这个LoadTexture应该Load成一个Render的资源
		void LoadDDSTextureToResource(std::wstring Path, int TextureIndex)override; //LoadTexture

		//这里BuildHeap还要重新做一下
		void BuildDescriptorHeaps() override;
		void BuildRootSignature() override;
		std::shared_ptr<RHIResource_Shader> CreateShader(std::string ShaderName, std::wstring ShaderPath) override;//InputLayout暂时写死了
		std::shared_ptr<RHIResource_Pipeline> CreatePipeline(std::string pipelineName, std::shared_ptr<RHIResource_Shader>, int NumRenderTargets, int RenderTargetType, bool isShadowPipeline) override;//暂定type0是basepipeline用的，1是shadow用的
		std::shared_ptr<RHIResource_RenderTarget> CreateRenderTarget(std::string RenderTargetName, int resourceType, int initialResourceStateType,std::shared_ptr<RHIResource_Heap>rtvHeap, std::shared_ptr<RHIResource_Heap>srvHeap, std::shared_ptr<RHIResource_Heap>dsvHeap, int SwapChainCount, float Width, float Height)override;//resourceType: 0.UNKNOW;1.BUFFER;2.TEXTURE1D;3.TEXTURE2D;4.TEXTURE3D
		void ResourceTransition(std::shared_ptr<RHIResource_GPUResource> myResource, int AfterStateType) override;//0COMMON;1DEPTH_WRITE;2RENDER_TARGET;3PRESENT;4GENERIC_READ;

		void LoadMeshAndSetBuffer()override;
		void CreateMeshBuffer()override;
		void ExecuteCommandList()override;
		void WaitCommandComplete()override;


	void InitDX_CreateCommandObjects();
	void CreateSwapChain()override;

public:
	void OnResize();
	//OnResize
		void resetRenderTarget()override;
		void ResizeSwapChain()override;
		void SetScreenSetViewPort(float Width,float Height) override;
		void SetScissorRect(long Right, long Bottom)override;


//	void Update() override;
		void CommitResourceToGPU(int elementIndex, ObjectConstants objConstants) override;
	
	//void Draw() override;
		void DrawReset() override;
		void DrawSceneToShadowMap() override;//这个函数的位置还要考虑一下,参考龙书Draw()里这个函数的位置
		void ResourceBarrier()override;
		void ClearRenderTargetView(std::shared_ptr<RHIResource_RenderTarget>renderTarget, Color mClearColor, int NumRects) override;
		void ClearDepthStencilView(std::shared_ptr<RHIResource_RenderTarget> renderTarget) override;
		void CommitShadowMap()override;
		void OMSetRenderTargets(std::shared_ptr<RHIResource_RenderTarget>renderTarget)override;
		void SetDescriptorHeap(std::shared_ptr<RHIResource_Heap> heap) override;
		void SetPipelineState(std::shared_ptr<RHIResource_Pipeline> pipeline)override;
		void DrawActor(int ActorIndex, int TextureIndex)override;
		void DrawFinal()override;

	void FlushCommandQueue();
	void SetSwapChain();
	//void LoadAsset();
	void CalculateFrameStats()override;

public:
	//void SetDescriptorHeaps();//往Heap里面灌数据
	//void BuildRootSignature();
	void BuildShadersAndInputLayout();

public:
	//ID3D12Resource* CurrentBackBuffer()const;//用新创建的RenderTarget里的这个方法
	//D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView()const;

public:
	std::unique_ptr<UploadBuffer<ObjectConstants>> mObjectCB = nullptr;

public://这一部分应该写到Game里
	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);
	//void OnKeyboardInput(const GameTimer& gt);

public:
	POINT mLastMousePos;
};