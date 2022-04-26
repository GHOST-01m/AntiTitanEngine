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

std::shared_ptr<Scene> Renderer::GetScene()
{
	return mScene;
}

bool Renderer::Init()
{
	Engine::Get()->GetAssetManager()->LoadExternalMapActor(MapActorLoadPath);
	mRenderPrimitiveManager = std::make_shared<RenderPrimitiveManager>();
	mCamera = std::make_shared<Camera>();
	mCamera->SetLens(0.25f * MathHelper::Pi, static_cast<float>(mClientWidth) / mClientHeight, 1.0f, 100000.0f);
	
	mRHI = std::make_shared<DXRHI>();
	mRHI->OpenDebugLayer();
	mRHI->InitPrimitiveManagerMember();
	CreateHeap();
	CreateRenderTarget();
	mRHI->ResetCommandList();
	mRHI->ExecuteCommandList();
	mRHI->WaitCommandComplete();
	mRHI->ResetCommandList();
	mRHI->BuildDescriptorHeaps();
	mRHI->BuildRootSignature();
	CreateShader();
	CreatePipeline();
	LoadMeshAndSetBuffer();
	
	std::shared_ptr<StaticMesh> triangle = std::make_shared<StaticMesh>();
	triangle->meshBuffer = mRHI->CreateTriangleMeshBuffer();
	Engine::Get()->GetAssetManager()->InsertStaticMeshToLib("triangle", triangle);

	mRHI->ExecuteCommandList();
	mRHI->WaitCommandComplete();
	return true;
}

void Renderer::CreateHeap()
{//Heap创建

	//base===============================================================================

	//基础Rtv
	mRenderPrimitiveManager->InsertHeapToLib(
		"mRtvHeap",mRHI->CreateDescriptorHeap("mRtvHeap", 100, RTV, NONE));

	//基础Dsv
	mRenderPrimitiveManager->InsertHeapToLib(
		"mDsvHeap",mRHI->CreateDescriptorHeap("mDsvHeap", 100, DSV, NONE));

	mRenderPrimitiveManager->InsertHeapToLib(
		"mCbvHeap",mRHI->CreateDescriptorHeap("mCbvHeap", 10000, CBVSRVUAV, SHADER_VISIBLE));
	//shadow===============================================================================

	//单独给shadow用的SrvHeap
	mRenderPrimitiveManager->InsertHeapToLib(
		"mShadowSrvDescriptorHeap",mRHI->CreateDescriptorHeap("mShadowSrvDescriptorHeap", 10000, CBVSRVUAV, SHADER_VISIBLE));

	//单独给shadow用的SrvHeap
	mRenderPrimitiveManager->InsertHeapToLib(
		"mShadowDsvDescriptorHeap",mRHI->CreateDescriptorHeap("mShadowDsvDescriptorHeap", 10000, DSV, NONE));

	//bloom===============================================================================
	//BloomRtvHeap
	mRenderPrimitiveManager->InsertHeapToLib(
		"BloomRtvHeap", mRHI->CreateDescriptorHeap("BloomRtvHeap", 1000, RTV, NONE));
	//BloomDsvHeap
	mRenderPrimitiveManager->InsertHeapToLib(
		"BloomDsvHeap", mRHI->CreateDescriptorHeap("BloomDsvHeap", 1000, DSV, NONE));
	//BloomSrvHeap
	mRenderPrimitiveManager->InsertHeapToLib(
		"BloomSrvHeap", mRHI->CreateDescriptorHeap("BloomSrvHeap", 10000, CBVSRVUAV, SHADER_VISIBLE));
}

void Renderer::CreateShader()
{
	mRenderPrimitiveManager->InsertShaderToLib("color", 
		mRHI->CreateShader("color", ShaderPath));

	mRenderPrimitiveManager->InsertShaderToLib("shadow", 
		mRHI->CreateShader("shadow", L"Shaders\\Shadows.hlsl"));

	mRenderPrimitiveManager->InsertShaderToLib("HDR",
		mRHI->CreateShader("HDR", L"Shaders\\HDR.hlsl"));

	mRenderPrimitiveManager->InsertShaderToLib("HDRShadowHighLightLess",
		mRHI->CreateShader("HDRShadowHighLightLess", L"Shaders\\HDRShadowHighLightLess.hlsl"));

	mRenderPrimitiveManager->InsertShaderToLib("bloomSetup",
		mRHI->CreateShader("bloomSetup", L"Shaders\\bloomSetup.hlsl"));
	
	mRenderPrimitiveManager->InsertShaderToLib("bloomDown",
		mRHI->CreateShader("bloomDown", L"Shaders\\bloomDown1.hlsl"));

	mRenderPrimitiveManager->InsertShaderToLib("bloomUp",
		mRHI->CreateShader("bloomUp", L"Shaders\\bloomUp.hlsl"));

	mRenderPrimitiveManager->InsertShaderToLib("bloomMerge",
		mRHI->CreateShader("bloomMerge", L"Shaders\\bloomMerge.hlsl"));

	mRenderPrimitiveManager->InsertShaderToLib("ToneMap",
		mRHI->CreateShader("ToneMap", L"Shaders\\ToneMap.hlsl"));

	mRenderPrimitiveManager->InsertShaderToLib("TestPostProcess",
		mRHI->CreateShader("TestPostProcess", L"Shaders\\TestPostProcess.hlsl"));

	mRenderPrimitiveManager->InsertShaderToLib("EdgeDetectionSobel",
		mRHI->CreateShader("EdgeDetectionSobel", L"Shaders\\EdgeDetectionSobel.hlsl"));

	mRenderPrimitiveManager->InsertShaderToLib("HDRUseTextureShader",
		mRHI->CreateShader("HDRUseTextureShader", L"Shaders\\HDRUseTexture.hlsl"));
}

