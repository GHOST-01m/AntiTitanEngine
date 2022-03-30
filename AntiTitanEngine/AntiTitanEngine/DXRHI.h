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
#include "DXRHIResource_ShadowMap.h"
#include "DXRHIResource_Texture.h"
#include "DXRHIResource_MeshBuffer.h"
#include "DXRHIResource_RenderTarget.h"

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
	DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

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
		std::shared_ptr<RHIResource_Heap> CreateDescriptorHeap(std::string heapName, int NumDescriptors, int HeapType, int Flag) override;//Type:0-CBVSRVUAV  1-SAMPLE  2-RTV  3-DSV  4-NUMTYPE
		void ResetCommandList() override;
		//����������һ��OnResize
		//���LoadTextureӦ��Load��һ��Render����Դ
		void LoadDDSTextureToResource(std::wstring Path, int TextureIndex)override; //LoadTexture

		//����SetHeap��Ҫ������һ��
		void SetDescriptorHeaps() override;
		void BuildRootSignature() override;
		void SetShader(std::wstring ShaderPath)override;

		//���ߴ������
		void InitPSO()override;

		//BuildShadow��Ķ���ʵ�����ǵ���֮ǰ�����õļ��ַ�������������
		void BuildShadow()override;
		void LoadMeshAndSetBuffer()override;
		void CreateVBIB()override;
		void ExecuteCommandList()override;
		void WaitCommandComplete()override;


	void InitDX_CreateCommandObjects();
	void InitDX_CreateSwapChain();

public:
	void OnResize();
	//OnResize
		void resetRenderTarget()override;
		void resizeSwapChain()override;
		void BuildRenderTarget()override;
		void SetScreenSetViewPort(
			float TopLeftX, float TopLeftY, 
			float Width,    float Height,
			float MinDepth, float MaxDepth) override;
		void SetScissorRect(
			long Left, long Top,
			long Right, long Bottom)override;


	void Update() override;
		 //void UpdateMVP(int Index, ObjectConstants& objConstants) override;
		 //void UpdateTime(ObjectConstants& objConstants) override;
		 //void UploadConstant(int offset, ObjectConstants& objConstants) override;
	void Draw() override;
		void DrawReset() override;
		void DrawSceneToShadowMap() override;//���������λ�û�Ҫ����һ��,�ο�����Draw()�����������λ��
		void ResetViewports(int NumViewport, ScreenViewport& vp) override;
		void ResetScissorRects(int NumRects,ScissorRect& sr)override;
		void ResourceBarrier()override;
		void ClearRenderTargetView(Color mClearColor, int NumRects) override;
		void ClearDepthStencilView() override;
		void OMSetRenderTargets()override;
		void CommitShadowMap()override;
		void DrawActor(int ActorIndex, int TextureIndex)override;
		void DrawFinal()override;

	void FlushCommandQueue();
	void SetSwapChain();
	//void LoadAsset();
	void CalculateFrameStats()override;

public:
	//void SetDescriptorHeaps();//��Heap���������
	//void BuildRootSignature();
	void BuildShadersAndInputLayout();
	void BuildPSO();

public:
	ID3D12Resource* CurrentBackBuffer()const;
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView()const;


public:
	std::unique_ptr<UploadBuffer<ObjectConstants>> mObjectCB = nullptr;
	ComPtr<ID3D12PipelineState> mPSO = nullptr;
	ComPtr<ID3D12PipelineState> mShadowMapPSO = nullptr;

public://��һ����Ӧ��д��Game��
	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);
	//void OnKeyboardInput(const GameTimer& gt);

public:
	POINT mLastMousePos;
};