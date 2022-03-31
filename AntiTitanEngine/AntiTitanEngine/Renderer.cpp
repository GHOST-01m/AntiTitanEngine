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
	mCamera->SetLens(0.25f * MathHelper::Pi, static_cast<float>(mClientWidth) / mClientHeight, 1.0f, 100000.0f);//这个应该放到GameLogic里
	
	mRHI = std::make_shared<DXRHI>();
	mRHI->OpenDebugLayer();
	mRHI->InitPrimitiveManagerMember();
	CreateHeap();
	mRHI->ResetCommandList();
	mRHI->ResizeSwapChain();
	mRHI->BuildRenderTarget();
	mRHI->ExecuteCommandList();
	mRHI->WaitCommandComplete();
	mRHI->SetScreenSetViewPort(0, 0, mClientWidth, mClientHeight, 0.0f, 1.0f);
	mRHI->SetScissorRect(0, 0, long(mClientWidth), long(mClientHeight));
	mRHI->ResetCommandList();

	Engine::Get()->GetAssetManager()->LoadExternalMapActor(MapActorLoadPath);//这个应该放到GameLogic里
	Engine::Get()->GetAssetManager()->mLight = std::make_shared<FLight>();//这个应该放到GameLogic里
	Engine::Get()->GetAssetManager()->mLight->LoadLightFromBat(MapLightLoadPath);//这个应该放到GameLogic里
	Engine::Get()->GetAssetManager()->mLight->InitView();//这个应该放到GameLogic里
	Engine::Get()->GetAssetManager()->mLight->InitProj();//这个应该放到GameLogic里

	mRHI->LoadDDSTextureToResource(TextureLoadPath,0);
	mRHI->BuildDescriptorHeaps();
	mRHI->BuildRootSignature();
	mRHI->BuildShadow();
	mRHI->SetShader(ShaderPath);
	CreateShader();
	CreatePipeline();
	mRHI->LoadMeshAndSetBuffer();
	mRHI->CreateVBIB();
	mRHI->ExecuteCommandList();
	mRHI->WaitCommandComplete();
	return true;
}

void Renderer::CreateHeap()
{//Heap创建

	//基础Rtv
	auto mRendertarget =GetRenderPrimitiveManager()->mRenderTarget;
	auto SwapChainBufferCount = std::dynamic_pointer_cast<DXRHIResource_RenderTarget>(mRendertarget)->GetSwapChainBufferCount();
	mRenderPrimitiveManager->InsertHeapToLib(
		"mRtvHeap",mRHI->CreateDescriptorHeap("mRtvHeap", SwapChainBufferCount, 2, 0));

	//基础Dsv
	mRenderPrimitiveManager->InsertHeapToLib(
		"mDsvHeap",mRHI->CreateDescriptorHeap("mDsvHeap", 100, 3, 0));

	mRenderPrimitiveManager->InsertHeapToLib(
		"mCbvHeap",mRHI->CreateDescriptorHeap("mCbvHeap", 10000, 0, 1));

	mRenderPrimitiveManager->InsertHeapToLib(
		"mTextureHeap",mRHI->CreateDescriptorHeap("mTextureHeap", 1, 0, 0));

	//单独给shadow用的SrvHeap，测试用
	mRenderPrimitiveManager->InsertHeapToLib(
		"mShadowSrvDescriptorHeap",mRHI->CreateDescriptorHeap("mShadowSrvDescriptorHeap", 10000, 0, 1));

	//单独给shadow用的SrvHeap，测试用
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
	auto baseShader = mRenderPrimitiveManager->GetShaderByName("color");
	mRenderPrimitiveManager->InsertPipelineToLib("basePipeline",
		mRHI->CreatePipeline("basePipeline", baseShader, 1, 0, false));

	auto shadowShader = mRenderPrimitiveManager->GetShaderByName("shadow");
	mRenderPrimitiveManager->InsertPipelineToLib("shadowPipeline",
		mRHI->CreatePipeline("shadowPipeline", shadowShader, 0, 1, true));
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
	mRHI->CommitShadowMap();
	mRHI->OMSetRenderTargets();
	mRHI->SetDescriptorHeap();

	mRHI->SetPipelineState(mRenderPrimitiveManager->GetPipelineByName("basePipeline"));
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