void Renderer::CreatePipeline()
{
	//基础管线====================================================================
	auto baseShader = mRenderPrimitiveManager->GetShaderByName("color");
	mRenderPrimitiveManager->InsertPipelineToLib("basePipeline",
		mRHI->CreatePipeline("basePipeline", baseShader, 1, RenderTargetFormat_R8G8B8A8_UNORM, false));

	//阴影管线=====================================================================
	auto shadowShader = mRenderPrimitiveManager->GetShaderByName("shadow");
	mRenderPrimitiveManager->InsertPipelineToLib("shadowPipeline",
		mRHI->CreatePipeline("shadowPipeline", shadowShader, 0, RenderTargetFormat_UNKNOWN, true));

	//HDR管线===================================================================
	auto HDRShader = mRenderPrimitiveManager->GetShaderByName("HDR");
	mRenderPrimitiveManager->InsertPipelineToLib("HDRPipeline",
		mRHI->CreatePipeline("HDRPipeline", HDRShader, 1, RenderTargetFormat_R16G16B16A16_FLOAT, false));

	auto HDRHDRShadowHighLightLessShader = mRenderPrimitiveManager->GetShaderByName("HDRShadowHighLightLess");
	mRenderPrimitiveManager->InsertPipelineToLib("HDRShadowHighLightLessPipeline",
		mRHI->CreatePipeline("HDRShadowHighLightLessPipeline", HDRHDRShadowHighLightLessShader, 1, RenderTargetFormat_R16G16B16A16_FLOAT, false));
	//Bloom管线===================================================================

	auto bloomSetUpShader = mRenderPrimitiveManager->GetShaderByName("bloomSetup");
	mRenderPrimitiveManager->InsertPipelineToLib("bloomSetupPipeline",
		mRHI->CreatePipeline("bloomSetupPipeline", bloomSetUpShader, 1, RenderTargetFormat_R16G16B16A16_FLOAT, false));

	auto bloomDownShader = mRenderPrimitiveManager->GetShaderByName("bloomDown");
	mRenderPrimitiveManager->InsertPipelineToLib("bloomDownPipeline",
		mRHI->CreatePipeline("bloomDownPipeline", bloomDownShader, 1, RenderTargetFormat_R16G16B16A16_FLOAT, false));
	
	auto bloomUpShader = mRenderPrimitiveManager->GetShaderByName("bloomUp");
	mRenderPrimitiveManager->InsertPipelineToLib("bloomUpPipeline",
		mRHI->CreatePipeline("bloomUpPipeline", bloomUpShader, 1, RenderTargetFormat_R16G16B16A16_FLOAT, false));

	auto bloomMergeShader = mRenderPrimitiveManager->GetShaderByName("bloomMerge");
	mRenderPrimitiveManager->InsertPipelineToLib("bloomMergePipeline",
		mRHI->CreatePipeline("bloomMergePipeline", bloomMergeShader, 1, RenderTargetFormat_R16G16B16A16_FLOAT, false));
	
	auto ToneMapShader = mRenderPrimitiveManager->GetShaderByName("ToneMap");
	mRenderPrimitiveManager->InsertPipelineToLib("ToneMapPipeline",
		mRHI->CreatePipeline("ToneMapPipeline", ToneMapShader, 1, RenderTargetFormat_R8G8B8A8_UNORM, false));

	auto TestPostProcessShader = mRenderPrimitiveManager->GetShaderByName("TestPostProcess");
	mRenderPrimitiveManager->InsertPipelineToLib("TestPostProcessPipeline",
		mRHI->CreatePipeline("TestPostProcessPipeline", TestPostProcessShader, 1, RenderTargetFormat_R8G8B8A8_UNORM, false));
	
		auto EdgeDetectionSobelShader = mRenderPrimitiveManager->GetShaderByName("EdgeDetectionSobel");
	mRenderPrimitiveManager->InsertPipelineToLib("EdgeDetectionSobelPipeline",
		mRHI->CreatePipeline("EdgeDetectionSobelPipeline", EdgeDetectionSobelShader, 1, RenderTargetFormat_R8G8B8A8_UNORM, false));

		auto HDRUseTextureShader = mRenderPrimitiveManager->GetShaderByName("HDRUseTextureShader");
	mRenderPrimitiveManager->InsertPipelineToLib("HDRUseTexturePipeline",
		mRHI->CreatePipeline("HDRUseTexturePipeline", HDRUseTextureShader, 1, RenderTargetFormat_R8G8B8A8_UNORM, false));
}

void Renderer::CreateRenderTarget()
{
	//基础RenderTarget=======================================================
	{
	auto baseRenderTarget = mRHI->CreateRenderTarget(
		"baseRenderTarget", TEXTURE2D, R8G8B8A8_UNORM,
		mRenderPrimitiveManager->GetHeapByName("mRtvHeap"),
		nullptr,
		mRenderPrimitiveManager->GetHeapByName("mDsvHeap"),
		true, 2, mClientWidth,mClientHeight);
	mRenderPrimitiveManager->InsertRenderTargetToLib("baseRenderTarget", baseRenderTarget);
	}
	//阴影RenderTarget=====================================================
	{
	auto shadowRenderTarget = mRHI->CreateRenderTarget(
		"shadowRenderTarget", TEXTURE2D,  D24_UNORM_S8_UINT,
		nullptr, 
		mRenderPrimitiveManager->GetHeapByName("mShadowSrvDescriptorHeap"), 
		mRenderPrimitiveManager->GetHeapByName("mShadowDsvDescriptorHeap"), 
		false, 0, 2048,2048);
	mRenderPrimitiveManager->InsertRenderTargetToLib("shadowRenderTarget", shadowRenderTarget);
	}
	//HDRRenderTarget=====================================================
	{
		{
			auto HDRRenderTarget = mRHI->CreateRenderTarget(
				"HDRRenderTarget", TEXTURE2D, R16G16B16A16_FLOAT,
				mRenderPrimitiveManager->GetHeapByName("BloomRtvHeap"),
				mRenderPrimitiveManager->GetHeapByName("BloomSrvHeap"),
				mRenderPrimitiveManager->GetHeapByName("BloomDsvHeap"),
				false, 1, mClientWidth, mClientHeight);
			mRenderPrimitiveManager->InsertRenderTargetToLib("HDRRenderTarget", HDRRenderTarget);
		}
		{
		auto HDRShadowHighLightLessRenderTarget = mRHI->CreateRenderTarget(
			"HDRShadowHighLightLessRenderTarget", TEXTURE2D,  R16G16B16A16_FLOAT,
			mRenderPrimitiveManager->GetHeapByName("BloomRtvHeap"), 
			mRenderPrimitiveManager->GetHeapByName("BloomSrvHeap"), 
			mRenderPrimitiveManager->GetHeapByName("BloomDsvHeap"), 
			false, 1, mClientWidth, mClientHeight);
		mRenderPrimitiveManager->InsertRenderTargetToLib("HDRShadowHighLightLessRenderTarget", HDRShadowHighLightLessRenderTarget);
		}
	}
	//BloomRenderTarget=====================================================
	{
		//------------------------------------
		{
			auto bloomSetupRenderTarget = mRHI->CreateRenderTarget(
				"bloomSetupRenderTarget", TEXTURE2D,  R16G16B16A16_FLOAT,
				mRenderPrimitiveManager->GetHeapByName("BloomRtvHeap"), 
				mRenderPrimitiveManager->GetHeapByName("BloomSrvHeap"), 
				mRenderPrimitiveManager->GetHeapByName("BloomDsvHeap"), 
				false, 1, mClientWidth / 4, mClientHeight / 4);
			mRenderPrimitiveManager->InsertRenderTargetToLib("bloomSetupRenderTarget", bloomSetupRenderTarget);
		}
		//------------------------------------
		{
			auto bloomDown2RenderTarget = mRHI->CreateRenderTarget(
				"bloomDown2RenderTarget", TEXTURE2D, R16G16B16A16_FLOAT,
				mRenderPrimitiveManager->GetHeapByName("BloomRtvHeap"), 
				mRenderPrimitiveManager->GetHeapByName("BloomSrvHeap"), 
				mRenderPrimitiveManager->GetHeapByName("BloomDsvHeap"), 
				false, 1, (mClientWidth / 4) / 2, (mClientHeight / 4) / 2);
			mRenderPrimitiveManager->InsertRenderTargetToLib("bloomDown2RenderTarget", bloomDown2RenderTarget);
		}
		//------------------------------------
		{
			auto bloomDown3RenderTarget = mRHI->CreateRenderTarget(
				"bloomDown3RenderTarget", TEXTURE2D,  R16G16B16A16_FLOAT,
				mRenderPrimitiveManager->GetHeapByName("BloomRtvHeap"), 
				mRenderPrimitiveManager->GetHeapByName("BloomSrvHeap"), 
				mRenderPrimitiveManager->GetHeapByName("BloomDsvHeap"), 
				false, 1, ((mClientWidth / 4) / 2)/2, ((mClientHeight / 4) / 2)/2);
			mRenderPrimitiveManager->InsertRenderTargetToLib("bloomDown3RenderTarget", bloomDown3RenderTarget);
		}
		//------------------------------------
		{
			auto bloomDown4RenderTarget = mRHI->CreateRenderTarget(
				"bloomDown4RenderTarget", TEXTURE2D, R16G16B16A16_FLOAT,
				mRenderPrimitiveManager->GetHeapByName("BloomRtvHeap"), 
				mRenderPrimitiveManager->GetHeapByName("BloomSrvHeap"), 
				mRenderPrimitiveManager->GetHeapByName("BloomDsvHeap"), 
				false, 1, (((mClientWidth / 4) / 2) / 2)/2, (((mClientHeight / 4) / 2) / 2)/2);
			mRenderPrimitiveManager->InsertRenderTargetToLib("bloomDown4RenderTarget", bloomDown4RenderTarget);
		}
		//------------------------------------
		{
			auto bloomUpRenderTarget = mRHI->CreateRenderTarget(
				"bloomUpRenderTarget", TEXTURE2D,  R16G16B16A16_FLOAT,
				mRenderPrimitiveManager->GetHeapByName("BloomRtvHeap"), 
				mRenderPrimitiveManager->GetHeapByName("BloomSrvHeap"), 
				mRenderPrimitiveManager->GetHeapByName("BloomDsvHeap"), 
				false, 1, (((mClientWidth / 4) / 2) / 2), (((mClientHeight / 4) / 2) / 2));
			mRenderPrimitiveManager->InsertRenderTargetToLib("bloomUpRenderTarget", bloomUpRenderTarget);
		}
		//------------------------------------
		{
			auto bloomUp2RenderTarget = mRHI->CreateRenderTarget(
				"bloomUp2RenderTarget", TEXTURE2D,  R16G16B16A16_FLOAT,
				mRenderPrimitiveManager->GetHeapByName("BloomRtvHeap"), 
				mRenderPrimitiveManager->GetHeapByName("BloomSrvHeap"), 
				mRenderPrimitiveManager->GetHeapByName("BloomDsvHeap"), 
				false, 1, (mClientWidth / 4)/2, (mClientHeight / 4)/2 );
			mRenderPrimitiveManager->InsertRenderTargetToLib("bloomUp2RenderTarget", bloomUp2RenderTarget);
		}
		//------------------------------------
		{
			auto bloomMergeRenderTarget = mRHI->CreateRenderTarget(
				"bloomMergeRenderTarget", TEXTURE2D,  R16G16B16A16_FLOAT,
				mRenderPrimitiveManager->GetHeapByName("BloomRtvHeap"), 
				mRenderPrimitiveManager->GetHeapByName("BloomSrvHeap"), 
				mRenderPrimitiveManager->GetHeapByName("BloomDsvHeap"), 
				false, 1, (mClientWidth / 4) , (mClientHeight / 4) );
			mRenderPrimitiveManager->InsertRenderTargetToLib("bloomMergeRenderTarget", bloomMergeRenderTarget);
		}
		{
			auto ToneMapRenderTarget = mRHI->CreateRenderTarget(
				"ToneMapRenderTarget", TEXTURE2D, R8G8B8A8_UNORM,
				mRenderPrimitiveManager->GetHeapByName("BloomRtvHeap"), 
				mRenderPrimitiveManager->GetHeapByName("BloomSrvHeap"), 
				mRenderPrimitiveManager->GetHeapByName("BloomDsvHeap"), 
				false, 1, mClientWidth, mClientHeight);
			mRenderPrimitiveManager->InsertRenderTargetToLib("ToneMapRenderTarget", ToneMapRenderTarget);
		}
	}
	//TestPostProcess======================================
	{
		auto TestPostProcessRenderTarget = mRHI->CreateRenderTarget(
			"TestPostProcessRenderTarget", TEXTURE2D, R8G8B8A8_UNORM,
			mRenderPrimitiveManager->GetHeapByName("BloomRtvHeap"), 
			mRenderPrimitiveManager->GetHeapByName("BloomSrvHeap"), 
			mRenderPrimitiveManager->GetHeapByName("BloomDsvHeap"), 
			false, 1, mClientWidth, mClientHeight);
		mRenderPrimitiveManager->InsertRenderTargetToLib("TestPostProcessRenderTarget", TestPostProcessRenderTarget);
	}
	//EdgeDetectionSobel===================================
	{
		auto EdgeDetectionSobelRenderTarget = mRHI->CreateRenderTarget(
			"EdgeDetectionSobelRenderTarget", TEXTURE2D, R8G8B8A8_UNORM,
			mRenderPrimitiveManager->GetHeapByName("BloomRtvHeap"), 
			mRenderPrimitiveManager->GetHeapByName("BloomSrvHeap"), 
			mRenderPrimitiveManager->GetHeapByName("BloomDsvHeap"), 
			false, 1, mClientWidth, mClientHeight);
		mRenderPrimitiveManager->InsertRenderTargetToLib("EdgeDetectionSobelRenderTarget", EdgeDetectionSobelRenderTarget);
	}
	//HDRUseTexture==========================================
	{
		auto HDRUseTextureRenderTarget = mRHI->CreateRenderTarget(
			"HDRUseTextureRenderTarget", TEXTURE2D, R8G8B8A8_UNORM,
			mRenderPrimitiveManager->GetHeapByName("BloomRtvHeap"),
			mRenderPrimitiveManager->GetHeapByName("BloomSrvHeap"),
			mRenderPrimitiveManager->GetHeapByName("BloomDsvHeap"),
			false, 1, mClientWidth, mClientHeight);
		mRenderPrimitiveManager->InsertRenderTargetToLib("HDRUseTextureRenderTarget", HDRUseTextureRenderTarget);
	}
}

