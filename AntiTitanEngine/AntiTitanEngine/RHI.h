#pragma once
#include "RenderPrimitiveManager.h"
#include "Camera.h"
#include "MyStruct.h"

class RHI
{
public:
	virtual bool Init()=0;
		virtual void InitPrimitiveManagerMember() = 0;
		virtual void OpenDebugLayer()=0;
		virtual std::shared_ptr<RHIResource_Heap> CreateDescriptorHeap(std::string heapName, int NumDescriptors,int HeapType, int Flag) = 0;
		virtual void ResetCommandList() = 0;
		virtual void LoadDDSTextureToResource(std::wstring Path,int TextureIndex) = 0;
		virtual void BuildDescriptorHeaps() = 0;
		virtual void BuildRootSignature() = 0;
		virtual std::shared_ptr<RHIResource_Shader> CreateShader(std::string ShaderName, std::wstring ShaderPath) = 0;//InputLayout暂时写死了
		virtual std::shared_ptr<RHIResource_Pipeline> CreatePipeline(std::string pipelineName, std::shared_ptr<RHIResource_Shader>,int NumRenderTargets,int RenderTargetType,bool isShadowPipeline) = 0;//暂定type0是basepipeline用的，1是shadow用的
		virtual std::shared_ptr<RHIResource_RenderTarget> CreateRenderTarget(std::string RenderTargetName, int resourceType,std::shared_ptr<RHIResource_Heap>rtvHeap, std::shared_ptr<RHIResource_Heap>srvHeap, std::shared_ptr<RHIResource_Heap>dsvHeap,int SwapChainCount,float Width,float Height)=0;//resourceType: 0.UNKNOW;1.BUFFER;2.TEXTURE1D;3.TEXTURE2D;4.TEXTURE3D
		virtual void ResourceTransition(std::shared_ptr<RHIResource_RenderTarget> renderTarget,int BeforeStateType, int AfterStateType) = 0;
		virtual void BuildShadow() = 0;
		virtual void LoadMeshAndSetBuffer() = 0;
		virtual void CreateVBIB() = 0;
		virtual void ExecuteCommandList() = 0;
		virtual void WaitCommandComplete() = 0;

	//OnResize
		virtual void resetRenderTarget() = 0;
		virtual void ResizeSwapChain() = 0;
		virtual void BuildRenderTarget() = 0;
		virtual void SetScreenSetViewPort(float TopLeftX, float TopLeftY, float Width, float Height,float MinDepth,float MaxDepth) = 0;
		virtual void SetScissorRect(long Left, long Top, long Right, long Bottom) = 0;


	virtual void Update()=0;
		//virtual void UpdateMVP(int Index, ObjectConstants& objConstants) = 0;
		//virtual void UpdateTime(ObjectConstants& objConstants) = 0;
		//virtual void UploadConstant(int offset, ObjectConstants& objConstants) = 0;
		virtual void CalculateFrameStats()=0;
	//virtual void Draw() = 0;
		virtual void DrawSceneToShadowMap() = 0;
		virtual void DrawReset() = 0;
		virtual void ResetViewports(int NumViewport, ScreenViewport& vp) = 0;
		virtual void ResetScissorRects(int NumRects, ScissorRect& sr) = 0;
		virtual void ResourceBarrier() = 0;
		virtual void ClearRenderTargetView(Color mClearColor,int NumRects) = 0;
		virtual void ClearDepthStencilView() = 0;
		virtual void OMSetRenderTargets() = 0;
		virtual void SetDescriptorHeap(std::shared_ptr<RHIResource_Heap> heap) = 0;
		virtual void SetPipelineState(std::shared_ptr<RHIResource_Pipeline> pipeline) = 0;
		virtual void CommitShadowMap() = 0;
		virtual void DrawActor(int ActorIndex,int TextureIndex) = 0;
		virtual void DrawFinal() = 0;
};

