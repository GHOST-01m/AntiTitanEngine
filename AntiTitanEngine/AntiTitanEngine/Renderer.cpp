#include "stdafx.h"
#include "Renderer.h"
#include "DXRHIResource.h"

Renderer::Renderer()
{
}

Renderer::~Renderer()
{
}

bool Renderer::Init()
{
	mRHI = std::make_shared<DXRHI>();
	return mRHI->Init();
}

void Renderer::Update()
{
	//mRHI->Update();
	for (int i = 0; i < Engine::Get()->GetAssetManager()->GetMapActorInfo()->Size(); i++) {
		mRHI->UpdateMVP(i,objConstants);
		mRHI->UpdateTime(objConstants);
		mRHI->UploadConstant(i,objConstants);
	}
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