void Renderer::LoadMeshAndSetBuffer()
{
	std::string StaticMeshPath;
	std::set<std::string> StaticMeshs;//用于去重
	Engine::Get()->GetAssetManager()->GetGeometryLibrary()->resize(Engine::Get()->GetAssetManager()->GetMapActorInfo()->Size());

	//循环加入MeshGeometry
	for (int i = 0; i < Engine::Get()->GetAssetManager()->Geos.size(); i++)
	{
		std::shared_ptr<StaticMesh> mesh = std::make_shared<StaticMesh>();

		//相同的StaticMesh不用重复建立
		auto check = StaticMeshs.find(Engine::Get()->GetAssetManager()->GetMapActorInfo()->MeshNameArray[i]);
		if (check == StaticMeshs.end()) {
			StaticMeshs.insert(Engine::Get()->GetAssetManager()->GetMapActorInfo()->MeshNameArray[i]);
			Engine::Get()->GetAssetManager()->GetMapofGeosMesh()->insert(std::pair<int, std::string>(i, Engine::Get()->GetAssetManager()->GetMapActorInfo()->MeshNameArray[i]));
		}
		else { continue; }

		//读取mesh信息
		StaticMeshPath = "SplitMesh/" + Engine::Get()->GetAssetManager()->GetMapActorInfo()->MeshNameArray[i];
		StaticMeshPath.erase(StaticMeshPath.length() - 1);
		StaticMeshPath += ".bat";
		mesh->LoadStaticMeshFromBat(StaticMeshPath);
		
		mesh->SetMeshBuffer(mRHI->CreateMeshBuffer(mesh));

		//Mesh添加Texture
		mesh->material = std::make_shared<FMaterial>();
		auto texture = mRHI->CreateTexture("TestTexture", TextureLoadPath,2000);
		mesh->material->InsertTextureToTexLib("TestTexture", texture);

		auto Normaltexture = mRHI->CreateTexture("NormalTexture", NormalLoadPath, 2001);
		mesh->material->InsertTextureToTexLib("NormalTexture", Normaltexture);

		Engine::Get()->GetAssetManager()->InsertStaticMeshToLib(mesh->GetMeshName(), mesh);
	}
}

void Renderer::Update()
{
	//mRHI->Update();
	for (int index = 0; index < Engine::Get()->GetAssetManager()->GetMapActorInfo()->Size(); index++)
	{
		ObjectConstants objConstants;
		UpdateShadow(index, objConstants);
		UpdateMesh(index, objConstants);
		mRHI->CommitConstantBufferToGPU(index,objConstants);
	}
	auto a = Engine::Get()->GetAssetManager()->GetMapActorInfo()->Size();
	mRHI->CalculateFrameStats();
}

