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
	mRHI->SetDescriptorHeaps();
	mRHI->BuildRootSignature();
	mRHI->BuildShadow();
	mRHI->SetShader(ShaderPath);
	mRHI->InitPSO() ;
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
	mRenderPrimitiveManager->InsertHeapToHeapLib(
		"mRtvHeap",mRHI->CreateDescriptorHeap("mRtvHeap", SwapChainBufferCount, 2, 0));

	//基础Dsv
	mRenderPrimitiveManager->InsertHeapToHeapLib(
		"mDsvHeap",mRHI->CreateDescriptorHeap("mDsvHeap", 100, 3, 0));

	mRenderPrimitiveManager->InsertHeapToHeapLib(
		"mCbvHeap",mRHI->CreateDescriptorHeap("mCbvHeap", 10000, 0, 1));

	mRenderPrimitiveManager->InsertHeapToHeapLib(
		"mTextureHeap",mRHI->CreateDescriptorHeap("mTextureHeap", 1, 0, 0));

	//单独给shadow用的SrvHeap，测试用
	mRenderPrimitiveManager->InsertHeapToHeapLib(
		"mShadowSrvDescriptorHeap",mRHI->CreateDescriptorHeap("mShadowSrvDescriptorHeap", 10000, 0, 1));

	//单独给shadow用的SrvHeap，测试用
	mRenderPrimitiveManager->InsertHeapToHeapLib(
		"mShadowDsvDescriptorHeap",mRHI->CreateDescriptorHeap("mShadowDsvDescriptorHeap", 10000, 3, 0));
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

