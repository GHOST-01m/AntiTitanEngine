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
	mCamera->SetLens(0.25f * MathHelper::Pi, static_cast<float>(mClientWidth) / mClientHeight, 1.0f, 100000.0f);//���Ӧ�÷ŵ�GameLogic��
	
	mRHI = std::make_shared<DXRHI>();
	mRHI->OpenDebugLayer();
	mRHI->InitPrimitiveManagerMember();
	CreateHeap();
	CreateRenderTarget();
	mRHI->ResetCommandList();
	mRHI->ExecuteCommandList();
	mRHI->WaitCommandComplete();
	//mRHI->SetScreenSetViewPort(0, 0, mClientWidth, mClientHeight, 0.0f, 1.0f);
	//mRHI->SetScissorRect(0, 0, long(mClientWidth), long(mClientHeight));
	mRHI->ResetCommandList();

	Engine::Get()->GetAssetManager()->LoadExternalMapActor(MapActorLoadPath);//���Ӧ�÷ŵ�GameLogic��
	Engine::Get()->GetAssetManager()->mLight = std::make_shared<FLight>();//���Ӧ�÷ŵ�GameLogic��
	Engine::Get()->GetAssetManager()->mLight->LoadLightFromBat(MapLightLoadPath);//���Ӧ�÷ŵ�GameLogic��
	Engine::Get()->GetAssetManager()->mLight->InitView();//���Ӧ�÷ŵ�GameLogic��
	Engine::Get()->GetAssetManager()->mLight->InitProj();//���Ӧ�÷ŵ�GameLogic��

	mRHI->LoadDDSTextureToResource(TextureLoadPath,0);
	mRHI->BuildDescriptorHeaps();
	mRHI->BuildRootSignature();
	//mRHI->BuildShadow();
	CreateShader();
	CreatePipeline();
	mRHI->LoadMeshAndSetBuffer();
	mRHI->CreateMeshBuffer();
	mRHI->ExecuteCommandList();
	mRHI->WaitCommandComplete();
	return true;
}

void Renderer::CreateHeap()
{//Heap����

	//����Rtv
	mRenderPrimitiveManager->InsertHeapToLib(
		"mRtvHeap",mRHI->CreateDescriptorHeap("mRtvHeap", 2, 2, 0));

	//����Dsv
	mRenderPrimitiveManager->InsertHeapToLib(
		"mDsvHeap",mRHI->CreateDescriptorHeap("mDsvHeap", 100, 3, 0));

	mRenderPrimitiveManager->InsertHeapToLib(
		"mCbvHeap",mRHI->CreateDescriptorHeap("mCbvHeap", 10000, 0, 1));

	mRenderPrimitiveManager->InsertHeapToLib(
		"mTextureHeap",mRHI->CreateDescriptorHeap("mTextureHeap", 1, 0, 0));

	//������shadow�õ�SrvHeap��������
	mRenderPrimitiveManager->InsertHeapToLib(
		"mShadowSrvDescriptorHeap",mRHI->CreateDescriptorHeap("mShadowSrvDescriptorHeap", 10000, 0, 1));

	//������shadow�õ�SrvHeap��������
	mRenderPrimitiveManager->InsertHeapToLib(
		"mShadowDsvDescriptorHeap",mRHI->CreateDescriptorHeap("mShadowDsvDescriptorHeap", 10000, 3, 0));
}

void Renderer::CreateShader()
{
	mRenderPrimitiveManager->InsertShaderToLib("color", 
		mRHI->CreateShader("color", ShaderPath));

	mRenderPrimitiveManager->InsertShaderToLib("shadow", 
		mRHI->CreateShader("shadow", L"Shaders\\Shadows.hlsl"));
}

void Renderer::CreatePipeline()
{
	//��������====================================================================
	auto baseShader = mRenderPrimitiveManager->GetShaderByName("color");
	mRenderPrimitiveManager->InsertPipelineToLib("basePipeline",
		mRHI->CreatePipeline("basePipeline", baseShader, 1, 0, false));

	//��Ӱ����=====================================================================
	auto shadowShader = mRenderPrimitiveManager->GetShaderByName("shadow");
	mRenderPrimitiveManager->InsertPipelineToLib("shadowPipeline",
		mRHI->CreatePipeline("shadowPipeline", shadowShader, 0, 1, true));
}

void Renderer::CreateRenderTarget()
{
	//����RenderTarget=======================================================
	auto baseRenderTarget = mRHI->CreateRenderTarget(
		"baseRenderTarget", 3 , 1 ,
		mRenderPrimitiveManager->GetHeapByName("mRtvHeap"),
		nullptr,
		mRenderPrimitiveManager->GetHeapByName("mDsvHeap"),
		2,
		mClientWidth,
		mClientHeight);
	mRenderPrimitiveManager->InsertRenderTargetToLib("baseRenderTarget", baseRenderTarget);
	
	//��ӰRenderTarget=====================================================
	auto shadowRenderTarget = mRHI->CreateRenderTarget(
		"shadowRenderTarget", 3, 1,
		nullptr,
		mRenderPrimitiveManager->GetHeapByName("mShadowSrvDescriptorHeap"),
		mRenderPrimitiveManager->GetHeapByName("mShadowDsvDescriptorHeap"),
		2,
		2048,
		2048);
	mRenderPrimitiveManager->InsertRenderTargetToLib("shadowRenderTarget", shadowRenderTarget);
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
	mRHI->CommitShadowMap();
	mRHI->SetScreenSetViewPort(1920,1080);
	mRHI->SetScissorRect(1920, 1080);
	mRHI->ResourceBarrier();
	
	mRHI->ClearRenderTargetView(mRenderPrimitiveManager->GetRenderTargetByName("baseRenderTarget"),mClearColor, 0);
	mRHI->ClearDepthStencilView(mRenderPrimitiveManager->GetRenderTargetByName("baseRenderTarget"));
	mRHI->OMSetRenderTargets(mRenderPrimitiveManager->GetRenderTargetByName("baseRenderTarget"));
	mRHI->SetPipelineState(mRenderPrimitiveManager->GetPipelineByName("basePipeline"));
	mRHI->SetDescriptorHeap(mRenderPrimitiveManager->GetHeapByName("mCbvHeap"));
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