void Renderer::Draw()
{
	mRHI->DrawReset();

	DrawBaseScenePass();
	DrawShadowPass();
	DrawBloomPass();
	DrawTestPostProcessPass();
	DrawTextureScenePass();
	DrawScenePass();

	mRHI->DrawFinal();
}

void Renderer::DrawBaseScenePass()
{
	{
		mRHI->RenderDocBeginEvent("DrawHDRBaseScene");
		// Draw HDR Scene ================================================
		auto HDRShadowHighLightLessRenderTarget = mRenderPrimitiveManager->GetRenderTargetByName("HDRShadowHighLightLessRenderTarget");

		mRHI->SetScreenSetViewPort(
			HDRShadowHighLightLessRenderTarget->width,
			HDRShadowHighLightLessRenderTarget->height);
		mRHI->SetScissorRect(
			long(HDRShadowHighLightLessRenderTarget->width),
			long(HDRShadowHighLightLessRenderTarget->height));
		mRHI->ResourceTransition(HDRShadowHighLightLessRenderTarget->mSwapChainResource[0], STATE_RENDER_TARGET);
		mRHI->ClearRenderTargetView(HDRShadowHighLightLessRenderTarget, mClearColor, 0);
		mRHI->ClearDepthStencilView(HDRShadowHighLightLessRenderTarget);
		mRHI->OMSetRenderTargets(HDRShadowHighLightLessRenderTarget);
		mRHI->SetPipelineState(mRenderPrimitiveManager->GetPipelineByName("HDRShadowHighLightLessPipeline"));
		mRHI->SetDescriptorHeap(mRenderPrimitiveManager->GetHeapByName("mCbvHeap"));

		auto scene = Engine::Get()->GetAssetManager()->GetScene();
		for (auto sceneActor : scene->actorLib)
		{
			auto actor = sceneActor.second;
			auto mCbvHeap = mRenderPrimitiveManager->GetHeapByName("mCbvHeap");
			auto DrawMeshName = actor->staticmeshName;
			auto mesh = Engine::Get()->GetAssetManager()->GetStaticMeshByName(DrawMeshName);

			mRHI->CommitShaderParameter_Heap(0, actor->CBVoffset, mCbvHeap);
			mRHI->CommitShaderParameter_Heap(1, mesh->material->GetTextureByName("TestTexture")->heapOffset, mCbvHeap);
			mRHI->CommitShaderParameter_Heap(3, mesh->material->GetTextureByName("NormalTexture")->heapOffset, mCbvHeap);
			mRHI->DrawMesh(mesh);
		}
		mRHI->ResourceTransition(HDRShadowHighLightLessRenderTarget->mSwapChainResource[0], STATE_PIXEL_SHADER_RESOURCE);
		mRHI->RenderDocEndEvent();
	}
}

void Renderer::UpdateMesh(int i, ObjectConstants& objConstants)
{
	//UPDate Actor--------------------------------------------------------------------------------
	//auto world = MathHelper::Identity4x4();
	auto location = XMMatrixTranslation(
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].translation.x,
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].translation.y,
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].translation.z
	);
	auto Scale = XMMatrixScaling(
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].scale3D.x,
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].scale3D.y,
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].scale3D.z
	);
	auto Rotator = XMMatrixScaling(
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].rotation.Pitch,
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].rotation.Yaw,
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].rotation.Roll
	);

	DirectX::XMVECTORF32 g_XMIdentityR3 = { { {
			Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsQuatArray[i].X,
			Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsQuatArray[i].Y,
			Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsQuatArray[i].Z,
			Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsQuatArray[i].W
		} } };

	auto mrotation = DirectX::XMMatrixRotationQuaternion(g_XMIdentityR3);

	glm::mat4 translateMat4 = glm::translate(glm::identity<glm::mat4>(), glm::vec3(
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].translation.x,
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].translation.y,
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].translation.z
	));

	glm::mat4 scaleMat4 = glm::scale(glm::identity<glm::mat4>(), glm::vec3(
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].scale3D.x,
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].scale3D.y,
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].scale3D.z
	));

	glm::quat rotationQuat(
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsQuatArray[i].X,
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsQuatArray[i].Y,
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsQuatArray[i].Z,
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsQuatArray[i].W
	);
	glm::mat4 rotationMat4 = glm::toMat4(rotationQuat);

	auto world = Scale * mrotation * location;
	glm::mat4 worldMat4 = scaleMat4 * rotationMat4 * translateMat4;
	//!!!旋转矩阵好像有问题，用glm的rotator传给Shader，mul(Normal,rotator)的值不对，表现出来的颜色不是正确的。
	//!!!要看正确的颜色可以乘XMMATRIX的rotation，XMMATRIX还要转化为FLOAT4X4

	auto mCamera = Engine::Get()->GetRenderer()->GetCamera();
	XMMATRIX worldViewProj = world * XMLoadFloat4x4(&mCamera->GetView4x4f()) * XMLoadFloat4x4(&mCamera->GetProj4x4f());
	glm::mat4 worldViewProjMat4 = mCamera->GetProjMat4() * mCamera->GetViewMat4() * worldMat4;

	//XMMATRIX worldViewProj = world * XMLoadFloat4x4(&mCamera.GetView4x4f()) * XMLoadFloat4x4((&mCamera.GetProj4x4f()));
	//glm::mat4 worldViewProjMat4 = mCamera.GetProjMat4() * mCamera.GetViewMat4() * worldMat4;

	XMStoreFloat4x4(&objConstants.WorldViewProj, XMMatrixTranspose(worldViewProj));
	objConstants.WorldViewProjMat4 = glm::transpose(worldViewProjMat4);
	//objConstants.mTime = Engine::Get()->gt.TotalTime();
	XMStoreFloat4x4(&objConstants.rotation, XMMatrixTranspose(mrotation));

	objConstants.LightDirection.x = Engine::Get()->GetAssetManager()->GetLight()->mLook.x;
	objConstants.LightDirection.y = Engine::Get()->GetAssetManager()->GetLight()->mLook.y;
	objConstants.LightDirection.z = Engine::Get()->GetAssetManager()->GetLight()->mLook.z;
	objConstants.LightDirection.w = 0.0f;

	auto meshName = Engine::Get()->GetAssetManager()->GetMapActorInfo()->MeshNameArray[i];
	auto mesh = Engine::Get()->GetAssetManager()->GetStaticMeshByName(meshName);

	objConstants.LightStrength  = Engine::Get()->GetAssetManager()->GetLight()->Strength;
	objConstants.gDiffuseAlbedo = mesh->material->DiffuseAlbedo;
	objConstants.gFresnelR0= mesh->material->FresnelR0;
	objConstants.gRoughness= mesh->material->Roughness;
	objConstants.LightLocation = Engine::Get()->GetAssetManager()->GetLight()->mLightInfo.mTransform.translation;
	objConstants.LightLocationW = 1;
	objConstants.CameraLocation = GetCamera()->GetPosition3f();
	objConstants.CameraLocationW = 1;
}

