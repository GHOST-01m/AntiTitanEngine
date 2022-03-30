#include "stdafx.h"
#include "Renderer.h"

Renderer::Renderer()
{
}

Renderer::~Renderer()
{
}

std::shared_ptr<RenderPrimitiveManager> Renderer::GetRenderPrimitiveManager()
{
	return mRenderPrimitiveManager;
}

bool Renderer::Init()
{
	mRenderPrimitiveManager = std::make_shared<RenderPrimitiveManager>();
	mCamera = std::make_shared<Camera>();
	mRHI = std::make_shared<DXRHI>();
	mRHI->InitPrimitiveManagerMember();

	mCamera->SetLens(0.25f * MathHelper::Pi, static_cast<float>(mClientWidth) / mClientHeight, 1.0f, 100000.0f);

	//创建Heap在这里创建,方法已经写好了，从InitMember下面copy过来就可以
	//OnResize拆开

	Engine::Get()->GetAssetManager()->LoadExternalMapActor(MapActorLoadPath);
	Engine::Get()->GetAssetManager()->mLight = std::make_shared<FLight>();
	Engine::Get()->GetAssetManager()->mLight->LoadLightFromBat(MapLightLoadPath);
	Engine::Get()->GetAssetManager()->mLight->InitView();
	Engine::Get()->GetAssetManager()->mLight->InitProj();
	mRHI->LoadDDSTextureToResource(TextureLoadPath,0);
	mRHI->SetDescriptorHeaps();
	mRHI->BuildRootSignature();
	mRHI->BuildShadow();
	mRHI->SetShader(ShaderPath);
	//mRHI->BuildPSO();
	mRHI->InitPSO() ;
	mRHI->LoadMeshAndSetBuffer();
	mRHI->CreateVBIB();
	mRHI->ExecuteCommandList();
	mRHI->WaitCommandComplete();
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

std::shared_ptr<Camera> Renderer::GetCamera()
{
	return mCamera;
}

