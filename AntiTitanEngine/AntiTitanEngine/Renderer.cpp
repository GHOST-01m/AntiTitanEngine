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
	mCamera->SetLens(0.25f * MathHelper::Pi, static_cast<float>(mClientWidth) / mClientHeight, 1.0f, 100000.0f);
	mRHI = std::make_shared<DXRHI>();
	mRHI->InitPrimitiveManagerMember();
	CreateHeap();
	mRHI->WaitCommandComplete();
	mRHI->ResetCommandList();
	mRHI->resizeSwapChain();
	mRHI->BuildRenderTarget();
	mRHI->ExecuteCommandList();
	mRHI->WaitCommandComplete();
	mRHI->SetScreenSetViewPort(0, 0, mClientWidth, mClientHeight, 0.0f, 1.0f);
	mRHI->SetScissorRect(0, 0, long(mClientWidth), long(mClientHeight));
	mRHI->ResetCommandList();
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

void Renderer::CreateHeap()
{//Heap创建

	//基础Rtv
	std::string rtvHeapName = "mRtvHeap";
	auto mRendertarget =GetRenderPrimitiveManager()->mRenderTarget;
	auto SwapChainBufferCount = std::dynamic_pointer_cast<DXRHIResource_RenderTarget>(mRendertarget)->GetSwapChainBufferCount();
	mRenderPrimitiveManager->mHeapsLib.insert(
		std::pair<std::string, std::shared_ptr<RHIResource_Heap>>(
		rtvHeapName, mRHI->CreateDescriptorHeap(rtvHeapName, SwapChainBufferCount, 2, 0)));
	//基础Dsv
	std::string dsvHeapName = "mDsvHeap";
	mRenderPrimitiveManager->mHeapsLib.insert(
		std::pair<std::string, std::shared_ptr<RHIResource_Heap>>(
			dsvHeapName, mRHI->CreateDescriptorHeap(dsvHeapName, 100, 3, 0)));

	std::string cbvHeapName = "mCbvHeap";
	mRenderPrimitiveManager->mHeapsLib.insert(
		std::pair<std::string, std::shared_ptr<RHIResource_Heap>>(
			cbvHeapName, mRHI->CreateDescriptorHeap(cbvHeapName, 10000, 0, 1)));

	std::string TextureHeapName = "mTextureHeap";
	mRenderPrimitiveManager->mHeapsLib.insert(
		std::pair<std::string, std::shared_ptr<RHIResource_Heap>>(
			TextureHeapName, mRHI->CreateDescriptorHeap(TextureHeapName, 1, 0, 0)));

	std::string ShadowSrvDescriptorHeapName = "mShadowSrvDescriptorHeap";
	mRenderPrimitiveManager->mHeapsLib.insert(
		std::pair<std::string, std::shared_ptr<RHIResource_Heap>>(
			ShadowSrvDescriptorHeapName, mRHI->CreateDescriptorHeap(ShadowSrvDescriptorHeapName, 10000, 0, 1)));

	std::string ShadowDsvDescriptorHeapName = "mShadowDsvDescriptorHeap";
	mRenderPrimitiveManager->mHeapsLib.insert(
		std::pair<std::string, std::shared_ptr<RHIResource_Heap>>(
			ShadowDsvDescriptorHeapName, mRHI->CreateDescriptorHeap(ShadowDsvDescriptorHeapName, 10000, 3, 0)));

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