void Renderer::UpdateShadow(int i, ObjectConstants& objConstants)
{
	auto location = XMMatrixTranslation(
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].translation.x,
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].translation.y,
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].translation.z
	);
	auto Scale = XMMatrixScaling(
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].scale3D.x,
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].scale3D.y,
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].scale3D.z
	);
	auto Rotator = XMMatrixScaling(
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].rotation.Pitch,
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].rotation.Yaw,
		Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsTransformArray[i].rotation.Roll
	);

	DirectX::XMVECTORF32 g_XMIdentityR3 = { { {
			Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsQuatArray[i].X,
			Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsQuatArray[i].Y,
			Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsQuatArray[i].Z,
			Engine::Get()->GetAssetManager()->GetMapActorInfo()->ActorsQuatArray[i].W
		} } };

	auto mrotation = DirectX::XMMatrixRotationQuaternion(g_XMIdentityR3);

	auto world = Scale * mrotation * location;

	auto TestVV = Engine::Get()->GetAssetManager()->mLight->GetView();//重新创建的V矩阵
	auto TestPP = Engine::Get()->GetAssetManager()->mLight->GetProj();//重新创建的P矩阵

	XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	////重新造的VP
	XMMATRIX VP = TestVV * TestPP;
	XMMATRIX S = TestVV * TestPP * T;
	XMMATRIX LightworldViewProj = world * TestVV * TestPP;
	XMMATRIX LightworldViewProjT = world * TestVV * TestPP * T;

	XMStoreFloat4x4(&objConstants.gWorld, XMMatrixTranspose(world));
	XMStoreFloat4x4(&objConstants.gLightVP, XMMatrixTranspose(VP));
	XMStoreFloat4x4(&objConstants.gShadowTransform, XMMatrixTranspose(S));
	XMStoreFloat4x4(&objConstants.gLightMVP, XMMatrixTranspose(LightworldViewProj));
	XMStoreFloat4x4(&objConstants.gLightMVPT, XMMatrixTranspose(LightworldViewProjT));
}

void Renderer::DrawShadowPass()
{
	mRHI->RenderDocBeginEvent("Shadow");
	auto Rendertarget = mRenderPrimitiveManager->GetRenderTargetByName("shadowRenderTarget");
	mRHI->SetScreenSetViewPort(
		Rendertarget->width,
		Rendertarget->height);
	mRHI->SetScissorRect(
		long(Rendertarget->width),
		long(Rendertarget->height));
	mRHI->ClearDepthStencilView(Rendertarget);
	mRHI->OMSetRenderTargets(Rendertarget);

	auto scene = Engine::Get()->GetAssetManager()->GetScene();
	for (auto sceneActor : scene->actorLib)
	{
		auto actor = sceneActor.second;
		auto mCbvHeap = mRenderPrimitiveManager->GetHeapByName("mCbvHeap");
		auto DrawMeshName = actor->staticmeshName;
		auto mesh = Engine::Get()->GetAssetManager()->GetStaticMeshByName(DrawMeshName);

		mRHI->SetDescriptorHeap(mRenderPrimitiveManager->GetHeapByName("mCbvHeap"));
		mRHI->SetPipelineState(mRenderPrimitiveManager->GetPipelineByName("shadowPipeline"));
		mRHI->CommitShaderParameter_Heap(0, actor->CBVoffset, mCbvHeap);
		mRHI->DrawMesh(mesh);

		mRHI->CommitShaderParameters();//这个提交shader函数没有做完
	}

	mRHI->RenderDocEndEvent();
}

