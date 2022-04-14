#pragma once
#include "RenderPrimitiveManager.h"
#include "Camera.h"
#include "MyStruct.h"
#include "PipelineState_DESC.h"

class RHI
{
public:
	virtual bool Init()=0;
		virtual void InitPrimitiveManagerMember() = 0;
		virtual void OpenDebugLayer()=0;
		virtual std::shared_ptr<Primitive_Heap> CreateDescriptorHeap(std::string heapName, int NumDescriptors,int HeapType, int Flag) = 0;//Type:0-CBVSRVUAV  1-SAMPLE  2-RTV  3-DSV  4-NUMTYPE
		virtual void ResetCommandList() = 0;
		virtual void LoadDDSTextureToResource(std::wstring Path, std::shared_ptr<Primitive_Texture>texture) = 0;
		virtual void BuildDescriptorHeaps() = 0;
		virtual void BuildRootSignature() = 0;
		virtual std::shared_ptr<Primitive_Shader> CreateShader(std::string ShaderName, std::wstring ShaderPath) = 0;//InputLayout暂时写死了
		virtual std::shared_ptr<Primitive_Pipeline> CreatePipeline(std::string pipelineName, std::shared_ptr<Primitive_Shader>,int NumRenderTargets,int RenderTargetType,bool isShadowPipeline) = 0;//暂定type0是basepipeline用的，1是shadow用的
		virtual std::shared_ptr<Primitive_RenderTarget> CreateRenderTarget(std::string RenderTargetName, int resourceDimension,int initialResourceStateType, int ResourceFormat,std::shared_ptr<Primitive_Heap>rtvHeap, int rtvOffset,std::shared_ptr<Primitive_Heap>srvHeap, int srvOffset, std::shared_ptr<Primitive_Heap>dsvHeap, int dsvOffset, bool rtvBindToSwapChain,int SwapChainCount,float Width,float Height)=0;//resourceType: 0.UNKNOW;1.BUFFER;2.TEXTURE1D;3.TEXTURE2D;4.TEXTURE3D
		virtual std::shared_ptr<Primitive_MeshBuffer> CreateMeshBuffer(std::shared_ptr<StaticMesh> mesh) = 0;
		virtual std::shared_ptr<Primitive_Texture>CreateTexture(std::string,std::wstring Path, int currentHeapOffset) = 0;

		virtual void ResourceTransition(std::shared_ptr<Primitive_GPUResource> myResource, int AfterStateType) = 0;//0COMMON;1DEPTH_WRITE;2RENDER_TARGET;3PRESENT;4GENERIC_READ;
		//virtual void BuildShadow() = 0;
		virtual void CreateSwapChain() = 0;
		//virtual void LoadMeshAndSetBuffer() = 0;
		//virtual void CreateMeshBuffers() = 0;
		virtual void ExecuteCommandList() = 0;
		virtual void WaitCommandComplete() = 0;

	//OnResize
		virtual void resetRenderTarget() = 0;
		virtual void ResizeSwapChain() = 0;
		virtual void SetScreenSetViewPort(float Width, float Height) = 0;
		virtual void SetScissorRect(long Right, long Bottom) = 0;


//	virtual void Update()=0;
		virtual void CommitResourceToGPU(int elementIndex, ObjectConstants objConstants) = 0;
		virtual void CalculateFrameStats()=0;
	//virtual void Draw() = 0;
		//virtual void DrawSceneToShadowMap() = 0;
		virtual void DrawReset() = 0;
		//virtual void ResetViewports(int NumViewport, ScreenViewport& vp) = 0;
		//virtual void ResetScissorRects(int NumRects, ScissorRect& sr) = 0;
		virtual void ResourceBarrier() = 0;
		virtual void ClearRenderTargetView(std::shared_ptr<Primitive_RenderTarget>renderTarget, Color mClearColor,int NumRects) = 0;
		virtual void ClearRenderTarget(std::shared_ptr<Primitive_RenderTarget>, std::string heapName)=0;
		virtual void ClearDepthStencilView(std::shared_ptr<Primitive_RenderTarget> renderTarget) = 0;
		virtual void OMSetRenderTargets(std::shared_ptr<Primitive_RenderTarget>renderTarget) = 0;
		virtual void SetDescriptorHeap(std::shared_ptr<Primitive_Heap> heap) = 0;
		virtual void SetPipelineState(std::shared_ptr<Primitive_Pipeline> pipeline) = 0;
		virtual void CommitShaderParameters() = 0;
		virtual void DrawMesh(int ActorIndex,int TextureIndex) = 0;
		virtual void DrawFinal() = 0;


		virtual std::shared_ptr<Primitive_MeshBuffer> CreateTriangleMeshBuffer()=0;
		virtual void BuildCBVHeapForTirangle() = 0;

		virtual void BuildTriangleAndDraw(std::shared_ptr<Primitive_MeshBuffer> Triangle) = 0;
};

