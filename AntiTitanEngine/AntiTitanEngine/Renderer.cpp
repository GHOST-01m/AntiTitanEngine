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
	mRHI = std::make_shared<DXRHI>();
	//mRenderResourceManager = std::make_shared<RenderResourceManager>();
	//mRenderResourceManager->mFactory = std::make_shared<RenderResource_Factory>();
	//mRenderResourceManager->mDevice = std::make_shared<RenderResource_Device>();
	//mRenderResourceManager->mFence = std::make_shared<RenderResource_Fence>();

	mRHI->InitMember();
	mRHI->LoadExternalMapActor(MapActorLoadPath);
	mRHI->LoadTexture(TextureLoadPath);
	//mRHI->BuildTexture("SkySphere", TextureLoadPath);
	mRHI->BuildMember();
	mRHI->SetShader(ShaderPath) ;
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
}

void Renderer::Draw()
{
	//mRHI->Draw();
	mRHI->DrawReset();
	mRHI->ResetViewports(1, mViewport);
	mRHI->ResetScissorRects(1, mScissorRect);
	mRHI->ResourceBarrier();
	mRHI->ClearRenderTargetView(mClearColor, 0);
	mRHI->ClearDepthStencilView();
	mRHI->OMSetRenderTargets();
	mRHI->SetDescriptorHeapsAndGraphicsRootSignature();
	for (int ActorIndex = 0; ActorIndex < Engine::Get()->GetAssetManager()->GetMapActorInfo()->Size(); ActorIndex++)
	{
		mRHI->DrawActor(ActorIndex);
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