void Renderer::DrawBloomPass()
{
	mRHI->RenderDocBeginEvent("Bloom");
	{	
		mRHI->RenderDocBeginEvent("DrawHDRScene");
		// Draw HDR Scene ================================================
		auto HDRRendertarget = mRenderPrimitiveManager->GetRenderTargetByName("HDRRenderTarget");

		mRHI->SetScreenSetViewPort(
			HDRRendertarget->width,
			HDRRendertarget->height);
		mRHI->SetScissorRect(
			long(HDRRendertarget->width),
			long(HDRRendertarget->height));
		mRHI->ResourceTransition(HDRRendertarget->mSwapChainResource[0], STATE_RENDER_TARGET);
		mRHI->ClearRenderTargetView(HDRRendertarget, mClearColor, 0);
		mRHI->ClearDepthStencilView(HDRRendertarget);
		mRHI->OMSetRenderTargets(HDRRendertarget);
		mRHI->SetPipelineState(mRenderPrimitiveManager->GetPipelineByName("HDRPipeline"));
		mRHI->SetDescriptorHeap(mRenderPrimitiveManager->GetHeapByName("mCbvHeap"));
		
		auto scene = Engine::Get()->GetAssetManager()->GetScene();
		for (auto sceneActor: scene->actorLib)
		{
			auto actor = sceneActor.second;
			auto mCbvHeap = mRenderPrimitiveManager->GetHeapByName("mCbvHeap");
			auto DrawMeshName = actor->staticmeshName;
			auto mesh = Engine::Get()->GetAssetManager()->GetStaticMeshByName(DrawMeshName);

			mRHI->CommitShaderParameter_Heap(0, actor->CBVoffset, mCbvHeap);
			mRHI->CommitShaderParameter_Heap(1, mesh->material->GetTextureByName("TestTexture")->heapOffset, mCbvHeap);
			mRHI->CommitShaderParameter_Heap(3, mesh->material->GetTextureByName("NormalTexture")->heapOffset, mCbvHeap);
			mRHI->DrawMesh(mesh);
		}
		mRHI->ResourceTransition(HDRRendertarget->mSwapChainResource[0], STATE_PIXEL_SHADER_RESOURCE);
		mRHI->RenderDocEndEvent();
	}
	//Draw Setup Scene================================================
	{
		mRHI->RenderDocBeginEvent("BloomSetup");
		auto HDRRendertarget = mRenderPrimitiveManager->GetRenderTargetByName("HDRRenderTarget");
		auto SetupRendertarget = mRenderPrimitiveManager->GetRenderTargetByName("bloomSetupRenderTarget");
		mRHI->SetScreenSetViewPort(
			SetupRendertarget->width,
			SetupRendertarget->height);
		mRHI->SetScissorRect(
			long(SetupRendertarget->width),
			long(SetupRendertarget->height));
		mRHI->ResourceTransition(SetupRendertarget->mSwapChainResource[0], STATE_RENDER_TARGET);
		mRHI->ClearRenderTargetView(SetupRendertarget, mClearColor, 0);
		mRHI->ClearDepthStencilView(SetupRendertarget);
		mRHI->OMSetRenderTargets(SetupRendertarget);
		mRHI->SetPipelineState(mRenderPrimitiveManager->GetPipelineByName("bloomSetupPipeline"));

		FVector4* screenSize=new FVector4();
		screenSize->X = mClientWidth;
		screenSize->Y = mClientHeight;
		screenSize->Z = -1.0f;
		screenSize->W = -1.0f;

 		mRHI->SetDescriptorHeap(mRenderPrimitiveManager->GetHeapByName("BloomSrvHeap"));
 		mRHI->CommitShaderParameter_Texture(1, HDRRendertarget);
		mRHI->CommitShaderParameter_Constant(2, 4, screenSize);

		delete screenSize;
		mRHI->BuildTriangleAndDraw(Engine::Get()->GetAssetManager()->GetStaticMeshByName("triangle")->meshBuffer);
		mRHI->ResourceTransition(SetupRendertarget->mSwapChainResource[0], STATE_PIXEL_SHADER_RESOURCE);
		
		mRHI->RenderDocEndEvent();
	}
	//Draw Down2 Scene================================================
	{
		mRHI->RenderDocBeginEvent("BloomDown");

		auto SetupRendertarget = mRenderPrimitiveManager->GetRenderTargetByName("bloomSetupRenderTarget");
		auto Down2Rendertarget = mRenderPrimitiveManager->GetRenderTargetByName("bloomDown2RenderTarget");
		mRHI->SetScreenSetViewPort(
			Down2Rendertarget->width,
			Down2Rendertarget->height);
		mRHI->SetScissorRect(
			long(Down2Rendertarget->width),
			long(Down2Rendertarget->height));
		mRHI->ResourceTransition(Down2Rendertarget->mSwapChainResource[0], STATE_RENDER_TARGET);
		mRHI->ClearRenderTargetView(Down2Rendertarget, mClearColor, 0);
		mRHI->ClearDepthStencilView(Down2Rendertarget);
		mRHI->OMSetRenderTargets(Down2Rendertarget);
		mRHI->SetPipelineState(mRenderPrimitiveManager->GetPipelineByName("bloomDownPipeline"));

		std::unique_ptr<FVector4> screenSize = std::make_unique<FVector4>();
		screenSize->X = mClientWidth/4;
		screenSize->Y = mClientHeight/4;
		screenSize->Z = -1.0f;
		screenSize->W = -1.0f;

		mRHI->SetDescriptorHeap(mRenderPrimitiveManager->GetHeapByName("BloomSrvHeap"));
		mRHI->CommitShaderParameter_Texture(1, SetupRendertarget);
		mRHI->CommitShaderParameter_Constant(2, 4, screenSize.get());
		mRHI->BuildTriangleAndDraw(Engine::Get()->GetAssetManager()->GetStaticMeshByName("triangle")->meshBuffer);
		
		mRHI->ResourceTransition(Down2Rendertarget->mSwapChainResource[0], STATE_PIXEL_SHADER_RESOURCE);
		mRHI->RenderDocEndEvent();
	}
	//Draw Down3 Scene================================================
	{
		mRHI->RenderDocBeginEvent("BloomDown");

		auto Down2Rendertarget = mRenderPrimitiveManager->GetRenderTargetByName("bloomDown2RenderTarget");
		auto Down3Rendertarget = mRenderPrimitiveManager->GetRenderTargetByName("bloomDown3RenderTarget");
		mRHI->SetScreenSetViewPort(
			Down3Rendertarget->width,
			Down3Rendertarget->height);
		mRHI->SetScissorRect(
			long(Down3Rendertarget->width),
			long(Down3Rendertarget->height));
		mRHI->ResourceTransition(Down3Rendertarget->mSwapChainResource[0], STATE_RENDER_TARGET);
		mRHI->ClearRenderTargetView(Down3Rendertarget, mClearColor, 0);
		mRHI->ClearDepthStencilView(Down3Rendertarget);
		mRHI->OMSetRenderTargets(Down3Rendertarget);
		mRHI->SetPipelineState(mRenderPrimitiveManager->GetPipelineByName("bloomDownPipeline"));

		FVector4* screenSize = new FVector4();
		screenSize->X = (mClientWidth / 4)/2;
		screenSize->Y = (mClientHeight / 4)/2;
		screenSize->Z = -1.0f;
		screenSize->W = -1.0f;

		mRHI->SetDescriptorHeap(mRenderPrimitiveManager->GetHeapByName("BloomSrvHeap"));
		mRHI->CommitShaderParameter_Texture(1, Down2Rendertarget);
		mRHI->CommitShaderParameter_Constant(2, 4, screenSize);
		mRHI->BuildTriangleAndDraw(Engine::Get()->GetAssetManager()->GetStaticMeshByName("triangle")->meshBuffer);
		delete screenSize;
		mRHI->ResourceTransition(Down3Rendertarget->mSwapChainResource[0], STATE_PIXEL_SHADER_RESOURCE);
		mRHI->RenderDocEndEvent();
	}
	//Draw Down4 Scene================================================
	{
		mRHI->RenderDocBeginEvent("BloomDown");

		auto Down3Rendertarget = mRenderPrimitiveManager->GetRenderTargetByName("bloomDown3RenderTarget");
		auto Down4Rendertarget = mRenderPrimitiveManager->GetRenderTargetByName("bloomDown4RenderTarget");
		mRHI->SetScreenSetViewPort(
			Down4Rendertarget->width,
			Down4Rendertarget->height);
		mRHI->SetScissorRect(
			long(Down4Rendertarget->width),
			long(Down4Rendertarget->height));
		mRHI->ResourceTransition(Down4Rendertarget->mSwapChainResource[0], STATE_RENDER_TARGET);
		mRHI->ClearRenderTargetView(Down4Rendertarget, mClearColor, 0);
		mRHI->ClearDepthStencilView(Down4Rendertarget);
		mRHI->OMSetRenderTargets(Down4Rendertarget);
		mRHI->SetPipelineState(mRenderPrimitiveManager->GetPipelineByName("bloomDownPipeline"));
		
		FVector4* screenSize = new FVector4();
		screenSize->X = ((mClientWidth / 4) / 2)/2;
		screenSize->Y = ((mClientHeight / 4) / 2)/2;
		screenSize->Z = -1.0f;
		screenSize->W = -1.0f;

		mRHI->SetDescriptorHeap(mRenderPrimitiveManager->GetHeapByName("BloomSrvHeap"));
		mRHI->CommitShaderParameter_Texture(1, Down3Rendertarget);
		mRHI->CommitShaderParameter_Constant(2, 4, screenSize);
		mRHI->BuildTriangleAndDraw(Engine::Get()->GetAssetManager()->GetStaticMeshByName("triangle")->meshBuffer);
		delete screenSize;
		mRHI->ResourceTransition(Down4Rendertarget->mSwapChainResource[0], STATE_PIXEL_SHADER_RESOURCE);
		mRHI->RenderDocEndEvent();
	}
	//Draw Up Scene================================================
	{
		mRHI->RenderDocBeginEvent("BloomUp");

		auto Down3Rendertarget = mRenderPrimitiveManager->GetRenderTargetByName("bloomDown3RenderTarget");
		auto Down4Rendertarget = mRenderPrimitiveManager->GetRenderTargetByName("bloomDown4RenderTarget");
		
		auto UpRendertarget = mRenderPrimitiveManager->GetRenderTargetByName("bloomUpRenderTarget");
		mRHI->SetScreenSetViewPort(
			UpRendertarget->width,
			UpRendertarget->height);
		mRHI->SetScissorRect(
			long(UpRendertarget->width),
			long(UpRendertarget->height));
		mRHI->ResourceTransition(UpRendertarget->mSwapChainResource[0], STATE_RENDER_TARGET);
		mRHI->ClearRenderTargetView(UpRendertarget, mClearColor, 0);
		mRHI->ClearDepthStencilView(UpRendertarget);
		mRHI->OMSetRenderTargets(UpRendertarget);
		mRHI->SetPipelineState(mRenderPrimitiveManager->GetPipelineByName("bloomUpPipeline"));

		FVector4* screenSize = new FVector4();
		screenSize->X = ((mClientWidth / 4) / 2) / 2;
		screenSize->Y = ((mClientHeight / 4) / 2) / 2;
		screenSize->Z = 10.0f;
		screenSize->W = 10.0f;

		mRHI->SetDescriptorHeap(mRenderPrimitiveManager->GetHeapByName("BloomSrvHeap"));
		mRHI->CommitShaderParameter_Texture(1, Down4Rendertarget);
		mRHI->CommitShaderParameter_Texture(3, Down3Rendertarget);
		mRHI->CommitShaderParameter_Constant(2, 4, screenSize);
		mRHI->BuildTriangleAndDraw(Engine::Get()->GetAssetManager()->GetStaticMeshByName("triangle")->meshBuffer);
		delete screenSize;
		mRHI->ResourceTransition(UpRendertarget->mSwapChainResource[0], STATE_PIXEL_SHADER_RESOURCE);
		mRHI->RenderDocEndEvent();
	}
	//Draw Up2 Scene================================================
	{
		mRHI->RenderDocBeginEvent("BloomUp");

		auto Down2Rendertarget = mRenderPrimitiveManager->GetRenderTargetByName("bloomDown2RenderTarget");
		auto UpRendertarget = mRenderPrimitiveManager->GetRenderTargetByName("bloomUpRenderTarget");

		auto Up2Rendertarget = mRenderPrimitiveManager->GetRenderTargetByName("bloomUp2RenderTarget");
		mRHI->SetScreenSetViewPort(
			Up2Rendertarget->width,
			Up2Rendertarget->height);
		mRHI->SetScissorRect(
			long(Up2Rendertarget->width),
			long(Up2Rendertarget->height));
		mRHI->ResourceTransition(Up2Rendertarget->mSwapChainResource[0], STATE_RENDER_TARGET);
		mRHI->ClearRenderTargetView(Up2Rendertarget, mClearColor, 0);
		mRHI->ClearDepthStencilView(Up2Rendertarget);
		mRHI->OMSetRenderTargets(Up2Rendertarget);
		mRHI->SetPipelineState(mRenderPrimitiveManager->GetPipelineByName("bloomUpPipeline"));
		mRHI->SetDescriptorHeap(mRenderPrimitiveManager->GetHeapByName("mCbvHeap"));

		FVector4* screenSize = new FVector4();
		screenSize->X = ((mClientWidth / 4) / 2) ;
		screenSize->Y = ((mClientHeight / 4) / 2) ;
		screenSize->Z = 10.0f;
		screenSize->W = 10.0f;

		mRHI->SetDescriptorHeap(mRenderPrimitiveManager->GetHeapByName("BloomSrvHeap"));
		mRHI->CommitShaderParameter_Texture(1, UpRendertarget);
		mRHI->CommitShaderParameter_Texture(3, Down2Rendertarget);
		mRHI->CommitShaderParameter_Constant(2, 4, screenSize);

		mRHI->BuildTriangleAndDraw(Engine::Get()->GetAssetManager()->GetStaticMeshByName("triangle")->meshBuffer);
		delete screenSize;
		mRHI->ResourceTransition(Up2Rendertarget->mSwapChainResource[0], STATE_PIXEL_SHADER_RESOURCE);
		mRHI->RenderDocEndEvent();
	}
	//Draw Merge Scene================================================
	{
		mRHI->RenderDocBeginEvent("BloomMerge");

		auto SetupRendertarget = mRenderPrimitiveManager->GetRenderTargetByName("bloomSetupRenderTarget");
		auto Up2Rendertarget = mRenderPrimitiveManager->GetRenderTargetByName("bloomUp2RenderTarget");

		auto bloomMergeRendertarget = mRenderPrimitiveManager->GetRenderTargetByName("bloomMergeRenderTarget");
		mRHI->SetScreenSetViewPort(
			bloomMergeRendertarget->width,
			bloomMergeRendertarget->height);
		mRHI->SetScissorRect(
			long(bloomMergeRendertarget->width),
			long(bloomMergeRendertarget->height));
		mRHI->ResourceTransition(bloomMergeRendertarget->mSwapChainResource[0], STATE_RENDER_TARGET);
		mRHI->ClearRenderTargetView(bloomMergeRendertarget, mClearColor, 0);
		mRHI->ClearDepthStencilView(bloomMergeRendertarget);
		mRHI->OMSetRenderTargets(bloomMergeRendertarget);
		mRHI->SetPipelineState(mRenderPrimitiveManager->GetPipelineByName("bloomMergePipeline"));
		
		FVector4* screenSize = new FVector4();
		screenSize->X = (mClientWidth / 4) ;
		screenSize->Y = (mClientHeight / 4) ;
		screenSize->Z = 10.0f;
		screenSize->W = 10.0f;

		mRHI->SetDescriptorHeap(mRenderPrimitiveManager->GetHeapByName("BloomSrvHeap"));
		mRHI->CommitShaderParameter_Texture(1, Up2Rendertarget);
		mRHI->CommitShaderParameter_Texture(3, SetupRendertarget);
		mRHI->CommitShaderParameter_Constant(2, 4, screenSize);
		mRHI->BuildTriangleAndDraw(Engine::Get()->GetAssetManager()->GetStaticMeshByName("triangle")->meshBuffer);
		delete screenSize;
		mRHI->ResourceTransition(bloomMergeRendertarget->mSwapChainResource[0], STATE_PIXEL_SHADER_RESOURCE);
		mRHI->RenderDocEndEvent();
	}
	mRHI->RenderDocEndEvent();
}

