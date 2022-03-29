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

	mRHI->InitMember();
	//����Heap�����ﴴ��,�����Ѿ�д���ˣ���InitMember����copy�����Ϳ���
	//OnResize��


	Engine::Get()->GetAssetManager()->LoadExternalMapActor(MapActorLoadPath);
	Engine::Get()->GetAssetManager()->mLight = std::make_shared<FLight>();
	Engine::Get()->GetAssetManager()->mLight->LoadLightFromBat(MapLightLoadPath);
	Engine::Get()->GetAssetManager()->mLight->InitView();
	Engine::Get()->GetAssetManager()->mLight->InitProj();
	mRHI->LoadDDSTextureToResource(TextureLoadPath,0);
	//mRHI->BuildTexture("SkySphere", TextureLoadPath);
	//mRHI->BuildMember();
	mRHI->SetDescriptorHeaps();
	mRHI->BuildRootSignature();
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

std::shared_ptr<Camera> Renderer::GetCamera()
{
	return mRHI->GetCamera();
}

