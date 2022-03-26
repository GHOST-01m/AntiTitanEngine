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
		virtual void LoadLightInfo(std::string Path) = 0;
		virtual void LoadTexture(std::wstring Path,int TextureIndex) = 0;
		virtual void BuildShadow() = 0;
		virtual void BuildTexture(std::string Name,std::wstring Path) = 0;
		virtual void BuildMember() = 0;
		virtual void SetShader(std::wstring ShaderPath) = 0;
		virtual void InitPSO() = 0;
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
		virtual void DrawActor(int ActorIndex,int TextureIndex) = 0;
		virtual void DrawFinal() = 0;

public:
	virtual std::shared_ptr<RHIResourceManager> GetResource() = 0;
	virtual std::shared_ptr<Camera> GetCamera() = 0;
	virtual void Set4xMsaaState(bool value) = 0;

public:
	std::shared_ptr<RHIResourceManager> mRHIResourceManager;
};

