#pragma once
#include "RHIResource.h"
#include "Camera.h"
#include "MyStruct.h"
class RHI
{
public:
	virtual bool Init()=0;
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
		virtual void SetDescriptorHeapsAndGraphicsRootSignature() = 0;
		virtual void DrawActor(int ActorIndex) = 0;//这个还能拆
		virtual void DrawFinal() = 0;
public:
	virtual std::shared_ptr<RHIResource> GetResource() = 0;
	virtual std::shared_ptr<Camera> GetCamera() = 0;
	virtual void Set4xMsaaState(bool value) = 0;

public:
	std::shared_ptr<RHIResource> mRHIResource;
};

