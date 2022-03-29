#pragma once
#include "RHIResourceManager.h"
#include "Camera.h"
#include "MyStruct.h"

class RHI
{
public:
	virtual bool Init()=0;
		virtual void InitMember() = 0;
		virtual std::shared_ptr<RHIResource_Heap> CreateDescriptorHeap(std::string heapName, int NumDescriptors,int HeapType, int Flag) = 0;
		virtual void InsertHeapToHeapLib(std::string heapName, std::shared_ptr<RHIResource_Heap> heap) = 0;
		virtual void ResetCommandList() = 0;
		virtual void LoadDDSTextureToResource(std::wstring Path,int TextureIndex) = 0;
		virtual void SetDescriptorHeaps() = 0;
		virtual void BuildRootSignature() = 0;
		virtual void SetShader(std::wstring ShaderPath) = 0;
		virtual void InitPSO() = 0;
		virtual void BuildShadow() = 0;
		virtual void LoadMeshAndSetBuffer() = 0;
		virtual void CreateVBIB() = 0;
		virtual void Execute() = 0;

	virtual void Update()=0;
		//virtual void UpdateMVP(int Index, ObjectConstants& objConstants) = 0;
		//virtual void UpdateTime(ObjectConstants& objConstants) = 0;
		//virtual void UploadConstant(int offset, ObjectConstants& objConstants) = 0;
		virtual void CalculateFrameStats()=0;
	virtual void Draw() = 0;
		virtual void DrawSceneToShadowMap() = 0;
		virtual void DrawReset() = 0;
		virtual void ResetViewports(int NumViewport, ScreenViewport& vp) = 0;
		virtual void ResetScissorRects(int NumRects, ScissorRect& sr) = 0;
		virtual void ResourceBarrier() = 0;
		virtual void ClearRenderTargetView(Color mClearColor,int NumRects) = 0;
		virtual void ClearDepthStencilView() = 0;
		virtual void OMSetRenderTargets() = 0;
		virtual void CommitShadowMap() = 0;
		virtual void DrawActor(int ActorIndex,int TextureIndex) = 0;
		virtual void DrawFinal() = 0;

public:
	virtual std::shared_ptr<RHIResourceManager> GetResource() = 0;
	virtual std::shared_ptr<Camera> GetCamera() = 0;

public:
	std::shared_ptr<RHIResourceManager> mRHIResourceManager;
	std::shared_ptr<RHIResourceManager> GetRHIResourceManager();

};

