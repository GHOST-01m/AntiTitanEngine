#include "stdafx.h"
#include "Renderer.h"
#include "DXRHIResourceManager.h"

Renderer::Renderer()
{
}

Renderer::~Renderer()
{
}

bool Renderer::Init()
{
	mRenderResourceManager = std::make_shared<RenderResourceManager>();

	mRHI = std::make_shared<DXRHI>();
	//mRenderResourceManager->mFactory = std::make_shared<RenderResource_Factory>();
	//mRenderResourceManager->mDevice = std::make_shared<RenderResource_Device>();
	//mRenderResourceManager->mFence = std::make_shared<RenderResource_Fence>();

	mRHI->InitMember();
	//创建Heap在这里创建,方法已经写好了，从InitMember下面copy过来就可以
	//OnResize拆开


	mRHI->LoadExternalMapActor(MapActorLoadPath);
	mRHI->LoadLightInfo(MapLightLoadPath);
	mRHI->LoadTexture(TextureLoadPath,0);
	//mRHI->BuildTexture("SkySphere", TextureLoadPath);
	mRHI->BuildMember();
	mRHI->BuildShadow();
	mRHI->SetShader(ShaderPath);
	//mRHI->BuildPSO();
	mRHI->InitPSO() ;
	mRHI->LoadMeshAndSetBuffer();
	mRHI->CreateVBIB();
	mRHI->Execute();

	return true;
	//return mRHI->Init();

}

void Renderer::Update()
{
	mRHI->Update();

	mRHI->CalculateFrameStats();
}

void Renderer::Draw()
{
	//mRHI->Draw();
	mRHI->DrawReset();
	mRHI->DrawSceneToShadowMap();
	mRHI->ResetViewports(1, mViewport);
	mRHI->ResetScissorRects(1, mScissorRect);
	mRHI->ResourceBarrier();
	mRHI->ClearRenderTargetView(mClearColor, 0);
	mRHI->ClearDepthStencilView();
	mRHI->OMSetRenderTargets();
	for (int ActorIndex = 0; ActorIndex < Engine::Get()->GetAssetManager()->GetMapActorInfo()->Size(); ActorIndex++)
	{
		mRHI->DrawActor(ActorIndex,0);
	}
	mRHI->DrawFinal();
}

void Renderer::Destroy()
{
	mRHI = nullptr;
}

std::shared_ptr<RHI> Renderer::GetRHI()
{
	return mRHI;
}

void Renderer::CalculateFrameStats()
{
}

void Renderer::Set4xMsaaState(bool value)
{
	mRHI->Set4xMsaaState(value);
}

std::shared_ptr<Camera> Renderer::GetCamera()
{
	return mRHI->GetCamera();
}

