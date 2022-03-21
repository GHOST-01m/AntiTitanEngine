#pragma once
#include "RHIResourceManager.h"
#include "Camera.h"
#include "MyStruct.h"
#include "RenderResourceManager.h"


class RHI
{
public:
	virtual bool Init()=0;
		virtual void InitMember() = 0;
		virtual void LoadExternalMapActor(std::string Path) = 0;
		virtual void LoadTexture(std::wstring Path) = 0;
		virtual void BuildTexture(std::string Name,std::wstring Path) = 0;
		virtual void BuildMember() = 0;
		virtual void SetShader(std::wstring ShaderPath) = 0;
		virtual void InitPSO() = 0;
		virtual void BuildPSO() = 0;
		virtual void LoadMeshAndSetBuffer() = 0;
		virtual void CreateVBIB() = 0;
		virtual void Execute() = 0;
		//virtual std::shared_ptr<RHIFactory> CreateFactory(std::shared_ptr<RenderResource_Factory> mFactory)=0;
		//virtual void SetFactory(std::shared_ptr<RHIFactory> mFactory)=0;
		//virtual std::shared_ptr<RHIDevice> CreateDevice(std::shared_ptr<RenderResource_Device> mDevice) = 0;
		//virtual void SetDevice(std::shared_ptr<RHIDevice> mDevice)=0;
		//virtual std::shared_ptr<RHIFence> CreateFence(std::shared_ptr<RenderResource_Fence> mFence) = 0;
		//virtual void SetFence(std::shared_ptr<RHIFence> mFence) = 0;
		//virtual void SetRtvSize() = 0;
		//virtual void SetDsvSize() = 0;
		//virtual void SetCbvSrvUavSize() = 0;
		//virtual void SetMultisampleQualityLevels(int SampleCount, int NumQualityLevels)=0;
		//virtual void SetCommandObjects() = 0;
		////virtual void SetSwapChain(bool m4xMsaaState, int m4xMsaaQuality, int Width, int Height, int RefreshNumerator, int RefreshDenominator, int SwapChainBufferCount) = 0;
		//virtual void SetRtvAndDsvDescriptorHeaps() = 0;
		//virtual void ResetCommandList() = 0;
		//virtual void OnNewResize()=0;
		//virtual void FinalInit()=0;

	virtual void Update()=0;
		virtual void UpdateMVP(int Index, ObjectConstants& objConstants) = 0;
		virtual void UpdateTime(ObjectConstants& objConstants) = 0;
		virtual void UploadConstant(int offset, ObjectConstants& objConstants) = 0;

	virtual void Draw() = 0;

		virtual void DrawReset() = 0;
		virtual void ResetViewports(int NumViewport, ScreenViewport& vp) = 0;
		virtual void ResetScissorRects(int NumRects, ScissorRect& sr) = 0;
		virtual void ResourceBarrier() = 0;
		virtual void ClearRenderTargetView(Color mClearColor,int NumRects) = 0;
		virtual void ClearDepthStencilView() = 0;
		virtual void OMSetRenderTargets() = 0;
		virtual void DrawActor(int ActorIndex,int TextureIndex) = 0;
		virtual void DrawFinal() = 0;

public:
	virtual std::shared_ptr<RHIResourceManager> GetResource() = 0;
	virtual std::shared_ptr<Camera> GetCamera() = 0;
	virtual void Set4xMsaaState(bool value) = 0;

public:
	std::shared_ptr<RHIResourceManager> mRHIResourceManager;
};