void Renderer::DrawTestPostProcessPass()
{
	{
		//Glich======================================================
		mRHI->RenderDocBeginEvent("TestPostProcess");
		auto HDRRendertarget = mRenderPrimitiveManager->GetRenderTargetByName("HDRRenderTarget");
		auto TestPostProcessRendertarget = mRenderPrimitiveManager->GetRenderTargetByName("TestPostProcessRenderTarget");
		mRHI->SetScreenSetViewPort(
			TestPostProcessRendertarget->width,
			TestPostProcessRendertarget->height);
		mRHI->SetScissorRect(
			long(TestPostProcessRendertarget->width),
			long(TestPostProcessRendertarget->height));
		mRHI->ResourceTransition(TestPostProcessRendertarget->mSwapChainResource[0], STATE_RENDER_TARGET);
		mRHI->ClearRenderTargetView(TestPostProcessRendertarget, mClearColor, 0);
		mRHI->ClearDepthStencilView(TestPostProcessRendertarget);
		mRHI->OMSetRenderTargets(TestPostProcessRendertarget);
		mRHI->SetPipelineState(mRenderPrimitiveManager->GetPipelineByName("TestPostProcessPipeline"));

		FVector4* screenSize = new FVector4();
		screenSize->X = mClientWidth;
		screenSize->Y = mClientHeight;
		screenSize->Z = -1.0f;
		screenSize->W = -1.0f;

		mRHI->SetDescriptorHeap(mRenderPrimitiveManager->GetHeapByName("BloomSrvHeap"));
		mRHI->CommitShaderParameter_Texture(1, HDRRendertarget);
		mRHI->CommitShaderParameter_Constant(2, 4, screenSize);

		delete screenSize;
		mRHI->BuildTriangleAndDraw(Engine::Get()->GetAssetManager()->GetStaticMeshByName("triangle")->meshBuffer);
		mRHI->ResourceTransition(TestPostProcessRendertarget->mSwapChainResource[0], STATE_PIXEL_SHADER_RESOURCE);
		mRHI->RenderDocEndEvent();
	}
	{
		//EdgeDetectionSobel=============================================
		mRHI->RenderDocBeginEvent("EdgeDetectionSobel");
		auto HDRShadowHighLightLessRenderTarget = mRenderPrimitiveManager->GetRenderTargetByName("HDRShadowHighLightLessRenderTarget");
		auto EdgeDetectionSobelRenderTarget = mRenderPrimitiveManager->GetRenderTargetByName("EdgeDetectionSobelRenderTarget");
		mRHI->SetScreenSetViewPort(
			EdgeDetectionSobelRenderTarget->width,
			EdgeDetectionSobelRenderTarget->height);
		mRHI->SetScissorRect(
			long(EdgeDetectionSobelRenderTarget->width),
			long(EdgeDetectionSobelRenderTarget->height));
		mRHI->ResourceTransition(EdgeDetectionSobelRenderTarget->mSwapChainResource[0], STATE_RENDER_TARGET);
		mRHI->ClearRenderTargetView(EdgeDetectionSobelRenderTarget, mClearColor, 0);
		mRHI->ClearDepthStencilView(EdgeDetectionSobelRenderTarget);
		mRHI->OMSetRenderTargets(EdgeDetectionSobelRenderTarget);
		mRHI->SetPipelineState(mRenderPrimitiveManager->GetPipelineByName("EdgeDetectionSobelPipeline"));

		FVector4* screenSize = new FVector4();
		screenSize->X = mClientWidth;
		screenSize->Y = mClientHeight;
		screenSize->Z = -1.0f;
		screenSize->W = -1.0f;

		mRHI->SetDescriptorHeap(mRenderPrimitiveManager->GetHeapByName("BloomSrvHeap"));
		mRHI->CommitShaderParameter_Texture(1, HDRShadowHighLightLessRenderTarget);
		mRHI->CommitShaderParameter_Constant(2, 4, screenSize);

		delete screenSize;
		mRHI->BuildTriangleAndDraw(Engine::Get()->GetAssetManager()->GetStaticMeshByName("triangle")->meshBuffer);
		mRHI->ResourceTransition(EdgeDetectionSobelRenderTarget->mSwapChainResource[0], STATE_PIXEL_SHADER_RESOURCE);
		mRHI->RenderDocEndEvent();
	}
}

void Renderer::DrawTextureScenePass()
{
	mRHI->RenderDocBeginEvent("DrawTextureScene");
	// Draw Texture Scene ================================================
	auto HDRUseTextureRenderTarget = mRenderPrimitiveManager->GetRenderTargetByName("HDRUseTextureRenderTarget");

	mRHI->SetScreenSetViewPort(
		HDRUseTextureRenderTarget->width,
		HDRUseTextureRenderTarget->height);
	mRHI->SetScissorRect(
		long(HDRUseTextureRenderTarget->width),
		long(HDRUseTextureRenderTarget->height));
	mRHI->ResourceTransition(HDRUseTextureRenderTarget->mSwapChainResource[0], STATE_RENDER_TARGET);
	mRHI->ClearRenderTargetView(HDRUseTextureRenderTarget, mClearColor, 0);
	mRHI->ClearDepthStencilView(HDRUseTextureRenderTarget);
	mRHI->OMSetRenderTargets(HDRUseTextureRenderTarget);
	mRHI->SetPipelineState(mRenderPrimitiveManager->GetPipelineByName("HDRUseTexturePipeline"));
	mRHI->SetDescriptorHeap(mRenderPrimitiveManager->GetHeapByName("mCbvHeap"));

	auto scene = Engine::Get()->GetAssetManager()->GetScene();
	for (auto sceneActor : scene->actorLib)
	{
		auto actor = sceneActor.second;
		auto mCbvHeap = mRenderPrimitiveManager->GetHeapByName("mCbvHeap");
		auto DrawMeshName = actor->staticmeshName;
		auto mesh = Engine::Get()->GetAssetManager()->GetStaticMeshByName(DrawMeshName);

		mRHI->CommitShaderParameter_Heap(0, actor->CBVoffset, mCbvHeap);
		mRHI->CommitShaderParameter_Heap(1, mesh->material->GetTextureByName("TestTexture")->heapOffset, mCbvHeap);
		mRHI->CommitShaderParameter_Heap(3, mesh->material->GetTextureByName("NormalTexture")->heapOffset, mCbvHeap);
		mRHI->DrawMesh(mesh);
	}
	mRHI->ResourceTransition(HDRUseTextureRenderTarget->mSwapChainResource[0], STATE_PIXEL_SHADER_RESOURCE);
	mRHI->RenderDocEndEvent();
}

void Renderer::DrawScenePass()
{
	//Draw Merge Scene================================================
	/*
	{
		auto HDRRendertarget = mRenderPrimitiveManager->GetRenderTargetByName("HDRRenderTarget");
		auto MergeRenderTarget = mRenderPrimitiveManager->GetRenderTargetByName("bloomMergeRenderTarget");

		auto ToneMapRenderTarget = mRenderPrimitiveManager->GetRenderTargetByName("ToneMapRenderTarget");
		mRHI->SetScreenSetViewPort(
			ToneMapRenderTarget->width,
			ToneMapRenderTarget->height);
		mRHI->SetScissorRect(
			long(ToneMapRenderTarget->width),
			long(ToneMapRenderTarget->height));
		//mRHI->ClearRenderTarget(Rendertarget, "BloomRtvHeap");
		mRHI->ClearRenderTargetView(ToneMapRenderTarget, mClearColor, 0);
		mRHI->ClearDepthStencilView(ToneMapRenderTarget);
		mRHI->OMSetRenderTargets(ToneMapRenderTarget);
		mRHI->SetPipelineState(mRenderPrimitiveManager->GetPipelineByName("ToneMapPipeline"));
		mRHI->SetDescriptorHeap(mRenderPrimitiveManager->GetHeapByName("mCbvHeap"));
		auto cbvheap = mRenderPrimitiveManager->GetHeapByName("mCbvHeap");
		int4 screenSize;
		screenSize.x = 1920 ;
		screenSize.y = 1080 ;
		screenSize.z = 10;
		screenSize.w = 10;
		ObjectConstants objConstant;
		objConstant.RenderTargetSize = screenSize;
		mRHI->CommitResourceToGPU(3, objConstant);
		mRHI->CommitShaderParameter_ConstantBuffer(3, cbvheap);

		mRHI->SetDescriptorHeap(mRenderPrimitiveManager->GetHeapByName("BloomSrvHeap"));
		mRHI->CommitShaderParameter_Table(1, HDRRendertarget);
		mRHI->CommitShaderParameter_Table(3, MergeRenderTarget);
		mRHI->CommitShaderParameter_Constant(5, 4, screenSize);
		mRHI->BuildTriangleAndDraw(Engine::Get()->GetAssetManager()->GetStaticMeshByName("triangle")->meshBuffer);
	}
	*/
	
	{
		mRHI->RenderDocBeginEvent("ToneMap");

		//auto HDRRendertarget = mRenderPrimitiveManager->GetRenderTargetByName("HDRShadowHighLightLessRenderTarget");
		auto HDRRendertarget = mRenderPrimitiveManager->GetRenderTargetByName("HDRRenderTarget");
		auto bloomMergeRendertarget = mRenderPrimitiveManager->GetRenderTargetByName("bloomMergeRenderTarget");
		
		auto Rendertarget = mRenderPrimitiveManager->GetRenderTargetByName("baseRenderTarget");

		mRHI->SetScreenSetViewPort(
			Rendertarget->width,
			Rendertarget->height);
		mRHI->SetScissorRect(
			long(Rendertarget->width),
			long(Rendertarget->height));
		//mRHI->ResourceBarrier();
		mRHI->ResourceTransition(Rendertarget->GetCurrentSwapChainBuffer(),STATE_RENDER_TARGET);
		mRHI->ClearRenderTargetView(Rendertarget, mClearColor, 0);
		mRHI->ClearDepthStencilView(Rendertarget);
		mRHI->OMSetRenderTargets(Rendertarget);
		//mRHI->SetPipelineState(mRenderPrimitiveManager->GetPipelineByName("TestPostProcessPipeline"));
		//mRHI->SetPipelineState(mRenderPrimitiveManager->GetPipelineByName("EdgeDetectionSobelPipeline"));
		mRHI->SetPipelineState(mRenderPrimitiveManager->GetPipelineByName("ToneMapPipeline"));
		
		FVector4* screenSize = new FVector4();
		screenSize->X = mClientWidth ;
		screenSize->Y = mClientHeight;
		screenSize->Z = 10.0f;
		screenSize->W = 10.0f;

		mRHI->SetDescriptorHeap(mRenderPrimitiveManager->GetHeapByName("BloomSrvHeap"));
		mRHI->CommitShaderParameter_Texture(1, HDRRendertarget);
		mRHI->CommitShaderParameter_Texture(3, bloomMergeRendertarget);
		mRHI->CommitShaderParameter_Constant(2, 4, screenSize);
		mRHI->BuildTriangleAndDraw(Engine::Get()->GetAssetManager()->GetStaticMeshByName("triangle")->meshBuffer);

		delete screenSize;
		mRHI->RenderDocEndEvent();
	}
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

